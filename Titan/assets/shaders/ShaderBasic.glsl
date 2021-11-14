#type vertex
#version 430 core

layout( location = 0 ) in vec3 a_Position;
layout( location = 1 ) in vec3 a_Normal;
layout( location = 2 ) in vec3 a_Tangent;
layout( location = 3 ) in vec3 a_Binormal;
layout( location = 4 ) in vec2 a_TexCoord;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;
uniform mat4 u_LightMatrix;

out VertexOutput
{
	vec2 TexCoord;
	//
	vec3 WorldPos;
	mat3 WorldTransform;
	//

} vs_Output;

void main()
{
	vs_Output.TexCoord = vec2( a_TexCoord.x, 1.0 - a_TexCoord.y );
	//
	vs_Output.WorldPos       = vec3( u_Transform * vec4( a_Position, 1.0 ) );
	vs_Output.WorldTransform = mat3( u_Transform );
	//

	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 430 core

layout( location = 0 ) out vec4 FinalColor;

in VertexOutput
{
	vec2 TexCoord;
	//
	vec3 WorldPos;
	mat3 WorldTransform;
	//

} vs_Input;

uniform sampler2D u_AlbedoTexture;

void main()
{
	FinalColor = texture( u_AlbedoTexture, vs_Input.TexCoord );
}