#type vertex
#version 450

// Inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 5) in vec4 a_TransformBufferR1;
layout(location = 6) in vec4 a_TransformBufferR2;
layout(location = 7) in vec4 a_TransformBufferR3;
layout(location = 8) in vec4 a_TransformBufferR4;

layout(set = 0, binding = 0) uniform Matrices
{
	mat4 ViewProjection;
} u_Matrices;

void main() 
{
	mat4 transform = mat4( 
		a_TransformBufferR1.x, a_TransformBufferR2.x, a_TransformBufferR3.x, 0.0, 
		a_TransformBufferR1.y, a_TransformBufferR2.y, a_TransformBufferR3.y, 0.0, 
		a_TransformBufferR1.z, a_TransformBufferR2.z, a_TransformBufferR3.z, 0.0, 
		a_TransformBufferR1.w, a_TransformBufferR2.w, a_TransformBufferR3.w, 1.0);

	gl_Position = u_Matrices.ViewProjection * transform * vec4( a_Position, 1.0 );
}
