// Scene Composite shader
// Really it's the texture pass.

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

vec3 ACESTonemap(vec3 color)
{
	mat3 m1 = mat3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
		1.60475, -0.10208, -0.00327,
		-0.53108, 1.10813, -0.07276,
		-0.07367, -0.00605, 1.07602
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

void main()
{
	const float gamma = 2.2;
	const float pureWhite = 1.0;

	vec3 GeometryPassColor = texture( u_GeometryPassTexture, vs_Input.TexCoord ).rgb;

	/*
	float luminance = dot(GeometryPassColor, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);
	
	vec3 mappedColor = ( mappedLuminance / luminance ) * GeometryPassColor;
	FinalColor = vec4( pow( mappedColor, vec3( 1.0 / gamma ) ), 1.0 );
	*/

	GeometryPassColor = ACESTonemap( GeometryPassColor );
	GeometryPassColor = GammaCorrect( GeometryPassColor.rgb, gamma );

	FinalColor = vec4( GeometryPassColor, 1.0 );
}