// Bloom shader
// Based from:
//	https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
//	http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare (Jorge Jimenez)
//	The Cherno's Hazel Engine https://github.com/TheCherno

#type compute
#version 450 core

layout( binding = 0, rgba32f ) restrict writeonly uniform image2D o_Image;

layout(binding = 1) uniform sampler2D u_InputTexture;
layout(binding = 2) uniform sampler2D u_BloomTexture;

layout(push_constant) uniform u_Settings 
{
	float Threshold;
	float Knee;
	float TK; // Threshold - Knee
	float DK; // Knee * 2.0f
	float QK; // Knee / 0.25f
	float Stage; // -1 = Prefilter, 0 = Downsample, 1 = Upsample
	float LOD;
} pc_Settings;

vec3 Downsample( sampler2D tex, float lod, vec2 uv, vec2 texelSize ) 
{
	// Center
    vec3 A = textureLod(tex, uv, lod).rgb;

    texelSize *= 0.5f; // Sample from center of texels

    // Inner box
    vec3 B = textureLod(tex, uv + texelSize * vec2(-1.0f, -1.0f), lod).rgb;
    vec3 C = textureLod(tex, uv + texelSize * vec2(-1.0f, 1.0f), lod).rgb;
    vec3 D = textureLod(tex, uv + texelSize * vec2(1.0f, 1.0f), lod).rgb;
    vec3 E = textureLod(tex, uv + texelSize * vec2(1.0f, -1.0f), lod).rgb;

    // Outer box
    vec3 F = textureLod(tex, uv + texelSize * vec2(-2.0f, -2.0f), lod).rgb;
    vec3 G = textureLod(tex, uv + texelSize * vec2(-2.0f, 0.0f), lod).rgb;
    vec3 H = textureLod(tex, uv + texelSize * vec2(0.0f, 2.0f), lod).rgb;
    vec3 I = textureLod(tex, uv + texelSize * vec2(2.0f, 2.0f), lod).rgb;
    vec3 J = textureLod(tex, uv + texelSize * vec2(2.0f, 2.0f), lod).rgb;
    vec3 K = textureLod(tex, uv + texelSize * vec2(2.0f, 0.0f), lod).rgb;
    vec3 L = textureLod(tex, uv + texelSize * vec2(-2.0f, -2.0f), lod).rgb;
    vec3 M = textureLod(tex, uv + texelSize * vec2(0.0f, -2.0f), lod).rgb;

    // Weights
    vec3 result = vec3(0.0);
    // Inner box
    result += (B + C + D + E) * 0.5f;
    // Bottom-left box
    result += (F + G + A + M) * 0.125f;
    // Top-left box
    result += (G + H + I + A) * 0.125f;
    // Top-right box
    result += (A + I + J + K) * 0.125f;
    // Bottom-right box
    result += (M + A + K + L) * 0.125f;

    // 4 samples each
    result *= 0.25f;

	return result;
}

vec3 Upsample( sampler2D tex, float lod, vec2 uv, vec2 texelSize, float radius ) 
{
	vec4 offset = texelSize.xyxy * vec4(1.0f, 1.0f, -1.0f, 0.0f) * radius;

    // Center
    vec3 result = textureLod(tex, uv, lod).rgb * 4.0f;

    result += textureLod(tex, uv - offset.xy, lod).rgb;
    result += textureLod(tex, uv - offset.wy, lod).rgb * 2.0;
    result += textureLod(tex, uv - offset.zy, lod).rgb;

    result += textureLod(tex, uv + offset.zw, lod).rgb * 2.0;
    result += textureLod(tex, uv + offset.xw, lod).rgb * 2.0;

    result += textureLod(tex, uv + offset.zy, lod).rgb;
    result += textureLod(tex, uv + offset.wy, lod).rgb * 2.0;
    result += textureLod(tex, uv + offset.xy, lod).rgb;

	return result * ( 1.0f / 16.0f );
}

const float Epsilon = 1.0e-4;

vec4 qt(vec4 color, float threshold, vec3 curve)
{
    // Maximum pixel brightness
    float brightness = max(max(color.r, color.g), color.b);
    // Quadratic curve
    float rq = clamp(brightness - curve.x, 0.0, curve.y);
    rq = (rq * rq) * curve.z;
    color *= max(rq, brightness - threshold) / max(brightness, Epsilon);
    return color;
}

vec4 Prefilter( vec4 color, vec2 tc ) 
{
	float val = 20.0f;

	color = clamp( color, vec4( 0.0f ), vec4( val ) );
	
	color = qt( color, pc_Settings.Threshold, vec3( pc_Settings.TK, pc_Settings.DK, pc_Settings.QK ) );

	return color;
}

layout(local_size_x = 4, local_size_y = 4) in;
void main() 
{
	vec2 imgSize = vec2( imageSize( o_Image ) );

	ivec2 id = ivec2( gl_GlobalInvocationID );
	vec2 tc = vec2( float( id.x ) / imgSize.x, float( id.y ) / imgSize.y );
	tc += (1.0f / imgSize) * 0.5f;

	vec2 texSize = vec2( textureSize( u_InputTexture, int( pc_Settings.LOD ) ) );
	vec4 color = vec4( 1, 0, 1, 1 );

	// Prefilter
	if( pc_Settings.Stage == -1 ) // Prefilter
	{
		color.rgb = Downsample( u_InputTexture, pc_Settings.LOD, tc, 1.0f / texSize );
		color = Prefilter( color, tc );
		color.a = 1.0f;
	}
	else if( pc_Settings.Stage == 0 )  // Downsample
	{
		color.rgb = Downsample( u_InputTexture, pc_Settings.LOD, tc, 1.0f / texSize );
	}
	else if( pc_Settings.Stage == -2 ) // First upsample 
	{
		vec2 texSize = vec2( textureSize( u_InputTexture, int( pc_Settings.LOD + 1.0f ) ) );
		float scale = 1.0f;

		vec3 upsampleTexture = Upsample( u_InputTexture, pc_Settings.LOD + 1.0f, tc, 1.0f / texSize, scale );

		vec3 last = textureLod( u_InputTexture, tc, pc_Settings.LOD ).rgb;
		color.rgb = last + upsampleTexture;
	}
	else if( pc_Settings.Stage == 1 ) // Upsample
	{
		vec2 texSize = vec2( textureSize( u_BloomTexture, int( pc_Settings.LOD + 1.0f ) ) );
		float scale = 1.0f;

		vec3 upsampleTexture = Upsample( u_BloomTexture, pc_Settings.LOD + 1.0f, tc, 1.0f / texSize, scale );

		vec3 last = textureLod( u_InputTexture, tc,  pc_Settings.LOD ).rgb;
		color.rgb = last + upsampleTexture;
	}

	imageStore( o_Image, ivec2( gl_GlobalInvocationID ), color );
}
