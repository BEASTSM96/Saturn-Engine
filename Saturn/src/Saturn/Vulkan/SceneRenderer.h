/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#pragma once

#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Entity.h"
#include "Mesh.h"
#include "Saturn/Core/UUID.h"

#include "Renderer.h"
#include "EnvironmentMap.h"
#include "DescriptorSet.h"
#include "Framebuffer.h"
#include "ComputePipeline.h"
#include "StorageBufferSet.h"

#include "Pipeline.h"

#define SHADOW_CASCADE_COUNT 4

namespace Saturn {

	struct DrawCommand
	{
		Entity entity;
		Ref< StaticMesh > Mesh = nullptr;
		glm::mat4 Transform;
		uint32_t SubmeshIndex;
	};

	struct ShadowCascade
	{
		Ref< Framebuffer > Framebuffer = nullptr;
		
		float SplitDepth = 0.0f;
		glm::mat4 ViewProjection;
	};

	// Most of theses structs MUST (most of the time) match the structs in the shader.
	// DirLight
	struct DirLight
	{
		glm::vec3 Direction {};
		float Padding = 0.0f;
		glm::vec3 Radiance {};
		float Multiplier = 0.0f;
	};

	// -1 = Prefilter, 0 = Downsample, 1 = Upsample
	enum class BloomStage
	{
		FirstUpsample = -2,
		Prefilter = -1,
		Downsample,
		Upsample
	};

	struct RendererCamera
	{
		Camera Camera;
		glm::mat4 ViewMatrix{};
	};

	struct StaticMeshKey
	{
	//	Submesh Submesh;
		uint32_t Count = 0;
	};

	// Data that gets sent to the vertex shader
	struct TransformBufferData
	{
		glm::vec4 TransfromBufferR[ 4 ];
	};

	// For each mesh, what offset are we and how much transform does it have.
	struct TransformBuffer
	{
		uint32_t Offset = 0;
		std::vector<TransformBufferData> Data;
	};

	struct SubmeshTransformVB
	{
		Ref<VertexBuffer> VertexBuffer;
		TransformBufferData* pData = nullptr;
	};

	struct RendererData
	{
		void Terminate();
		
		//////////////////////////////////////////////////////////////////////////
		// COMMAND POOLS & BUFFERS
		//////////////////////////////////////////////////////////////////////////
		
		VkCommandPool CommandPool = nullptr;
		VkCommandBuffer CommandBuffer = nullptr;
		
		//////////////////////////////////////////////////////////////////////////

		uint32_t FrameCount = 0;

		//////////////////////////////////////////////////////////////////////////

		RendererCamera CurrentCamera;

		//////////////////////////////////////////////////////////////////////////
		
		bool IsSwapchainTarget = false;

		//////////////////////////////////////////////////////////////////////////

		uint32_t Width = 0;
		uint32_t Height = 0;
		
		bool Resized = false;

		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// TIMERS
		//////////////////////////////////////////////////////////////////////////
	
		Timer GeometryPassTimer;
		Timer ShadowMapTimers[SHADOW_CASCADE_COUNT];
		Timer SSAOPassTimer;
		Timer AOCompositeTimer;
		Timer PreDepthTimer;
		Timer LightCullingTimer;
		Timer BloomTimer;

		//////////////////////////////////////////////////////////////////////////

		struct GridMatricesObject
		{
			glm::mat4 ViewProjection;
			
			glm::mat4 Transform;

			float Scale;
			float Res;
		};
		
		struct SkyboxMatricesObject
		{
			glm::mat4 InverseVP;
		};
		
		struct StaticMeshMatrices
		{
			glm::mat4 ViewProjection;
			glm::mat4 View;
		};

		struct StaticMeshMaterial
		{
			alignas( 4 ) float UseAlbedoTexture;
			alignas( 4 ) float UseMetallicTexture;
			alignas( 4 ) float UseRoughnessTexture;
			alignas( 4 ) float UseNormalTexture;

			alignas( 16 ) glm::vec4 AlbedoColor;
			alignas( 4 ) float Metalness;
			alignas( 4 ) float Roughness;
		};

		struct PointLights
		{
			uint32_t nbLights = 0;
			PointLight Lights[ 1024 ]{};
		};

		//////////////////////////////////////////////////////////////////////////
		Ref<StorageBufferSet> StorageBufferSet;

		// DirShadowMap
		//////////////////////////////////////////////////////////////////////////
		
		bool EnableShadows = true;

		std::vector< Ref< Pass > > DirShadowMapPasses;
		std::vector< Ref< Pipeline > > DirShadowMapPipelines;

		float CascadeSplitLambda = 0.92f;
		float CascadeFarPlaneOffset = 100.0f;
		float CascadeNearPlaneOffset = -150.0f;
		
		std::vector< ShadowCascade > ShadowCascades;

		// PreDepth + Light culling
		//////////////////////////////////////////////////////////////////////////

		Ref<Pass> PreDepthPass = nullptr;
		Ref<Pipeline> PreDepthPipeline = nullptr;
		Ref<Framebuffer> PreDepthFramebuffer = nullptr;
		//Ref< DescriptorSet > PreDepthDescriptorSet = nullptr;

		Ref< ComputePipeline > LightCullingPipeline = nullptr;
		Ref< DescriptorSet > LightCullingDescriptorSet = nullptr;
		glm::vec3 LightCullingWorkGroups{};

		// Geometry
		//////////////////////////////////////////////////////////////////////////

		// Render pass for all grid, skybox and meshes.
		Ref<Pass> GeometryPass = nullptr;
		Ref<Framebuffer> GeometryFramebuffer = nullptr;

		// STATIC MESHES

		// Main geometry for static meshes.
		Ref<Pipeline> StaticMeshPipeline = nullptr;
	
		// GRID

		Ref<Pipeline> GridPipeline = nullptr;

		Ref< DescriptorSet > GridDescriptorSet = nullptr;

		VertexBuffer* GridVertexBuffer = nullptr;
		IndexBuffer* GridIndexBuffer = nullptr;

		// SKYBOX

		Ref<EnvironmentMap> SceneEnvironment = nullptr;

		Ref<Pipeline> SkyboxPipeline = nullptr;

		Ref< DescriptorSet > SkyboxDescriptorSet = nullptr;
		Ref< DescriptorSet > PreethamDescriptorSet = nullptr;
				
		VertexBuffer* SkyboxVertexBuffer = nullptr;
		IndexBuffer* SkyboxIndexBuffer = nullptr;

		float SkyboxLod = 0.0f;

		//////////////////////////////////////////////////////////////////////////
		
		// End Geometry
		 
		//////////////////////////////////////////////////////////////////////////

		// Begin Scene Composite
		
		Ref<Pass> SceneComposite = nullptr;
		Ref< Framebuffer > SceneCompositeFramebuffer = nullptr;

		Ref<Pipeline> SceneCompositePipeline = nullptr;
		
		// Input
		Ref< DescriptorSet > SC_DescriptorSet = nullptr;

		VertexBuffer* SC_VertexBuffer = nullptr;
		IndexBuffer* SC_IndexBuffer = nullptr;
		
		// Texture pass
		//////////////////////////////////////////////////////////////////////////
		Ref<Pass> TexturePass = nullptr;
		Ref<Pipeline> TexturePassPipeline = nullptr;
		// Input
		Ref< DescriptorSet > TexturePassDescriptorSet = nullptr;

		//////////////////////////////////////////////////////////////////////////
		// End Scene Composite
		//////////////////////////////////////////////////////////////////////////

		// Bloom
		//////////////////////////////////////////////////////////////////////////

		Ref<ComputePipeline> BloomComputePipeline = nullptr;
		Ref<Texture2D> BloomTextures[ 3 ];
		Ref<Texture2D> BloomDirtTexture = nullptr;
		Ref< DescriptorSet > BloomDS = nullptr;

		uint32_t BloomWorkSize = 4;

		float BloomDirtIntensity = 20.0f;

		// BDRF Lut
		//////////////////////////////////////////////////////////////////////////
		Ref<Texture2D> BRDFLUT_Texture = nullptr;

		// Instanced Rendering
		//////////////////////////////////////////////////////////////////////////
		// MESH ID -> SUBMESH INDEX -> KEY
		std::unordered_map< UUID, std::unordered_map<uint32_t, StaticMeshKey> > InstancedMeshes;
		
		// MESH ID -> TRANSFORMS
		std::unordered_map< UUID, TransformBuffer > MeshTransforms;

		// This holds the entire transform data for each submesh, per frame in flight.
		std::vector< SubmeshTransformVB > SubmeshTransformData;

		//////////////////////////////////////////////////////////////////////////
		// SHADERS

		Ref< Shader > GridShader = nullptr;
		Ref< Shader > SkyboxShader = nullptr;
		Ref< Shader > PreethamShader = nullptr;
		Ref< Shader > StaticMeshShader = nullptr;
		Ref< Shader > SceneCompositeShader = nullptr;
		Ref< Shader > TexturePassShader = nullptr;
		Ref< Shader > DirShadowMapShader = nullptr;
		Ref< Shader > SelectedGeometryShader = nullptr;
		Ref< Shader > AOCompositeShader = nullptr;
		Ref< Shader > PreDepthShader = nullptr;
		Ref< Shader > LightCullingShader = nullptr;
		Ref< Shader > BloomShader = nullptr;
	};

	class SceneRenderer : public RefTarget
	{
		using ScheduledFunc = std::function<void()>;
	public:
		SceneRenderer() { Init(); }
		~SceneRenderer() {}

		void ImGuiRender();

		void SetCurrentScene( Scene* pScene ) { m_pScene = pScene; }

		void SubmitStaticMesh( Entity entity, Ref< StaticMesh > mesh, const glm::mat4 transform );
		void SubmitSelectedMesh( Entity entity, Ref< StaticMesh > mesh, const glm::mat4 transform );

		void SetViewportSize( uint32_t w, uint32_t h );

		void FlushDrawList();		

		void Recreate();

		void RenderScene();

		void SetCamera( const RendererCamera& Camera );

		std::vector< DrawCommand >& GetDrawCmds() { return m_DrawList; }

		Ref<Pass> GetGeometryPass() { return m_RendererData.GeometryPass; }
		const Ref<Pass> GetGeometryPass() const { return m_RendererData.GeometryPass; }

		Ref<Image2D> CompositeImage();

		void SetDynamicSky( float Turbidity, float Azimuth, float Inclination );
		
		void SetSwapchainTarget( bool target ) { m_RendererData.IsSwapchainTarget = target; }

		void Terminate();

		uint32_t Width() { return m_RendererData.Width; }
		uint32_t Height() { return m_RendererData.Height; }

	private:
		void Init();

		void RenderGrid();
		void RenderSkybox();
		void CheckInvalidSkybox();

		void UpdateCascades( const glm::vec3& Direction );

		void CreateGridComponents();

		void CreateSkyboxComponents();

		void InitGeometryPass();
		void InitDirShadowMap();
		void InitPreDepth();
		void InitSceneComposite();
		void InitBloom();
		void InitTexturePass();

		void DirShadowMapPass();
		void PreDepthPass();
		void LightCullingPass();
		void GeometryPass();
		void BloomPass();
		void SceneCompositePass();
		void TexturePass();

		void AddScheduledFunction( ScheduledFunc&& rrFunc );

		Ref<TextureCube> CreateDymanicSky();
	private:

		RendererData m_RendererData{};
		Scene* m_pScene = nullptr;

		std::vector< DrawCommand > m_DrawList;
		std::vector< DrawCommand > m_ShadowMapDrawList;
		std::vector< DrawCommand > m_SelectedMeshDrawList;

		std::vector< ScheduledFunc > m_ScheduledFunctions;

		ScheduledFunc m_LightCullingFunction;

	private:
		friend class Scene;
		friend class VulkanContext;
	};
}