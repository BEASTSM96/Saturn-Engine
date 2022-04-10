#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_FragTexCoord;

layout(push_constant) uniform Push {
	mat4 Transform;
	mat4 VPM;
} push;

layout(binding = 0) uniform UniformBufferObject {
    mat4 Model;
    mat4 View;
    mat4 Proj;
    mat4 VP;
} ubo;


void main()
{
	v_FragTexCoord = a_TexCoord;

	mat4 VP = ubo.View * ubo.Proj;

	gl_Position = ubo.Proj * ubo.View * ubo.Model * vec4( a_Position, 1.0 );
}