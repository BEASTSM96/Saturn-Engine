// Debug Line Shader

#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(binding = 0) uniform Matrices 
{
	mat4 ViewProjection;
} u_Matrices;

struct VertOut 
{
	vec4 Color;
};

layout(location = 0) out VertOut vs_Output;

void main()
{
	vs_Output.Color = a_Color;

	gl_Position = u_Matrices.ViewProjection * mat4(1.0) * vec4(a_Position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 FinalColor;

struct VertOut 
{
	vec4 Color;
};

layout(location = 0) in VertOut vs_Input;

void main()
{
	FinalColor = vs_Input.Color;
}