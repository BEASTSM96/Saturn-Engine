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

#include "Pipeline.h"

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
	
	struct MeshDescriptorSet
	{
		bool operator ==( const MeshDescriptorSet& rOther )
		{
			return ( rOther.Owner == Owner );
		}

		void Terminate() 
		{
			for( Submesh& rSubmesh : Mesh->Submeshes() )
			{
				DescriptorSets[ rSubmesh ]->Terminate();
			}

			DescriptorSets.clear();
		}
		
		void BindAll( VkCommandBuffer CommandBuffer, VkPipelineLayout PipelineLayout )
		{
			for( Submesh& rSubmesh : Mesh->Submeshes() )
			{
				DescriptorSets[ rSubmesh ]->Bind( CommandBuffer, PipelineLayout );
			}
		}

		void Bind( Submesh& rSubmesh, VkCommandBuffer CommandBuffer, VkPipelineLayout PipelineLayout )
		{
			DescriptorSets[ rSubmesh ]->Bind( CommandBuffer, PipelineLayout );
		}

		UUID Owner;
		Ref< Saturn::Mesh > Mesh;
		std::unordered_map< Submesh, Ref< DescriptorSet > > DescriptorSets;
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
		
		uint32_t Width = 3440;
		uint32_t Height = 1440;
		
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
		};

		struct StaticMeshMaterial
		{
			alignas( 16 ) glm::mat4 Transform;

			alignas( 4 ) float UseAlbedoTexture;
			alignas( 4 ) float UseMetallicTexture;
			alignas( 4 ) float UseRoughnessTexture;
			alignas( 4 ) float UseNormalTexture;

			alignas( 16 ) glm::vec4 AlbedoColor;
			alignas( 16 ) glm::vec4 MetallicColor;
			alignas( 16 ) glm::vec4 RoughnessColor;
		};

		// Geometry
		//////////////////////////////////////////////////////////////////////////

		// Render pass for all grid, skybox and meshes.
		Pass GeometryPass;
		Resource GeometryPassDepth;
		Resource GeometryPassColor;
		VkFramebuffer GeometryFramebuffer;
		
		Ref< DescriptorPool > GeometryDescriptorPool;

		// Buffer image.
		VkDescriptorSet RenderPassResult;
		
		// STATIC MESHES

		// Static mesh geometry.
		VkBuffer SM_MatricesUBO;
		// Main geometry for static meshes.
		Pipeline StaticMeshPipeline;
		
		DescriptorSetLayout SM_DescriptorSetLayout;

		std::unordered_map< UUID, MeshDescriptorSet > StaticMeshDescriptorSets;
		
		// Dynamic mesh geometry.
		// For animated meshes.
		Pipeline DynamicMeshPipeline;

		// GRID

		Pipeline GridPipeline;

		Ref< DescriptorSet > GridDescriptorSet;
		Ref< DescriptorPool > GridDescriptorPool;

		VkBuffer GridUniformBuffer;
		VmaAllocation GridUBOAllocation;

		VertexBuffer* GridVertexBuffer;
		IndexBuffer* GridIndexBuffer;

		// SKYBOX

		Pipeline SkyboxPipeline;

		Ref< DescriptorSet > SkyboxDescriptorSet;
		Ref< DescriptorPool > SkyboxDescriptorPool;
		
		VkBuffer SkyboxUniformBuffer;
		
		VertexBuffer* SkyboxVertexBuffer;
		IndexBuffer* SkyboxIndexBuffer;

		//////////////////////////////////////////////////////////////////////////
		
		// End Geometry
		 
		//////////////////////////////////////////////////////////////////////////

		// Begin Scene Composite
		
		Pass SceneComposite;
		Resource SceneCompositeColor;
		Resource SceneCompositeDepth;
		VkFramebuffer SceneCompositeFramebuffer;

		Pipeline SceneCompositePipeline;
		
		Ref< DescriptorSet > SC_DescriptorSet;
		Ref< DescriptorPool > SC_DescriptorPool;
		DescriptorSetLayout SC_DescriptorSetLayout;

		VertexBuffer* SC_VertexBuffer;
		IndexBuffer* SC_IndexBuffer;
		
		VkDescriptorSet SceneCompositeResult;

		//////////////////////////////////////////////////////////////////////////
		// End Scene Composite
		//////////////////////////////////////////////////////////////////////////
		
		// SHADERS

		Ref< Shader > GridShader = nullptr;
		Ref< Shader > SkyboxShader = nullptr;
		Ref< Shader > StaticMeshShader = nullptr;
		Ref< Shader > SceneCompositeShader = nullptr;
	};

	class SceneRenderer
	{
		SINGLETON( SceneRenderer );
	public:
		SceneRenderer() { Init(); }
		~SceneRenderer() {}

		void SetRendererData( RendererData Data ) { m_RendererData = Data; }
		void SetCommandBuffer( VkCommandBuffer CommandBuffer ) { m_RendererData.CommandBuffer = CommandBuffer; }

		void ImGuiRender();

		void SetCurrentScene( Scene* pScene ) { m_pSence = pScene; }

		void AddDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform );

		void RenderDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform );
		
		void SetWidthAndHeight( uint32_t w, uint32_t h ) { m_RendererData.Width = w; m_RendererData.Height = h; Recreate(); }

		void FlushDrawList();		

		void Recreate();

		void RenderScene();

		void SetEditorCamera( const EditorCamera& Camera );

		// TODO: For every static mesh we need a descriptor set.
		//		 Only adds a descriptor set for a static mesh if it doesn't exist.
		void AddDescriptorSet( const DescriptorSet& rDescriptorSet );

		MeshDescriptorSet CreateSMDescriptorSet( UUID& rUUID, const glm::mat4 Transform, const Ref< Mesh >& rMesh );
		void DestroySMDescriptorSet( UUID uuid );

		std::vector< DrawCommand >& GetDrawCmds() { return m_DrawList; }

		Pass& GetGeometryPass() { return m_RendererData.GeometryPass; }
		const Pass& GetGeometryPass() const { return m_RendererData.GeometryPass; }

		VkDescriptorSet& GetGeometryResult() { return m_RendererData.RenderPassResult; }
		VkDescriptorSet& CompositeImage() { return m_RendererData.SceneCompositeResult; }

		void CreateGeometryResult();

		void Terminate();

	private:
		void Init();

		void RenderGrid();
		void RenderSkybox();

		void CreateGridComponents();
		void DestroyGridComponents();

		void CreateSkyboxComponents();
		void DestroySkyboxComponents();

		void InitGeometryPass();
		void InitSceneComposite();

		void CreateFullscreenQuad( VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer );
		void CreateFullscreenQuad( float x, float y, float w, float h,
			VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer );

		void GeometryPass();
		void SceneCompositePass();

	private:

		RendererData m_RendererData;
		std::vector< DrawCommand > m_DrawList;
		Scene* m_pSence;

	private:
		friend class Scene;
		friend class VulkanContext;
	};

}

namespace std {
	
	template<>
	struct hash< Saturn::MeshDescriptorSet >
	{
		size_t operator()( const Saturn::MeshDescriptorSet& rOther ) const
		{
			return rOther.Owner;
		}
	};
}