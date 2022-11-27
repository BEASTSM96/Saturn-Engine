#type vertex
#version 450

// Inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

// Uniforms
layout(binding = 0) uniform Matrices 
{
	mat4 ViewProjection;
	mat4 Transform;
} u_Matrices;

void main() 
{	
	gl_Position = u_Matrices.ViewProjection * u_Matrices.Transform * vec4( a_Position, 1.0 );	
}

#type fragment
#version 450

void main() 
{
}