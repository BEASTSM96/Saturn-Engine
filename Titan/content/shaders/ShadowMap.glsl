// Shadow Map shader

#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(set = 0, binding = 0) uniform Matrices
{
	mat4 ViewProjection[4];
} u_Matrices;

layout(push_constant) uniform u_Transform
{
	mat4 Transform;
	uint CascadeIndex;
};

void main()
{
	gl_Position = u_Matrices.ViewProjection[ CascadeIndex ] * Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 430

void main()
{
}