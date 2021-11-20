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
	mat3 WorldNormals;
	//
	vec4 LightColor;
	vec3 Normal;
	//
	// We dont need this but we may need it.
	mat3 TBN;
} vs_Output;

void main()
{
	mat3 normalMatrix = transpose( inverse( mat3( u_Transform ) ) );
	vec3 T = normalize( normalMatrix * a_Tangent );
	vec3 N = normalize( normalMatrix * a_Normal );
	T = normalize( T - dot( T, N ) * N );
	vec3 B = cross( N, T );

	vs_Output.TexCoord = vec2( a_TexCoord.x, 1.0 - a_TexCoord.y );
	//
	vs_Output.WorldPos       = vec3( u_Transform * vec4( a_Position, 1.0 ) );
	vs_Output.WorldTransform = mat3( u_Transform );
	vs_Output.WorldNormals   = mat3( u_Transform ) * mat3( a_Tangent, a_Binormal, a_Normal );
	//
	vs_Output.LightColor = vec4( u_LightColor );
	vs_Output.Normal = mat3( u_Transform ) * a_Normal;
	//
	vs_Output.TBN = transpose( mat3( T, B, N ) );

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
	mat3 WorldNormals;
	//
	vec4 LightColor;
	vec3 Normal;
	//
	mat3 TBN;
} vs_Input;

uniform vec3 u_LightPosition;
uniform vec3 u_CameraPosition;

uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_SpecularTexture;
uniform sampler2D u_NormalTexture;

vec3 TLightPos;
vec3 TViewPos;
vec3 TPos;

void main()
{
	TLightPos = vs_Input.TBN  * u_LightPosition;
	TViewPos  = vs_Input.TBN  * u_CameraPosition;
	TPos =      vs_Input.TBN  * vs_Input.WorldPos;

	vec3 normalRGB = texture( u_NormalTexture, vs_Input.TexCoord ).rgb;
	normalRGB = normalize( normalRGB * 2.0 - 1.0 );

	float ambient = 0.2f;

	vec3 lightDir = normalize( TLightPos - TPos );
	vec3 viewDir = normalize( TViewPos - TPos );

	float diff = max( dot( lightDir, normalRGB ), 0.0f );
	vec3 diffuseL = diff + vec3( ambient );
	
	float specularL = 0.50f;
	vec3 reflectionDir = reflect( -lightDir, normalRGB );
	vec3 halfwayDir = normalize( lightDir + viewDir );
	float specAmount = pow( max( dot( normalRGB, halfwayDir ), 0.0f ), 32 );
	float specular = specAmount * specularL;

	FinalColor = texture( u_AlbedoTexture, vs_Input.TexCoord ) * vs_Input.LightColor * vec4( diffuseL, 1.0f ) + texture( u_SpecularTexture, vs_Input.TexCoord ).r * specular;
}