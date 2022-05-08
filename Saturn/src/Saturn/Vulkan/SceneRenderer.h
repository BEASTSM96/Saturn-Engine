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

#include "Renderer.h"

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

	struct RenderSection 
	{
		VkCommandBuffer CommandBuffer;
		Saturn::Pipeline Pipeline;

		void Begin();

		void End();
	};

	struct RendererData
	{
		//////////////////////////////////////////////////////////////////////////
		// COMMAND POOLS & BUFFERS
		//////////////////////////////////////////////////////////////////////////
		
		VkCommandPool CommandPool;
		VkCommandBuffer CommandBuffer;
		
		//////////////////////////////////////////////////////////////////////////

		uint32_t FrameCount;

		//////////////////////////////////////////////////////////////////////////

		// Grid
		Pipeline GridPipeline;

		//////////////////////////////////////////////////////////////////////////

		struct GridMatricesObject
		{
			glm::mat4 ViewProjection;
			glm::mat4 Transform;

			float Scale;
			float Res;
		};

		//////////////////////////////////////////////////////////////////////////
	
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
			glm::mat4 Transform;
			glm::mat4 ViewProjection;

			bool UseAlbedoTexture;
			bool UseMetallicTexture;
			bool UseRoughnessTexture;
			bool UseNormalTexture;

			glm::vec4 AlbedoColor;
			glm::vec4 MetallicColor;
			glm::vec4 RoughnessColor;
		};

		//////////////////////////////////////////////////////////////////////////

		// GRID
		
		VkDescriptorSet GridDescriptorSet = nullptr;
		VkDescriptorSetLayout GridDescriptorSetLayout = nullptr;
		VkDescriptorPool GridDescriptorPool = nullptr;
		Buffer GridUniformBuffer;
		
		VertexBuffer* GridVertexBuffer;
		IndexBuffer* GridIndexBuffer;

		//////////////////////////////////////////////////////////////////////////

		// SKYBOX

		// Skybox
		Pipeline SkyboxPipeline;

		VkDescriptorSet SkyboxDescriptorSet = nullptr;
		VkDescriptorSetLayout SkyboxDescriptorSetLayout = nullptr;
		VkDescriptorPool SkyboxDescriptorPool = nullptr;
		Buffer SkyboxUniformBuffer;
		
		VertexBuffer* SkyboxVertexBuffer;
		IndexBuffer* SkyboxIndexBuffer;

		//////////////////////////////////////////////////////////////////////////

		// Geometry
		Pipeline GeometryPipeline;

		Pass GeometryPass;
		Resource GeometryPassDepth;
		Resource GeometryPassColor;
		
		VkCommandPool GeometryCmdPool;
		VkFramebuffer GeometryFramebuffer;


		//////////////////////////////////////////////////////////////////////////

		// SHADERS

		Ref< Shader > GridShader = nullptr;
		Ref< Shader > SkyboxShader = nullptr;
		Ref< Shader > StaticMeshShader = nullptr;
	};

	class SceneRenderer
	{
		SINGLETON( SceneRenderer );
	public:
		 SceneRenderer() { Init(); }
		 ~SceneRenderer() {}
		
		void SetRendererData( RendererData Data ) { m_RendererData = Data; }
		void SetCommandBuffer( VkCommandBuffer CommandBuffer ) { m_RendererData.CommandBuffer = CommandBuffer; }

		void SetCurrentScene( Scene* pScene ) { m_pSence = pScene; }

		void AddDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform );
		
		void RenderDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform );
		
		void FlushDrawList();

		void RenderScene();

		std::vector< DrawCommand >& GetDrawCmds() { return m_DrawList; }

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

		void CreateFullscreenQuad( VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer );
		void CreateFullscreenQuad( float x, float y, float w, float h,
			VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer );

		void GeometryPass();

	private:

		RendererData m_RendererData;
		std::vector< DrawCommand > m_DrawList;
		Scene* m_pSence;
	};
}