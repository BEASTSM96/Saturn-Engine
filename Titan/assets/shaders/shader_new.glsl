// PBR Shader test
// Based on PBR_Static, but with my one stuff

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
	mat4 View;
} u_Matrices;

layout(binding = 1) uniform LightData
{
	mat4 LightMatrix[4];
};

layout(push_constant) uniform u_Transform
{
	mat4 Transform;
};

struct VertexOutput 
{
	vec3 Normal;
	vec3 Tangent;
	vec3 Bitangent;
	vec3 Position;
	vec2 TexCoord;
	mat3 WorldNormals;

	vec4 ShadowMapCoords[4];
	vec3 ViewPosition;
};

layout( location = 1 ) out VertexOutput vs_Output;

void main()
{
	// Init members of vs_Output
	vs_Output.Normal     = mat3( Transform ) * a_Normal;
	vs_Output.Tangent    = vec3( a_Tangent );
	vs_Output.Bitangent  = vec3( a_Bitangent );
	vs_Output.TexCoord   = vec2( a_TexCoord );
	vs_Output.Position = vec3( Transform * vec4( a_Position, 1.0 ) );

	vs_Output.WorldNormals = mat3( Transform ) * mat3( a_Tangent, a_Bitangent, a_Normal );

	vs_Output.ShadowMapCoords[0] = LightMatrix[0] * vec4( vs_Output.Position, 1.0 );
	vs_Output.ShadowMapCoords[1] = LightMatrix[1] * vec4( vs_Output.Position, 1.0 );
	vs_Output.ShadowMapCoords[2] = LightMatrix[2] * vec4( vs_Output.Position, 1.0 );
	vs_Output.ShadowMapCoords[3] = LightMatrix[3] * vec4( vs_Output.Position, 1.0 );
	
	vs_Output.ViewPosition = vec3( u_Matrices.View * vec4( vs_Output.Position, 1.0 ) );
	
	gl_Position = u_Matrices.ViewProjection * Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 450

const float PI = 3.141592;
const float Epsilon = 0.00001;

const int LightCount = 1;

const vec3 Fdielectric = vec3( 0.04 );

struct Light
{
	vec3 Direction;
	vec3 Radiance;
	float Multiplier;
};

layout(push_constant) uniform pc_Materials
{
	layout(offset = 64) vec3 AlbedoColor;
	float UseNormalMap;
	
	float Metalness;
	float Roughness;

} u_Materials; 

layout(set = 0, binding = 2) uniform Camera 
{
	Light Lights;
	vec3 CameraPosition;
} u_Camera;

layout(set = 0, binding = 3) uniform ShadowData 
{
	vec4 CascadeSplits;
};

// Textures
layout (set = 0, binding = 4) uniform sampler2D u_AlbedoTexture;
layout (set = 0, binding = 5) uniform sampler2D u_NormalTexture;
layout (set = 0, binding = 6) uniform sampler2D u_MetallicTexture;
layout (set = 0, binding = 7) uniform sampler2D u_RoughnessTexture;

// Set 1, owned by renderer, environment settings.

layout (set = 1, binding = 8) uniform sampler2DArray u_ShadowMap;

layout (location = 0) out vec4 FinalColor;

struct VertexOutput 
{
	vec3 Normal;
	vec3 Tangent;
	vec3 Bitangent;
	vec3 Position;
	vec2 TexCoord;
	mat3 WorldNormals;

	vec4 ShadowMapCoords[4];
	vec3 ViewPosition;
};

layout( location = 1 ) in VertexOutput vs_Input;

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
};

PBRParameters m_Params;

//////////////////////////////////////////////////////////////////////////
// SHADOWS

float GetShadowBias() 
{
	const float MINIMUM_SHADOW_BIAS = 0.002;
	float bias = max( MINIMUM_SHADOW_BIAS * ( 1.0 - dot( m_Params.Normal, u_Camera.Lights.Direction ) ), MINIMUM_SHADOW_BIAS );
	return bias;
}

float HardShadows( sampler2DArray ShadowMap, vec3 ShadowCoords, uint index ) 
{
	float bias = GetShadowBias();
	float map = texture( ShadowMap, vec3( ShadowCoords.xy * 0.5 + 0.5, index ) ).x;
	return step( ShadowCoords.z, map + bias ) * 1.0;
}

//////////////////////////////////////////////////////////////////////////
// LIGHTING

// Shlick's approximation of the Fresnel factor.
vec3 FresnelSchlick( vec3 F0, float cosTheta ) 
{
	return F0 + ( 1.0 - F0 ) * pow( 1.0 - cosTheta, 5.0 );
}

float NDFGGX( float cosLh, float r ) 
{	
	// alpha = roughness^2;
	float a = r * r;
	float a2 = a * a;

	float denom = ( cosLh * cosLh ) * ( a2 - 1.0 ) + 1.0;
	return a2 / ( PI * denom * denom );
}

float gaSchlickG1( float cosTheta, float k )
{
	return cosTheta / ( cosTheta * ( 1.0 - k ) + k );
}

float gaSchlickGGX( float cosLi, float NdotV, float roughness ) 
{
	float r = roughness + 1.0;
	float k = ( r * r ) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1( cosLi, k ) * gaSchlickG1( NdotV, k );
}

vec3 Lighting( vec3 F0 ) 
{
	vec3 result = vec3( 0.0 );
	
	for( int i = 0; i < LightCount; i++ )
	{
		vec3 Li = u_Camera.Lights.Direction;
		vec3 Lradiance = u_Camera.Lights.Radiance * u_Camera.Lights.Multiplier;
		vec3 Lh = normalize( Li + m_Params.View );

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max( 0.0, dot( m_Params.Normal, Li ) );
		float cosLh = max( 0.0, dot( m_Params.Normal, Lh ) );

		vec3 F = FresnelSchlick( F0, max( 0.0, dot( Lh, m_Params.View ) ) );
		float D = NDFGGX( cosLh, m_Params.Roughness );
		float G = gaSchlickGGX( cosLi, m_Params.NdotV, m_Params.Roughness );

		vec3 kd = ( 1.0 - F ) * ( 1.0 - m_Params.Metalness );
		vec3 DiffuseBRDF = kd * m_Params.Albedo;

		vec3 SpecularBRDF = ( F * D * G ) / max( Epsilon, 4.0 * cosLi * m_Params.NdotV );
		result += ( DiffuseBRDF + SpecularBRDF ) * Lradiance * cosLi;
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

void main() 
{
	m_Params.Albedo = texture( u_AlbedoTexture, vs_Input.TexCoord ).rgb * u_Materials.AlbedoColor;
	m_Params.Metalness = texture( u_MetallicTexture, vs_Input.TexCoord ).r * u_Materials.Metalness;
	m_Params.Roughness = texture( u_RoughnessTexture, vs_Input.TexCoord ).r * u_Materials.Roughness;
	m_Params.Roughness = max( m_Params.Roughness, 0.05 ); // Minimum roughness of 0.05 to keep specular highlight

	float Ambient = 0.20;
	
	// Normals (either from vertex or map)
	m_Params.Normal = normalize( vs_Input.Normal );
	
	if( u_Materials.UseNormalMap > 0.5 )
	{
		m_Params.Normal = normalize( 2.0 * texture( u_NormalTexture, vs_Input.TexCoord ).rgb - 1.0 );
		m_Params.Normal = normalize( vs_Input.WorldNormals * m_Params.Normal );
	}

	m_Params.View = normalize( u_Camera.CameraPosition - vs_Input.Position );
	m_Params.NdotV = max( dot( m_Params.Normal, m_Params.View ), 0.0 );

	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

	vec3 F0 = mix( Fdielectric, m_Params.Albedo, m_Params.Metalness );

	// SHADOWS

	uint cascadeIndex = 0;
	
	const uint SHADOW_MAP_CASCADES = 4;
	
	for( uint i = 0; i < SHADOW_MAP_CASCADES - 1; i++ )
	{
		if( vs_Input.ViewPosition.z < CascadeSplits[ i ] )
			cascadeIndex = i + 1;
	}

	vec3 ShadowCoords = (vs_Input.ShadowMapCoords[cascadeIndex].xyz / vs_Input.ShadowMapCoords[cascadeIndex].w);
	
	float ShadowAmount = HardShadows( u_ShadowMap, ShadowCoords, cascadeIndex );

	// OUTPUT
	vec3 Lighting = Lighting( F0 ) * ShadowAmount;

	FinalColor = vec4( Lighting, 1.0 );

	switch(cascadeIndex)
	{
	case 0:
		FinalColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
		break;
	case 1:
		FinalColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
		break;
	case 2:
		FinalColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
		break;
	case 3:
		FinalColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
		break;
	}

}