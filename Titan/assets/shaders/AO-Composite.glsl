#type vertex
#version 450

// Inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

// Outputs
layout(location = 0) out vec2 o_TexCoord;

void main() 
{	
	vec4 position = vec4( a_Position.xy, 0.0, 1.0 );
	gl_Position = position;

	o_TexCoord = a_TexCoord;
}

#type fragment
#version 450

layout( set = 0, binding = 1 ) uniform sampler2D u_AOTexture;

layout( location = 0 ) in vec2 o_TexCoord;
layout( location = 0 ) out vec4 FinalColor;

void main() 
{
	vec4 beforeAO = FinalColor;

	float x = texture( u_AOTexture, o_TexCoord ).r;

	FinalColor.rgb = x.rrr;
	FinalColor.rgb *= beforeAO.rgb;
}