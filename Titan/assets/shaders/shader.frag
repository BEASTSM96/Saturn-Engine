#version 450

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 Transform;
	vec2 Offset;
	vec3 Color;
	mat4 VPM;
} push;

void main() {
  outColor = vec4(push.Color, 1.0);
}