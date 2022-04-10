#version 450

layout (location = 0) in vec2 v_FragTexCoord;
layout (binding = 1) uniform sampler2D texSampler;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 Transform;
	mat4 VPM;
} push;

void main() {
	
	//vec3 Color = vec3( 1.0, 0.0, 1.0 );
	
	//outColor = vec4( Color * texture( texSampler, v_FragTexCoord ).rgb, 1.0 );
	outColor = texture( texSampler, v_FragTexCoord );
}