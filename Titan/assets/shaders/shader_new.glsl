#type vertex
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

#type fragment
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