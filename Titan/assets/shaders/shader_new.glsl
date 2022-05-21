#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(set = 0, binding = 0) uniform Matrices 
{
    mat4 Transform;
    mat4 ViewProjection;
} u_Matrices;

layout(push_constant) uniform u_Materials
{
	float UseAlbedoTexture;
	float UseMetallicTexture;
	float UseRoughnessTexture;
	float UseNormalTexture;

	//

	vec4 AlbedoColor;
	vec4 MetallicColor;
	vec4 RoughnessColor;
} pc_Materials;

layout(location = 1) out VertexOutput 
{
	vec3 Normal;
	vec3 Tangent;
	vec3 Bitangent;
	vec3 Position;
	vec2 TexCoord;
} vs_Output;

void main()
{
	// Init members of vs_Output
	vs_Output.Normal     = vec3( a_Normal );
	vs_Output.Tangent    = vec3( a_Tangent );
	vs_Output.Bitangent  = vec3( a_Bitangent );
	vs_Output.Position   = vec3( a_Position );
	vs_Output.TexCoord   = vec2( a_TexCoord );

	gl_Position = u_Matrices.ViewProjection * u_Matrices.Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 450


layout(set = 0, binding = 0) uniform Matrices 
{
    mat4 Transform;
    mat4 ViewProjection;
} u_Matrices;

layout(push_constant) uniform u_Materials
{
	float UseAlbedoTexture;
	float UseMetallicTexture;
	float UseRoughnessTexture;
	float UseNormalTexture;

	//

	vec4 AlbedoColor;
	vec4 MetallicColor;
	vec4 RoughnessColor;
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
} vs_Input;

void main() 
{
	if( pc_Materials.UseAlbedoTexture != 0.5 ) 
	{
		FinalColor = texture( u_AlbedoTexture, vs_Input.TexCoord );
	}
	else
	{
		FinalColor = vec4( pc_Materials.AlbedoColor );
	}
}