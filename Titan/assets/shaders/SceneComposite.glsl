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

void main()
{
	const float gamma = 2.2;
	const float pureWhite = 1.0;

	vec3 GeometryPassColor = texture( u_GeometryPassTexture, vs_Input.TexCoord ).rgb;

	// From "Photographic Tone Reproduction for Digital Images", eq. 4

	float luminance = dot(GeometryPassColor, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);
	
	vec3 mappedColor = ( mappedLuminance / luminance ) * GeometryPassColor;
	FinalColor = vec4( pow( mappedColor, vec3( 1.0 / gamma ) ), 1.0 );
}