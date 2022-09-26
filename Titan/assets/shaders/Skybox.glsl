// Skybox shader
// Final skybox compsite

#type vertex
#version 450

// Inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

// Outputs
layout(location = 0) out vec3 v_Position;

// Uniforms
layout(binding = 0) uniform Matrices 
{
	mat4 InverseVP;
} u_Matrices;

void main() 
{	
	vec4 position = vec4( a_Position.xy, 1.0, 1.0 );
	
	v_Position = ( u_Matrices.InverseVP * position ).xyz;

	gl_Position = position;
}

#type fragment
#version 450

// Final Color
layout(location = 0) out vec4 FinalColor;

// Inputs
layout(location = 0) in vec3 v_Position;

layout(binding = 1) uniform samplerCube u_CubeTexture;

void main() 
{
	FinalColor = texture( u_CubeTexture, v_Position );
}