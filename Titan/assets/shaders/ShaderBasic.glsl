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
// This should really be in the fragment shader... but we need it for the struct.
uniform vec4 u_LightColor;

out VertexOutput
{
	vec2 TexCoord;
	//
	vec3 WorldPos;
	mat3 WorldTransform;
	//
	vec4 LightColor;
	vec3 Normal;

} vs_Output;

void main()
{
	vs_Output.TexCoord = vec2( a_TexCoord.x, 1.0 - a_TexCoord.y );
	//
	vs_Output.WorldPos       = vec3( u_Transform * vec4( a_Position, 1.0 ) );
	vs_Output.WorldTransform = mat3( u_Transform );
	//
	vs_Output.LightColor = vec4( u_LightColor );
	vs_Output.Normal = mat3( u_Transform ) * a_Normal;
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
	vec4 LightColor;
	vec3 Normal;

} vs_Input;

uniform vec3 u_LightPosition;
uniform sampler2D u_AlbedoTexture;

void main()
{
	float ambient = 0.2f;

	vec3 lightDir = normalize( u_LightPosition - vs_Input.WorldPos );

	float diff = max( dot( lightDir, normalize( vs_Input.Normal ) ), 0.0f );
	vec3 diffuseL = diff + vec3( ambient );

	FinalColor = texture( u_AlbedoTexture, vs_Input.TexCoord ) * vs_Input.LightColor * vec4( diffuseL, 1.0f );
}