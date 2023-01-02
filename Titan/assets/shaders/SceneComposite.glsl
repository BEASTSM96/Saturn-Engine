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

void main()
{
	const float gamma = 2.2;
	const float pureWhite = 1.0;

	vec3 GeometryPassColor = texture( u_GeometryPassTexture, vs_Input.TexCoord ).rgb;

	// Bloom
	float sampleScale = 0.5;
	ivec2 texSize = textureSize(u_BloomTexture, 0);
	vec2 fTexSize = vec2(float(texSize.x), float(texSize.y));
	vec3 bloom = texture(u_BloomTexture, vs_Input.TexCoord ).rgb * 1.0f;
	vec3 dirt = texture( u_BloomDirtTexture, vs_Input.TexCoord ).rgb * 20.0f; 

	GeometryPassColor += bloom;
	//GeometryPassColor += bloom * dirt;

	float luminance = dot(GeometryPassColor, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);
	
	vec3 mappedColor = ( mappedLuminance / luminance ) * GeometryPassColor;

	GeometryPassColor = GammaCorrect( mappedColor, gamma );

	FinalColor = vec4( GeometryPassColor, 1.0 );
}