#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(set = 0, binding = 0) uniform Matrices 
{
    mat4 ViewProjection;
} u_Matrices;

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
} vs_Output;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main()
{
	// Init members of vs_Output
	vs_Output.Normal     = vec3( a_Normal );
	vs_Output.Tangent    = vec3( a_Tangent );
	vs_Output.Bitangent  = vec3( a_Bitangent );
	vs_Output.Position   = vec3( a_Position );
	vs_Output.TexCoord   = vec2( a_TexCoord );

	vs_Output.WorldNormals = mat3( Transform ) * mat3( a_Tangent, a_Bitangent, a_Normal );
	
	gl_Position = u_Matrices.ViewProjection * Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 450

layout(push_constant) uniform u_Materials
{
	layout(offset = 64) vec3 AlbedoColor;
	float Metalness;
	float Roughness;
	
	float AmbientOcclusion;

	bool UseAlbedoMap;

} pc_Materials;

// Textures
layout (binding = 1) uniform sampler2D u_AlbedoTexture;
layout (binding = 2) uniform sampler2D u_NormalTexture;
layout (binding = 3) uniform sampler2D u_MetallicTexture;
layout (binding = 4) uniform sampler2D u_RoughnessTexture;

layout (location = 0) out vec4 FinalColor;

layout(location = 1) in VertexOutput 
{
	vec3 Normal;
	vec3 Tangent;
	vec3 Bitangent;
	vec3 Position;
	vec2 TexCoord;
	mat3 WorldNormals;
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
	m_Params.Albedo = texture( u_AlbedoTexture, vs_Input.TexCoord ).rgb * pc_Materials.AlbedoColor;
	
	float Ambient = 0.20;

	/*
	vec3 normal = normalize( 2.0 * texture( u_NormalTexture, vs_Input.TexCoord ).rgb - 1.0 );
	normal = normalize( vs_Input.WorldNormals * normal );
	
	vec3 lightDir = vec3( -0.5, 0.5, -0.5 );
	float lightIntensity = clamp( dot( lightDir, normal ), 0.1, 1.0 );
	*/

	FinalColor = vec4( m_Params.Albedo, 1.0 );
	//FinalColor.rgb *= Ambient;
}