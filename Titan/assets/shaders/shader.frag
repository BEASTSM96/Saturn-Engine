#version 450

layout (location = 0) in vec2 v_FragTexCoord;
layout (binding = 1) uniform sampler2D texSampler;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 Transform;
	mat4 VPM;
} push;

void main() {
	outColor = texture( texSampler, v_FragTexCoord );
}