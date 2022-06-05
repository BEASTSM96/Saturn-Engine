﻿#type vertex
#version 430 core

layout( location = 0 ) in vec3 a_Position;
layout( location = 1 ) in vec3 a_Normal;
layout( location = 2 ) in vec3 a_Tangent;
layout( location = 3 ) in vec3 a_Binormal;
layout( location = 4 ) in vec2 a_TexCoord;

layout(set = 0, binding = 0) uniform Matrices
{
    mat4 ViewProjectionMatrix;
} u_Matrices;

layout(set = 0, binding = 1) uniform Transform
{
    mat4 Transform;
} u_Transform;

layout(set = 0, binding = 2) uniform LightMatrix
{
    mat4 LightMatrix;
} u_LightMatrix;

layout(location = 1) out VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
	vec4 LightSpace;
} vs_Output;

void main()
{
	vs_Output.WorldPosition = vec3( u_Transform * vec4( a_Position, 1.0 ) );
	vs_Output.Normal = mat3( u_Transform ) * a_Normal;
	vs_Output.TexCoord = vec2( a_TexCoord.x, 1.0 - a_TexCoord.y );
	vs_Output.WorldNormals = mat3( u_Transform ) * mat3( a_Tangent, a_Binormal, a_Normal );
	vs_Output.WorldTransform = mat3( u_Transform );
	vs_Output.Binormal = a_Binormal;
	vs_Output.LightSpace = u_LightMatrix * vec4( vs_Output.WorldPosition, 1.0 );

	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4( a_Position, 1.0 );
}

#type fragment
#version 430 core

const float PI = 3.141592;
const float Epsilon = 0.00001;

const int LightCount = 1;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3( 0.04 );

struct Light
{
	vec3 Direction;
	vec3 Radiance;
	float Multiplier;
};

layout(location = 1) in VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
	vec4 LightSpace;
} vs_Input;

layout( location = 0 ) out vec4 color;

layout(set = 0, binding = 0) uniform Camera 
{
	vec3 CameraPosition;
} u_Camera;

uniform Light lights;

// PBR texture inputs
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetalnessTexture;
uniform sampler2D u_RoughnessTexture;

// Environment maps
uniform samplerCube u_EnvRadianceTex;
uniform samplerCube u_EnvIrradianceTex;

// BRDF LUT
uniform sampler2D u_BRDFLUTTexture;

layout(push_constant) uniform u_Materials 
{
	float UseAlbedoTexture;
	float UseMetallicTexture;
	float UseRoughnessTexture;
	float UseNormalTexture;

	//

	vec4 AlbedoColor;
	float Metalness;
	float Roughness;
	float RadiancePrefilter;
} pc_Materials;

uniform float u_EnvMapRotation;

// Gamma
uniform float u_Gamma;

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

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
// From: https://learnopengl.com/PBR/Theory (at BRDF, Normal distribution function)
float ndfGGX( float cosLh, float roughness )
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = ( cosLh * cosLh ) * ( alphaSq - 1.0 ) + 1.0;
	return alphaSq / ( PI * denom * denom );
}

// Single term for separable Schlick-GGX below.
// From: https://learnopengl.com/PBR/Theory (at BRDF, Fresnel equation)
float gaSchlickG1( float cosTheta, float k )
{
	return cosTheta / ( cosTheta * ( 1.0 - k ) + k );
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
// From: https://learnopengl.com/PBR/Theory (at BRDF, Fresnel equation)
float gaSchlickGGX( float cosLi, float NdotV, float roughness )
{
	float r = roughness + 1.0;
	float k = ( r * r ) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1( cosLi, k ) * gaSchlickG1( NdotV, k );
}

// From: https://learnopengl.com/PBR/Theory (at BRDF, Fresnel equation)
float GeometrySchlickGGX( float NdotV, float roughness )
{
	float r = ( roughness + 1.0 );
	float k = ( r * r ) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * ( 1.0 - k ) + k;

	return nom / denom;
}

float GeometrySmith( vec3 N, vec3 V, vec3 L, float roughness )
{
	float NdotV = max( dot( N, V ), 0.0 );
	float NdotL = max( dot( N, L ), 0.0 );
	float ggx2 = GeometrySchlickGGX( NdotV, roughness );
	float ggx1 = GeometrySchlickGGX( NdotL, roughness );

	return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick( vec3 F0, float cosTheta )
{
	return F0 + ( 1.0 - F0 ) * pow( 1.0 - cosTheta, 5.0 );
}

vec3 fresnelSchlickRoughness( vec3 F0, float cosTheta, float roughness )
{
	return F0 + ( max( vec3( 1.0 - roughness ), F0 ) - F0 ) * pow( 1.0 - cosTheta, 5.0 );
}

// ---------------------------------------------------------------------------------------------------
// The following code (from Unreal Engine 4's paper) shows how to filter the environment map
// for different roughnesses. This is mean to be computed offline and stored in cube map mips,
// so turning this on online will cause poor performance
float RadicalInverse_VdC( uint bits )
{
	bits = ( bits << 16u ) | ( bits >> 16u );
	bits = ( ( bits & 0x55555555u ) << 1u ) | ( ( bits & 0xAAAAAAAAu ) >> 1u );
	bits = ( ( bits & 0x33333333u ) << 2u ) | ( ( bits & 0xCCCCCCCCu ) >> 2u );
	bits = ( ( bits & 0x0F0F0F0Fu ) << 4u ) | ( ( bits & 0xF0F0F0F0u ) >> 4u );
	bits = ( ( bits & 0x00FF00FFu ) << 8u ) | ( ( bits & 0xFF00FF00u ) >> 8u );
	return float( bits ) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley( uint i, uint N )
{
	return vec2( float( i ) / float( N ), RadicalInverse_VdC( i ) );
}

vec3 ImportanceSampleGGX( vec2 Xi, float Roughness, vec3 N )
{
	float a = Roughness * Roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt( ( 1 - Xi.y ) / ( 1 + ( a * a - 1 ) * Xi.y ) );
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );
	vec3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;
	vec3 UpVector = abs( N.z ) < 0.999 ? vec3( 0, 0, 1 ) : vec3( 1, 0, 0 );
	vec3 TangentX = normalize( cross( UpVector, N ) );
	vec3 TangentY = cross( N, TangentX );
	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float TotalWeight = 0.0;

vec3 PrefilterEnvMap( float Roughness, vec3 R )
{
	vec3 N = R;
	vec3 V = R;
	vec3 PrefilteredColor = vec3( 0.0 );
	int NumSamples = 1024;
	for( int i = 0; i < NumSamples; i++ )
	{
		vec2 Xi = Hammersley( i, NumSamples );
		vec3 H = ImportanceSampleGGX( Xi, Roughness, N );
		vec3 L = 2 * dot( V, H ) * H - V;
		float NoL = clamp( dot( N, L ), 0.0, 1.0 );
		if( NoL > 0 )
		{
			PrefilteredColor += texture( u_EnvRadianceTex, L ).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}

// ---------------------------------------------------------------------------------------------------

vec3 RotateVectorAboutY( float angle, vec3 vec )
{
	angle = radians( angle );
	mat3x3 rotationMatrix ={ vec3( cos( angle ),0.0,sin( angle ) ),
							vec3( 0.0,1.0,0.0 ),
							vec3( -sin( angle ),0.0,cos( angle ) ) };
	return rotationMatrix * vec;
}

vec3 Lighting( vec3 F0 )
{
	vec3 result = vec3( 0.0 );
	for( int i = 0; i < LightCount; i++ )
	{
		vec3 Li = -lights.Direction;
		vec3 Lradiance = lights.Radiance * lights.Multiplier;
		vec3 Lh = normalize( Li + m_Params.View );

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max( 0.0, dot( m_Params.Normal, Li ) );
		float cosLh = max( 0.0, dot( m_Params.Normal, Lh ) );

		vec3 F = fresnelSchlick( F0, max( 0.0, dot( Lh, m_Params.View ) ) );
		float D = ndfGGX( cosLh, m_Params.Roughness );
		float G = gaSchlickGGX( cosLi, m_Params.NdotV, m_Params.Roughness );

		vec3 kd = ( 1.0 - F ) * ( 1.0 - m_Params.Metalness );
		vec3 diffuseBRDF = kd * m_Params.Albedo;

		// Cook-Torrance
		vec3 specularBRDF = ( F * D * G ) / max( Epsilon, 4.0 * cosLi * m_Params.NdotV );

		result += ( diffuseBRDF + specularBRDF ) * Lradiance * cosLi;
	}
	return result;
}

vec3 IBL( vec3 F0, vec3 Lr )
{
	vec3 irradiance = texture( u_EnvIrradianceTex, m_Params.Normal ).rgb;
	vec3 F = fresnelSchlickRoughness( F0, m_Params.NdotV, m_Params.Roughness );
	vec3 kd = ( 1.0 - F ) * ( 1.0 - m_Params.Metalness );
	vec3 diffuseIBL = m_Params.Albedo * irradiance;

	int u_EnvRadianceTexLevels = textureQueryLevels( u_EnvRadianceTex );
	float NoV = clamp( m_Params.NdotV, 0.0, 1.0 );
	vec3 R = 2.0 * dot( m_Params.View, m_Params.Normal ) * m_Params.Normal - m_Params.View;
	vec3 specularIrradiance = textureLod( u_EnvRadianceTex, RotateVectorAboutY( u_EnvMapRotation, Lr ), ( m_Params.Roughness ) * u_EnvRadianceTexLevels ).rgb;

	// Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
	vec2 specularBRDF = texture( u_BRDFLUTTexture, vec2( m_Params.NdotV, 1.0 - m_Params.Roughness ) ).rg;
	vec3 specularIBL = specularIrradiance * ( F * specularBRDF.x + specularBRDF.y );

	return kd * diffuseIBL + specularIBL;
}

void main()
{
	u_Gamma = 2.2;

	// Standard PBR inputs
	m_Params.Albedo = u_AlbedoTexToggle > 0.5 ? texture( u_AlbedoTexture, vs_Input.TexCoord ).rgb : u_AlbedoColor;
	m_Params.Metalness = u_MetalnessTexToggle > 0.5 ? texture( u_MetalnessTexture, vs_Input.TexCoord ).r : u_Metalness;
	m_Params.Roughness = u_RoughnessTexToggle > 0.5 ? texture( u_RoughnessTexture, vs_Input.TexCoord ).r : u_Roughness;
	m_Params.Roughness = max( m_Params.Roughness, 0.05 ); // Minimum roughness of 0.05 to keep specular highlight

	m_ShadowParams.LightSpace = vs_Input.LightSpace;

	// Normals (either from vertex or map)
	m_Params.Normal = normalize( vs_Input.Normal );
	if( u_NormalTexToggle > 0.5 )
	{
		m_Params.Normal = normalize( 2.0 * texture( u_NormalTexture, vs_Input.TexCoord ).rgb - 1.0 );
		m_Params.Normal = normalize( vs_Input.WorldNormals * m_Params.Normal );
	}

	m_Params.View = normalize( u_CameraPosition - vs_Input.WorldPosition );
	m_Params.NdotV = max( dot( m_Params.Normal, m_Params.View ), 0.0 );

	// Specular reflection vector
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix( Fdielectric, m_Params.Albedo, m_Params.Metalness );

	vec3 lightContribution = Lighting( F0 );
	vec3 iblContribution = IBL( F0, Lr );

	vec3 ambient = 0.3 * vec3( lightContribution + iblContribution );

	// diffuse
	vec3 lightDir = normalize( u_LightPos - vs_Input.WorldPosition );
	float diff = max( dot( lightDir, normalize( vs_Input.Normal ) ), 0.0 );
	vec3 diffuse = diff * vec3( 0.3 );

	vec3 lighting = ( ambient + ( diffuse + m_Params.Metalness ) ) * m_Params.Albedo * vec3( lightContribution + iblContribution );

	//color = vec4( lighting, 1.0f );

	color = texture( u_AlbedoTexture, vs_Input.TexCoord );
}