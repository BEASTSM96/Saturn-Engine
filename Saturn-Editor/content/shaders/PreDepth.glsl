#type vertex
#version 450

// Inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(set = 0, binding = 0) uniform Matrices
{
	mat4 ViewProjection;
} u_Matrices;

layout(push_constant) uniform u_Transform
{
	mat4 Transform;
};

void main() 
{
	gl_Position = u_Matrices.ViewProjection * Transform * vec4( a_Position, 1.0 );
}