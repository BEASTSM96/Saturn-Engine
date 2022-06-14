// Shadow Map shader

#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

#define SHADOW_CASCADE_COUNT 1

layout(set = 0, binding = 0) uniform Matrices
{
	mat4[SHADOW_CASCADE_COUNT] ViewProjections;
} u_Matrices;

layout(set = 0, binding = 1) uniform RendererData
{
	mat4 Transform;
};

layout(push_constant) uniform u_CascadeInfo 
{
	uint CascadeIndex;
} pc_CascadeInfo;

void main()
{
	gl_Position = u_Matrices.ViewProjections[ pc_CascadeInfo.CascadeIndex ] * Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 430

layout( location = 0 ) out vec4 FinalColor;

void main()
{
}