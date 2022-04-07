#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(push_constant) uniform Push {
	mat4 Transform;
	vec2 Offset;
	vec3 Color;
	mat4 VPM;
} push;

void main()
{
	gl_Position = push.VPM * push.Transform * vec4( position, 1.0 );
}