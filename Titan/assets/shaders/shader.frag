#version 450

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 Transform;
	mat4 VPM;
} push;

void main() {
  outColor = vec4(0.0, 0.0, 0.0, 1.0);
}