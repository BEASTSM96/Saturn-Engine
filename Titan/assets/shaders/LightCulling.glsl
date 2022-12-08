// FROM: https://github.com/bcrusco/Forward-Plus-Renderer/blob/master/Forward-Plus/Forward-Plus/source/shaders/light_culling.comp.glsl
// ALSO: https://github.com/WindyDarian/Vulkan-Forward-Plus-Renderer/blob/master/src/shaders/light_culling.comp.glsl

#type compute
#version 450 core

struct PointLight
{
	vec3 Position;
	vec3 Radiance;

	float Multiplier;
	float LightSize;
	float Radius;
	float MinRadius;
	float Falloff;
};

layout(std140, set = 0, binding = 0) uniform PointLightData
{
	uint nbLights;
	PointLight Lights[1024];
} u_Lights;

layout(std430, set = 0, binding = 2) buffer VisiblePointLightIndicesBuffer
{
	int Indices[];
} s_VisiblePointLightIndicesBuffer;

layout(std140, binding = 3) uniform ScreenData
{
	vec2 FullResolution;
} u_ScreenInfo;

layout(set = 0, binding = 4) uniform Matrices 
{
	mat4 ViewProjection;
	mat4 View;
} u_Matrices;

layout(set = 0, binding = 5) uniform Camera 
{
	vec2 DepthUnpack;
} u_Camera;

layout(set = 0, binding = 6) uniform sampler2D u_PreDepth;

#define MAX_LIGHT_COUNT 1024
#define TILE_SIZE 16

// Shared values between all the threads in the group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];
shared int visibleLightIndices[MAX_LIGHT_COUNT];

float ScreenSpaceToViewSpaceDepth(const float screenDepth)
{
	float depthLinearizeMul = u_Camera.DepthUnpack.x;
	float depthLinearizeAdd = u_Camera.DepthUnpack.y;
	// Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"
	return depthLinearizeMul / (depthLinearizeAdd - screenDepth);
}

layout( local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1 ) in;

void main() 
{
	ivec2 location = ivec2(gl_GlobalInvocationID.xy);
	ivec2 itemID = ivec2(gl_LocalInvocationID.xy);
	ivec2 tileID = ivec2(gl_WorkGroupID.xy);
	ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);

	uint index = tileID.y * tileNumber.x + tileID.x;

	if (gl_LocalInvocationIndex == 0) 
	{
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visibleLightCount = 0;
	}

	barrier();

	// Step 1: Calculate the minimum and maximum depth values (from the depth buffer) for this group's tile
	vec2 tc = vec2( location ) / u_ScreenInfo.FullResolution;
	float linearDepth = ScreenSpaceToViewSpaceDepth(textureLod(u_PreDepth, tc, 0).r);

	// Convert depth to uint so we can do atomic min and max comparisons between the threads
	uint depthInt = floatBitsToUint(linearDepth);
	atomicMin(minDepthInt, depthInt);
	atomicMax(maxDepthInt, depthInt);

	barrier();

	// Step 2: One thread should calculate the frustum planes to be used for this tile
	if (gl_LocalInvocationIndex == 0) 
	{
		// Convert the min and max across the entire tile back to float
		float minDepth = uintBitsToFloat(minDepthInt);
		float maxDepth = uintBitsToFloat(maxDepthInt);

		// Steps based on tile sale
		vec2 negativeStep = (2.0 * vec2(tileID)) / vec2(tileNumber);
		vec2 positiveStep = (2.0 * vec2(tileID + ivec2(1, 1))) / vec2(tileNumber);

		// Set up starting values for planes using steps and min and max z values
		frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
		frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
		frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minDepth); // Near
		frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth); // Far

		// Transform the first four planes
		for (uint i = 0; i < 4; i++) {
			frustumPlanes[i] *= u_Matrices.ViewProjection;
			frustumPlanes[i] /= length(frustumPlanes[i].xyz);
		}

		// Transform the depth planes
		frustumPlanes[4] *= u_Matrices.View;
		frustumPlanes[4] /= length(frustumPlanes[4].xyz);
		frustumPlanes[5] *= u_Matrices.View;
		frustumPlanes[5] /= length(frustumPlanes[5].xyz);
	}

	barrier();

	// Step 3: Cull lights.
	// Parallelize the threads against the lights now.
	// Can handle 256 simultaniously. Anymore lights than that and additional passes are performed

	uint threadCount = TILE_SIZE * TILE_SIZE;
	uint passCount = (u_Lights.nbLights + threadCount - 1) / threadCount;

	for (uint i = 0; i < passCount; i++)
	{
		// Get the lightIndex to test for this thread / pass. If the index is >= light count, then this thread can stop testing lights
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex;
		if (lightIndex >= u_Lights.nbLights) 
		{
			break;
		}

		vec4 position = vec4( u_Lights.Lights[lightIndex].Position, 1.0 );
		float radius = u_Lights.Lights[lightIndex].Radius;
		radius += radius * 3.0f;

		// Check if light radius is in frustum
		float distance = 0.0;
		for (uint j = 0; j < 6; j++)
		{
			distance = dot(position, frustumPlanes[j]) + radius;
			if (distance <= 0.0) // No intersection
				break;
		}

		// If greater than zero, then it is a visible light
		if (distance > 0.0)
		{
			// Add index to the shared array of visible indices
			uint offset = atomicAdd(visibleLightCount, 1);
			visibleLightIndices[offset] = int(lightIndex);
		}
	}

	barrier();

	// One thread should fill the global light buffer
	if (gl_LocalInvocationIndex == 0)
	{
		uint offset = index * MAX_LIGHT_COUNT; // Determine bosition in global buffer
		for (uint i = 0; i < visibleLightCount; i++) {
			s_VisiblePointLightIndicesBuffer.Indices[offset + i] = visibleLightIndices[i];
		}

		if (visibleLightCount != MAX_LIGHT_COUNT)
		{
			// Unless we have totally filled the entire array, mark it's end with -1
			// Final shader step will use this to determine where to stop (without having to pass the light count)
			s_VisiblePointLightIndicesBuffer.Indices[offset + visibleLightCount] = -1;
		}
	}
}