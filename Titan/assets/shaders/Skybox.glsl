// Preetham Skybox shader

#type vertex
#version 450

// Inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(binding = 0) uniform Matrices 
{
	mat4 View;
	mat4 Projection;
	vec4 ViewPos;
	
	float Turbidity;
	float Azimuth;
	float Inclination;
			
} u_Matrices;

layout(location = 1) out VertexOutput 
{
	vec3 TexCoord;
} vs_Output;

void main() 
{
	// Inverse matrices.
	mat4 InvView = inverse( u_Matrices.View );
	mat4 InvProj = inverse( u_Matrices.Projection );
	mat4 InvViewProj = InvView * InvProj;

	vec4 position = vec4( a_Position.xy, 1.0, 1.0 );
	
	gl_Position = position;	
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

	vs_Output.TexCoord = ( InvViewProj * position ).xyz;
}

#type fragment
#version 450

#define PI 3.14159265359

layout(location = 0) out vec4 FinalColor;

layout(binding = 0) uniform Matrices 
{
	mat4 View;
	mat4 Projection;
	vec4 ViewPos;
	
	float Turbidity;
	float Azimuth;
	float Inclination;
			
} u_Matrices;

layout(location = 1) in VertexOutput 
{
	vec3 TexCoord;
} vs_Input;

float saturatedDot( in vec3 a, in vec3 b )
{
	return max( dot( a, b ), 0.0 );   
}

vec3 YxyToXYZ( in vec3 Yxy )
{
	float Y = Yxy.r;
	float x = Yxy.g;
	float y = Yxy.b;

	float X = x * ( Y / y );
	float Z = ( 1.0 - x - y ) * ( Y / y );

	return vec3(X,Y,Z);
}

vec3 XYZToRGB( in vec3 XYZ )
{
	// CIE/E
	mat3 M = mat3
	(
		 2.3706743, -0.9000405, -0.4706338,
		-0.5138850,  1.4253036,  0.0885814,
		 0.0052982, -0.0146949,  1.0093968
	);

	return XYZ * M;
}

vec3 YxyToRGB( in vec3 Yxy )
{
	vec3 XYZ = YxyToXYZ( Yxy );
	vec3 RGB = XYZToRGB( XYZ );
	return RGB;
}

void calculatePerezDistribution( in float t, out vec3 A, out vec3 B, out vec3 C, out vec3 D, out vec3 E )
{
	A = vec3(  0.1787 * t - 1.4630, -0.0193 * t - 0.2592, -0.0167 * t - 0.2608 );
	B = vec3( -0.3554 * t + 0.4275, -0.0665 * t + 0.0008, -0.0950 * t + 0.0092 );
	C = vec3( -0.0227 * t + 5.3251, -0.0004 * t + 0.2125, -0.0079 * t + 0.2102 );
	D = vec3(  0.1206 * t - 2.5771, -0.0641 * t - 0.8989, -0.0441 * t - 1.6537 );
	E = vec3( -0.0670 * t + 0.3703, -0.0033 * t + 0.0452, -0.0109 * t + 0.0529 );
}

vec3 calculateZenithLuminanceYxy( in float t, in float thetaS )
{
	float chi  	 	= ( 4.0 / 9.0 - t / 120.0 ) * ( PI - 2.0 * thetaS );
	float Yz   	 	= ( 4.0453 * t - 4.9710 ) * tan( chi ) - 0.2155 * t + 2.4192;

	float theta2 	= thetaS * thetaS;
	float theta3 	= theta2 * thetaS;
	float T 	 	= t;
	float T2 	 	= t * t;

	float xz =
	  ( 0.00165 * theta3 - 0.00375 * theta2 + 0.00209 * thetaS + 0.0)     * T2 +
	  (-0.02903 * theta3 + 0.06377 * theta2 - 0.03202 * thetaS + 0.00394) * T +
	  ( 0.11693 * theta3 - 0.21196 * theta2 + 0.06052 * thetaS + 0.25886);

	float yz =
	  ( 0.00275 * theta3 - 0.00610 * theta2 + 0.00317 * thetaS + 0.0)     * T2 +
	  (-0.04214 * theta3 + 0.08970 * theta2 - 0.04153 * thetaS + 0.00516) * T +
	  ( 0.15346 * theta3 - 0.26756 * theta2 + 0.06670 * thetaS + 0.26688);

	return vec3( Yz, xz, yz );
}

vec3 calculatePerezLuminanceYxy( in float theta, in float gamma, in vec3 A, in vec3 B, in vec3 C, in vec3 D, in vec3 E )
{
	return ( 1.0 + A * exp( B / cos( theta ) ) ) * ( 1.0 + C * exp( D * gamma ) + E * cos( gamma ) * cos( gamma ) );
}

vec3 calculateSkyLuminanceRGB( in vec3 s, in vec3 e, in float t )
{
	vec3 A, B, C, D, E;
	calculatePerezDistribution( t, A, B, C, D, E );

	float thetaS = acos( saturatedDot( s, vec3(0,1,0) ) );
	float thetaE = acos( saturatedDot( e, vec3(0,1,0) ) );
	float gammaE = acos( saturatedDot( s, e )		   );

	vec3 Yz = calculateZenithLuminanceYxy( t, thetaS );

	vec3 fThetaGamma = calculatePerezLuminanceYxy( thetaE, gammaE, A, B, C, D, E );
	vec3 fZeroThetaS = calculatePerezLuminanceYxy( 0.0,    thetaS, A, B, C, D, E );

	vec3 Yp = Yz * ( fThetaGamma / fZeroThetaS );

	return YxyToRGB( Yp );
}

void main() 
{
	vec2 uv = vs_Input.TexCoord.xy / 2.0 + 0.5;
	
	float Inclination = u_Matrices.Inclination;
	float Azimuth = u_Matrices.Azimuth;
	float Turbidity = u_Matrices.Turbidity;
	
	vec3 sunDir = normalize( vec3( sin( Inclination ) * cos( Azimuth ), cos( Inclination ), sin( Inclination ) * sin( Azimuth ) ) );
	vec3 viewDir = normalize( vec3( ( uv * 2.0 ) - 1.0, 1.0 ) );
	vec3 skyLuminance = calculateSkyLuminanceRGB( sunDir, viewDir, Turbidity );
	
	FinalColor = vec4( skyLuminance * 0.05, 1.0 );
}
