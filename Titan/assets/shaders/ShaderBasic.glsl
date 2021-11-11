#type vertex
#version 430 core

layout( location = 0 ) in vec3 a_Position;
layout( location = 1 ) in vec3 a_Normal;
layout( location = 2 ) in vec3 a_Tangent;
layout( location = 3 ) in vec3 a_Binormal;
layout( location = 4 ) in vec2 a_TexCoord;

out vec2 TexCoord;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;
uniform mat4 u_LightMatrix;

void main()
{
	TexCoord = vec2( a_TexCoord.x, 1.0 - a_TexCoord.y );
	
	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 430 core

layout( location = 0 ) out vec4 FinalColor;

in vec2 TexCoord;

uniform sampler2D u_AlbedoTexture;

void main()
{
	FinalColor = texture( u_AlbedoTexture, TexCoord );
}