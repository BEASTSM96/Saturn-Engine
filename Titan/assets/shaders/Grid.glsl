// Grid Shader

#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in float a_Scale;
layout(location = 3) in float a_Res;

layout(binding = 0) uniform Matrices {
	mat4 u_ViewProjection;
	mat4 u_Transform;
} u_Matrices;

layout(location = 1) out VertexOutput 
{
	float Scale;
	float Res;
} vs_Output;

layout(location = 0) out vec2 v_TexCoord;

void main()
{
	vs_Output.Scale = a_Scale;
	vs_Output.Res = a_Res;

	vec4 position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
	gl_Position = position;

	v_TexCoord = a_TexCoord;
}

#type fragment
#version 430

layout(location = 0) out vec4 FinalColor;
layout(location = 0) in vec2 v_TexCoord;

layout(location = 1) in VertexOutput 
{
	float Scale;
	float Res;
} vs_Input;

float grid(vec2 st)
{
	float res = vs_Input.Scale;
	vec2 grid =  fract( (st / 2) );
	return step(res, grid.x * 1) * step(res, grid.y * 1);
}

void main()
{
	float scale = vs_Input.Scale;
	float resolution = vs_Input.Res;
	float x = grid(v_TexCoord * scale);

	FinalColor = vec4(vec3(0.2), 0.5) * (1.0 - x);
}