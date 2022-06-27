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
	vec4 ShadowMapCoordsBiased;
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
	vs_Output.ShadowMapCoordsBiased = LightMatrix * vec4( vs_Output.Position, 1.0 );
	
	gl_Position = u_Matrices.ViewProjection * Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 450

layout(push_constant) uniform pc_Materials
{
	layout(offset = 64) vec3 AlbedoColor;
} u_Materials; 

// Textures
layout (set = 0, binding = 2) uniform sampler2D u_AlbedoTexture;
layout (set = 0, binding = 3) uniform sampler2D u_NormalTexture;
layout (set = 0, binding = 4) uniform sampler2D u_MetallicTexture;
layout (set = 0, binding = 5) uniform sampler2D u_RoughnessTexture;

// Set 1, owned by renderer, environment settings.

layout (set = 1, binding = 6) uniform sampler2D u_ShadowMap;

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
	vec4 ShadowMapCoordsBiased;
} vs_Input;

struct PBRParameters 
{
	vec3 Albedo;
	float Roughness;
	float Metalness;
};

PBRParameters m_Params;

void main() 
{
	m_Params.Albedo = texture( u_AlbedoTexture, vs_Input.TexCoord ).rgb * u_Materials.AlbedoColor;
	
	float Ambient = 0.20;
	
	// Normals (either from vertex or map)
	vec3 normal = normalize( vs_Input.Normal );

	normal = normalize( 2.0 * texture( u_NormalTexture, vs_Input.TexCoord ).rgb - 1.0 );
	normal = normalize( vs_Input.WorldNormals * normal );
	
	vec3 lightDir = vec3( -0.5, 0.5, -0.5 );
	float lightIntensity = max( dot( lightDir, normal ), 0.0 );

	vec3 ShadowMapCoords = ( vs_Input.ShadowMapCoords.xyz / vs_Input.ShadowMapCoords.w );
	vec3 ShadowMapCoordsBiased = ( vs_Input.ShadowMapCoordsBiased.xyz / vs_Input.ShadowMapCoordsBiased.w );

	float LightSize = 0.5;
	float ShadowAmount = 0.0;

	FinalColor = vec4( m_Params.Albedo, 1.0 );
	FinalColor.rgb *= lightIntensity + Ambient;
}