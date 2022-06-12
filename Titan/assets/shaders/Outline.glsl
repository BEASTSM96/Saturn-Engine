// Outline Shader

#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(binding = 0) uniform Matrices 
{
	mat4 ViewProjection;
	mat4 Transform;
} u_Matrices;

layout(location = 1) out VertexOutput 
{
	vec2 TexCoord;
} vs_Output;

void main()
{
	gl_Position = u_Matrices.ViewProjection * u_Matrices.Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 FinalColor;

void main()
{
	FinalColor = vec4(1.0, 0.5, 0.0, 1.0);
}