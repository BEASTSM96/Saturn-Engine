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

	// Flip textures.
	vs_Output.TexCoord = vec2( a_TexCoord.s, 1.0 - a_TexCoord.t );

	vec4 position = vec4( a_Position.xy, 0.0, 1.0 );
	
	gl_Position = position;
	gl_Position.y *= -1.0;
	gl_Position.z = ( gl_Position.z + gl_Position.w ) / 2.0;
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
	float gamma = 2.2;

	vec4 GeometryPassColor = texture( u_GeometryPassTexture, vs_Input.TexCoord );

	// Gamma correction.
	GeometryPassColor.rgb = GammaCorrect( GeometryPassColor.rgb, gamma );

	FinalColor = GeometryPassColor;
}