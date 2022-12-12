/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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
		Ref< Mesh > Mesh = nullptr;
		glm::mat4 Transform;
		uint32_t SubmeshIndex;
	};

	struct ShadowCascade
	{
		Ref< Framebuffer > Framebuffer = nullptr;
		
		float SplitDepth = 0.0f;
		glm::mat4 ViewProjection;
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

		Saturn::EditorCamera EditorCamera;
		Saturn::Camera RuntimeCamera;

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
		
		bool ViewShadowCascades = false;

		std::vector< ShadowCascade > ShadowCascades;

		// PreDepth + Light culling
		//////////////////////////////////////////////////////////////////////////

		Ref<Pass> PreDepthPass = nullptr;
		Ref<Pipeline> PreDepthPipeline = nullptr;
		Ref<Framebuffer> PreDepthFramebuffer = nullptr;
		//Ref< DescriptorSet > PreDepthDescriptorSet = nullptr;

		Ref< ComputePipeline > LightCullingPipeline;
		Ref< DescriptorSet > LightCullingDescriptorSet = nullptr;
		glm::vec3 LightCullingWorkGroups;

		bool ShowLightCulling = false;
		bool ShowLightComplexity = false;

		// Geometry
		//////////////////////////////////////////////////////////////////////////

		// Render pass for all grid, skybox and meshes.
		Ref<Pass> GeometryPass = nullptr;
		Ref<Framebuffer> GeometryFramebuffer = nullptr;

		// STATIC MESHES

		// Main geometry for static meshes.
		Ref<Pipeline> StaticMeshPipeline = nullptr;
	
		// GRID

		Ref<Pipeline> GridPipeline;

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

		float SkyboxLod;

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
		
		//////////////////////////////////////////////////////////////////////////
		// End Scene Composite
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// AO
		//////////////////////////////////////////////////////////////////////////

		// AO-Composite

		Ref< DescriptorSet > AO_DescriptorSet = nullptr;
		Ref<Pipeline> AOCompositePipeline;
		Ref<Pass> AOComposite;
		Ref<Framebuffer> AOCompositeFramebuffer;

		// Screen-Space AO

		int SSAO_KERNEL_SIZE = 32;
		float SSAO_RADIUS    = 0.3F;
		float SSAO_NOISE_DIM = 4.0F;

		std::vector<glm::vec4> SSAOKernel;

		Ref<Image2D> SSAONoiseImage = nullptr;

		Ref<Pipeline> SSAOPipeline;
		Ref<Pipeline> SSAOBlurPipeline;

		Ref<Pass> SSAORenderPass;
		Ref<Pass> SSAOBlurRenderPass;

		Ref<Framebuffer> SSAOFramebuffer;
		Ref<Framebuffer> SSAOBlurFramebuffer;

		Ref< DescriptorSet > SSAO_DescriptorSet = nullptr;
		VertexBuffer* SSAO_VertexBuffer = nullptr;
		IndexBuffer* SSAO_IndexBuffer = nullptr;

		Ref< Image2D > SSAO_NoiseImage;

		//////////////////////////////////////////////////////////////////////////
		// BDRF Lut
		Ref<Texture2D> BRDFLUT_Texture;

		//////////////////////////////////////////////////////////////////////////
		// SHADERS

		Ref< Shader > GridShader = nullptr;
		Ref< Shader > SkyboxShader = nullptr;
		Ref< Shader > PreethamShader = nullptr;
		Ref< Shader > StaticMeshShader = nullptr;
		Ref< Shader > SceneCompositeShader = nullptr;
		Ref< Shader > DirShadowMapShader = nullptr;
		Ref< Shader > SelectedGeometryShader = nullptr;
		Ref< Shader > SSAOShader = nullptr;
		Ref< Shader > SSAOBlurShader = nullptr;
		Ref< Shader > AOCompositeShader = nullptr;
		Ref< Shader > PreDepthShader = nullptr;
		Ref< Shader > LightCullingShader = nullptr;
	};

	class SceneRenderer : public CountedObj
	{
		SINGLETON( SceneRenderer );

		using ScheduledFunc = std::function<void()>;

	public:
		SceneRenderer() { Init(); }
		~SceneRenderer() {}

		void SetRendererData( RendererData Data ) { m_RendererData = Data; }
		void SetCommandBuffer( VkCommandBuffer CommandBuffer ) { m_RendererData.CommandBuffer = CommandBuffer; }

		void ImGuiRender();

		void SetCurrentScene( Scene* pScene ) { m_pScene = pScene; }

		void SubmitSelectedMesh( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform );
		void SubmitMesh( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform );

		void SetViewportSize( uint32_t w, uint32_t h );

		void FlushDrawList();		

		void Recreate();

		void RenderScene();

		void SetEditorCamera( const EditorCamera& Camera );

		std::vector< DrawCommand >& GetDrawCmds() { return m_DrawList; }

		Ref<Pass> GetGeometryPass() { return m_RendererData.GeometryPass; }
		const Ref<Pass> GetGeometryPass() const { return m_RendererData.GeometryPass; }

		Ref<Image2D> CompositeImage();

		void SetDynamicSky( float Turbidity, float Azimuth, float Inclination );
		
		void Terminate();

	private:
		void Init();

		void RenderGrid();
		void RenderSkybox();
		void PrepareSkybox();

		void UpdateCascades( const glm::vec3& Direction );

		void CreateGridComponents();

		void CreateSkyboxComponents();

		void InitGeometryPass();
		void InitDirShadowMap();
		void InitPreDepth();
		void InitSceneComposite();
		void InitAO();
		void InitAOComposite();

		void DirShadowMapPass();
		void PreDepthPass();
		void GeometryPass();
		void SceneCompositePass();
		void AOPass();
		void AOCompositePass();
		void LightCullingPass();

		void AddScheduledFunction( ScheduledFunc&& rrFunc );

		Ref<TextureCube> CreateDymanicSky();
	private:

		RendererData m_RendererData;
		Scene* m_pScene;

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