// Scene Composite shader

#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 1) out VertexOutput 
{
	vec3 Position;
	vec2 TexCoord;
} vs_Output;

void main()
{
	vs_Output.Position = a_Position;
	vs_Output.TexCoord = a_TexCoord;

	vec4 position = vec4( a_Position.xy, 0.0, 1.0 );
	
	gl_Position = position;
}

#type fragment
#version 450

layout (binding = 0) uniform sampler2D u_GeometryPassTexture;
layout (binding = 1) uniform sampler2D u_BloomTexture;
layout (binding = 2) uniform sampler2D u_BloomDirtTexture;
layout (binding = 3) uniform sampler2D u_DepthTexture;

layout(location = 0) out vec4 FinalColor;

layout(location = 1) in VertexOutput 
{
	vec3 Position;
	vec2 TexCoord;
} vs_Input;

vec3 GammaCorrect( vec3 color, float gamma ) 
{
	return pow( color, vec3( 1.0f / gamma ) );
}

vec3 UpsampleTent9(sampler2D tex, float lod, vec2 uv, vec2 texelSize, float radius)
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

	return result * (1.0f / 16.0f);
}

vec3 Reinhard( vec3 x )
{
	return x / (1.0 + x);
}

float Luminance( vec3 colr )  
{
	return dot( colr, vec3(0.2126, 0.7152, 0.0722) );
}

vec3 WhiteLuminance( vec3 colr ) 
{
	const float pureWhite = 1.0;
	float l = Luminance( colr );
	float mappedLuminance = (l * (1.0 + l / (pureWhite * pureWhite))) / (1.0 + l);
	
	return mappedLuminance / l * colr;
}

vec3 ACES( vec3 colr ) 
{
	mat3 m1 = mat3( 0.59719, 0.07600, 0.02840, 0.35458, 0.90834, 0.13383, 0.04823, 0.01566, 0.83777 );
	mat3 m2 = mat3( 1.60475, -0.10208, -0.00327, -0.53108, 1.10813, -0.07276, -0.07367, -0.00605, 1.07602 );

	vec3 v = m1 * colr;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

void main()
{
	const float gamma = 2.2;

	vec3 GeometryPassColor = texture( u_GeometryPassTexture, vs_Input.TexCoord ).rgb;

	// Bloom
	float sampleScale = 0.5;
	ivec2 texSize = textureSize(u_BloomTexture, 0);
	vec2 fTexSize = vec2(float(texSize.x), float(texSize.y));
	vec3 bloom = UpsampleTent9( u_BloomTexture, 0, vs_Input.TexCoord, 1.0f / fTexSize, sampleScale );
	vec3 dirt = texture( u_BloomDirtTexture, vs_Input.TexCoord ).rgb * 20.0f; 

	GeometryPassColor += bloom;

	GeometryPassColor = ACES( GeometryPassColor );
	GeometryPassColor = GammaCorrect( GeometryPassColor, gamma );

	/*
	float d = texture( u_DepthTexture, vs_Input.TexCoord ).r;
	
	float fogStart = 2.0f;
	float fogFalloff = 40.0f;

	float fogAmount = smoothstep( fogStart, fogStart + fogFalloff, d );

	vec3 fogColor = vec3(0.0f);

	GeometryPassColor = mix( GeometryPassColor, fogColor, fogAmount );
	*/
	GeometryPassColor *= 0.80;

	vec2 coord = vs_Input.TexCoord * 2.0 - 1.0;
	float dist = length( coord );
    dist = sqrt( dist );
    float vi = 1.0 - dist;
    vi += 0.7;
    dist = clamp( vi, 0.0, 1.0 );

	GeometryPassColor *= dist;

	FinalColor = vec4( GeometryPassColor, 1.0 );
}