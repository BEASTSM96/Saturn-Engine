#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(binding = 0) uniform Matrices 
{
    mat4 ViewProjection;
} u_Matrices;

layout(binding = 1) uniform LightData
{
    mat4 LightMatrix;
};

layout(push_constant) uniform u_Transform
{
    mat4 Transform;
};

layout(location = 1) out VertexOutput 
{
	vec3 Normal;
	vec3 Tangent;
	vec3 Bitangent;
	vec3 Position;
	vec2 TexCoord;
	mat3 WorldNormals;
	vec4 ShadowMapCoords;
} vs_Output;

void main()
{
	// Init members of vs_Output
	vs_Output.Normal     = mat3( Transform ) * a_Normal;
	vs_Output.Tangent    = vec3( a_Tangent );
	vs_Output.Bitangent  = vec3( a_Bitangent );
	vs_Output.TexCoord   = vec2( a_TexCoord );
	vs_Output.Position = vec3( Transform * vec4( a_Position, 1.0 ) );

	vs_Output.WorldNormals = mat3( Transform ) * mat3( a_Tangent, a_Bitangent, a_Normal );

	vs_Output.ShadowMapCoords = LightMatrix * vec4( vs_Output.Position, 1.0 );
	
	gl_Position = u_Matrices.ViewProjection * Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 450

const float PI = 3.141592;
const float Epsilon = 0.00001;

const int LightCount = 1;

struct Light
{
	vec3 Direction;
	vec3 Radiance;
	float Multiplier;
};

layout(push_constant) uniform pc_Materials
{
	layout(offset = 64) vec3 AlbedoColor;
} u_Materials; 

layout(set = 0, binding = 2) uniform Camera 
{
	Light Lights;
	vec3 CameraPosition;
} u_Camera;

// Textures
layout (set = 0, binding = 3) uniform sampler2D u_AlbedoTexture;
layout (set = 0, binding = 4) uniform sampler2D u_NormalTexture;
layout (set = 0, binding = 5) uniform sampler2D u_MetallicTexture;
layout (set = 0, binding = 6) uniform sampler2D u_RoughnessTexture;

// Set 1, owned by renderer, environment settings.

layout (set = 1, binding = 7) uniform sampler2D u_ShadowMap;

layout (location = 0) out vec4 FinalColor;

layout(location = 1) in VertexOutput 
{
	vec3 Normal;
	vec3 Tangent;
	vec3 Bitangent;
	vec3 Position;
	vec2 TexCoord;
	mat3 WorldNormals;
	vec4 ShadowMapCoords;
} vs_Input;

struct PBRParameters 
{
	vec3 Albedo;
	vec3 Normal;
	
	float Roughness;
	float Metalness;
};

PBRParameters m_Params;

float GetShadowBias() 
{
	const float MINIMUM_SHADOW_BIAS = 0.002;
	float bias = max( MINIMUM_SHADOW_BIAS * ( 1.0 - dot( m_Params.Normal, u_Camera.Lights.Direction ) ), MINIMUM_SHADOW_BIAS );
	return bias;
}

float HardShadows( sampler2D ShadowMap, vec3 ShadowCoords ) 
{
	float bias = GetShadowBias();
	float map = texture( ShadowMap, ShadowCoords.xy * 0.5 + 0.5 ).x;
	return step( ShadowCoords.z, map + bias ) * 1.0;
}

void main() 
{
	m_Params.Albedo = texture( u_AlbedoTexture, vs_Input.TexCoord ).rgb * u_Materials.AlbedoColor;
	
	float Ambient = 0.20;
	
	// Normals (either from vertex or map)
	m_Params.Normal = normalize( vs_Input.Normal );

	// SHADOWS

	vec3 ShadowCoords = (vs_Input.ShadowMapCoords.xyz / vs_Input.ShadowMapCoords.w);
	
	float ShadowAmount = HardShadows( u_ShadowMap, ShadowCoords );

	// OUTPUT
	FinalColor = vec4( m_Params.Albedo * ShadowAmount, 1.0 );
}