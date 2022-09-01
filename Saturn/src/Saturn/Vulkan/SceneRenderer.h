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
#include "DescriptorSet.h"
#include "Framebuffer.h"

#include "Pipeline.h"

#define SHADOW_CASCADE_COUNT 4

namespace Saturn {

	struct DrawCommand
	{
		DrawCommand( Entity e, Ref< Mesh > mesh, glm::mat4 trans ) : entity( e ), Mesh( mesh ), Transform( trans )
		{
		}

		Entity entity;
		Ref< Mesh > Mesh = nullptr;
		glm::mat4 Transform;
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
		
		VkCommandPool CommandPool;
		VkCommandBuffer CommandBuffer;
		
		//////////////////////////////////////////////////////////////////////////

		uint32_t FrameCount;

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
			glm::mat4 View;
			glm::mat4 Projection;
			glm::vec4 ViewPos;
			
			float Turbidity;
			float Azimuth;
			float Inclination;
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

		// DirShadowMap
		//////////////////////////////////////////////////////////////////////////
		
		std::vector< Ref< Pass > > DirShadowMapPasses;
		std::vector< Ref< Pipeline > > DirShadowMapPipelines;

		float CascadeSplitLambda = 0.92f;
		float CascadeFarPlaneOffset = 100.0f;
		float CascadeNearPlaneOffset = -150.0f;
		
		bool ViewShadowCascades = false;

		std::vector< ShadowCascade > ShadowCascades;

		// Geometry
		//////////////////////////////////////////////////////////////////////////

		// Render pass for all grid, skybox and meshes.
		Ref<Pass> GeometryPass = nullptr;
		Ref<Framebuffer> GeometryFramebuffer = nullptr;

		// STATIC MESHES

		// Main geometry for static meshes.
		Ref<Pipeline> StaticMeshPipeline;
	
		// GRID

		Ref<Pipeline> GridPipeline;

		Ref< DescriptorSet > GridDescriptorSet = nullptr;

		VertexBuffer* GridVertexBuffer;
		IndexBuffer* GridIndexBuffer;

		// SKYBOX

		Ref<Pipeline> SkyboxPipeline;

		Ref< DescriptorSet > SkyboxDescriptorSet = nullptr;
				
		VertexBuffer* SkyboxVertexBuffer;
		IndexBuffer* SkyboxIndexBuffer;

		//////////////////////////////////////////////////////////////////////////
		
		// End Geometry
		 
		//////////////////////////////////////////////////////////////////////////

		// Begin Scene Composite
		
		Ref<Pass> SceneComposite = nullptr;
		Ref< Framebuffer > SceneCompositeFramebuffer = nullptr;

		Ref<Pipeline> SceneCompositePipeline;
		
		// Input
		Ref< DescriptorSet > SC_DescriptorSet = nullptr;

		VertexBuffer* SC_VertexBuffer;
		IndexBuffer* SC_IndexBuffer;
		
		//////////////////////////////////////////////////////////////////////////
		// End Scene Composite
		//////////////////////////////////////////////////////////////////////////

		// SHADERS

		Ref< Shader > GridShader = nullptr;
		Ref< Shader > SkyboxShader = nullptr;
		Ref< Shader > StaticMeshShader = nullptr;
		Ref< Shader > SceneCompositeShader = nullptr;
		Ref< Shader > DirShadowMapShader = nullptr;
		Ref< Shader > SelectedGeometryShader = nullptr;
	};

	class SceneRenderer : public CountedObj
	{
		SINGLETON( SceneRenderer );
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
		
		void Terminate();

	private:
		void Init();

		void RenderGrid();
		void RenderSkybox();

		void UpdateCascades( const glm::vec3& Direction );

		void CreateGridComponents();

		void CreateSkyboxComponents();

		void InitGeometryPass();
		void InitDirShadowMap();
		void InitSceneComposite();

		void GeometryPass();
		void DirShadowMapPass();
		void SceneCompositePass();

	private:

		RendererData m_RendererData;
		std::vector< DrawCommand > m_DrawList;
		std::vector< DrawCommand > m_SelectedMeshDrawList;
		Scene* m_pScene;

	private:
		friend class Scene;
		friend class VulkanContext;
	};
}