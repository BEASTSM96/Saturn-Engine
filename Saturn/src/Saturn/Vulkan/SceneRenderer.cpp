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

#include "sppch.h"
#include "SceneRenderer.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

#include <glm/gtx/matrix_decompose.hpp>

#define M_PI 3.14159265358979323846

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::Init()
	{
		// Create grid.
		CreateGridComponents();

		CreateSkyboxComponents();
	}

	void SceneRenderer::Terminate()
	{
		// Destroy grid.
		DestroyGridComponents();

		DestroySkyboxComponents();

		m_pSence = nullptr;
		
		m_DrawList.clear();
		
		m_RendererData = {};
	}

	void SceneRenderer::RenderGrid()
	{
		// RENDER GRID
		{
			// BIND THE PIPELINE
			{
				vkCmdBindPipeline( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.GridPipeline.GetPipeline() );
			}

			// BIND THE DESCRIPTOR SET
			{
				vkCmdBindDescriptorSets( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.GridPipeline.GetPipelineLayout(), 0, 1, &m_RendererData.GridDescriptorSet, 0, nullptr );
			}

			// UPDATE UNIFORM BUFFERS
			{
				auto trans = glm::rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) * glm::scale( glm::mat4( 1.0f ), glm::vec3( 16.0f ) );

				RendererData::GridMatricesObject GridMatricesObject = {};
				GridMatricesObject.Transform = trans;
				GridMatricesObject.ViewProjection = VulkanContext::Get().GetEditorCamera().ViewProjection();

				GridMatricesObject.Res = 0.025f;
				GridMatricesObject.Scale = 16.025f;

				void* Data;

				m_RendererData.GridUniformBuffer.Map( &Data, sizeof( GridMatricesObject ) );

				memcpy( Data, &GridMatricesObject, sizeof( GridMatricesObject ) );
				
				m_RendererData.GridUniformBuffer.Unmap();
			}

			// DRAW
			{
				m_RendererData.GridVertexBuffer->Bind( m_RendererData.CommandBuffer );
				m_RendererData.GridIndexBuffer->Bind( m_RendererData.CommandBuffer );

				m_RendererData.GridIndexBuffer->Draw( m_RendererData.CommandBuffer );
			}
		}
	}

	void SceneRenderer::RenderSkybox()
	{
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		// BIND THE PIPELINE
		{
			vkCmdBindPipeline( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.SkyboxPipeline.GetPipeline() );
		}

		// BIND THE DESCRIPTOR SET
		{
			vkCmdBindDescriptorSets( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.SkyboxPipeline.GetPipelineLayout(), 0, 1, &m_RendererData.SkyboxDescriptorSet, 0, nullptr );
		}

		// UPDATE UNIFORM BUFFERS
		{
			RendererData::SkyboxMatricesObject SkyboxMatricesObject = {};

			SkyboxMatricesObject.View = VulkanContext::Get().GetEditorCamera().ViewMatrix();
			SkyboxMatricesObject.Projection = VulkanContext::Get().GetEditorCamera().ProjectionMatrix();
			SkyboxMatricesObject.Turbidity = 2.0;
			SkyboxMatricesObject.Azimuth = 0.210;
			SkyboxMatricesObject.Inclination = 2.050;

			void* Data;
			
			m_RendererData.SkyboxUniformBuffer.Map( &Data, sizeof( SkyboxMatricesObject ) );

			memcpy( Data, &SkyboxMatricesObject, sizeof( SkyboxMatricesObject ) );

			m_RendererData.SkyboxUniformBuffer.Unmap();
		}

		// DRAW
		{
			m_RendererData.SkyboxVertexBuffer->Bind( CommandBuffer );
			m_RendererData.SkyboxIndexBuffer->Bind( CommandBuffer );

			m_RendererData.SkyboxIndexBuffer->Draw( CommandBuffer );
		}
	}

	void SceneRenderer::CreateGridComponents()
	{
		VertexBufferLayout Layout =
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		// Create fullscreen quad.
		CreateFullscreenQuad( &m_RendererData.GridVertexBuffer, &m_RendererData.GridIndexBuffer );
		
		m_RendererData.GridShader = Ref< Shader >::Create( "Grid", "assets/shaders/Grid.glsl" );
	
		ShaderWorker::Get().AddShader( m_RendererData.GridShader.Pointer() );
		ShaderWorker::Get().CompileShader( m_RendererData.GridShader.Pointer() );
		
		// Create uniform buffer.
		VkDeviceSize BufferSize = sizeof( RendererData::GridMatricesObject );
		m_RendererData.GridUniformBuffer = Buffer();

		m_RendererData.GridUniformBuffer.Create( nullptr, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		
		// Create descriptor set layout.
		// TODO: Move to vulkan context.
		VkDescriptorSetLayoutBinding UBOLayoutBinding = {};
		UBOLayoutBinding.binding = 0;
		UBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		UBOLayoutBinding.descriptorCount = 1;
		UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		
		VkDescriptorSetLayoutCreateInfo LayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		LayoutCreateInfo.bindingCount = 1;
		LayoutCreateInfo.pBindings = &UBOLayoutBinding;
		
		VK_CHECK( vkCreateDescriptorSetLayout( VulkanContext::Get().GetDevice(), &LayoutCreateInfo, nullptr, &m_RendererData.GridDescriptorSetLayout ) );

		// Create descriptor pool.
		// TODO: Move to vulkan context as we may only need one pool.
		VkDescriptorPoolSize PoolSize = {};
		PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSize.descriptorCount = 1;
		
		VkDescriptorPoolCreateInfo PoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		PoolCreateInfo.poolSizeCount = 1;
		PoolCreateInfo.pPoolSizes = &PoolSize;
		PoolCreateInfo.maxSets = 1;
		
		VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_RendererData.GridDescriptorPool ) );

		// Create descriptor set.
		VkDescriptorSetAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		AllocateInfo.descriptorPool = m_RendererData.GridDescriptorPool;
		AllocateInfo.descriptorSetCount = 1;
		AllocateInfo.pSetLayouts = &m_RendererData.GridDescriptorSetLayout;
		
		VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &AllocateInfo, &m_RendererData.GridDescriptorSet ) );
		
		// Create descriptor write.
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = m_RendererData.GridUniformBuffer;
		BufferInfo.offset = 0;
		BufferInfo.range = BufferSize;
		
		VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		Write.dstSet = m_RendererData.GridDescriptorSet;
		Write.dstBinding = 0;
		Write.dstArrayElement = 0;
		Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Write.descriptorCount = 1;
		Write.pBufferInfo = &BufferInfo;
		
		vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &Write, 0, nullptr );

		// Gird shader attribute descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;
		
		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( GridVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( GridVertex, Texcoord ) } );

		// Grid shader binding descriptions.
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );

		// Grid pipeline spec.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = Window::Get().Width();
		PipelineSpec.Height = Window::Get().Height();
		PipelineSpec.Name = "Grid";
		PipelineSpec.pShader = m_RendererData.GridShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.GridDescriptorSetLayout } };
		PipelineSpec.RenderPass = VulkanContext::Get().GetOffscreenRenderPass();
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		PipelineSpec.UseDepthTest = true;
		
		m_RendererData.GridPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::DestroyGridComponents()
	{
		// Destroy grid pipeline.
		m_RendererData.GridPipeline.Terminate();

		// Destroy uniform buffer.
		m_RendererData.GridUniformBuffer.Terminate();

		// Destroy grid index buffer.
		m_RendererData.GridIndexBuffer->Terminate();

		// Destroy grid vertex buffer.
		m_RendererData.GridVertexBuffer->Terminate();

		// Destroy descriptor set layout.
		vkDestroyDescriptorSetLayout( VulkanContext::Get().GetDevice(), m_RendererData.GridDescriptorSetLayout, nullptr );

		// Destroy descriptor pool.
		vkDestroyDescriptorPool( VulkanContext::Get().GetDevice(), m_RendererData.GridDescriptorPool, nullptr );
	}

	//////////////////////////////////////////////////////////////////////////
	// Skybox
	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::CreateSkyboxComponents()
	{		
		VertexBufferLayout Layout = 
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};

		// Create fullscreen quad.
		CreateFullscreenQuad( &m_RendererData.SkyboxVertexBuffer, &m_RendererData.SkyboxIndexBuffer );

		// Create skybox shader.
		m_RendererData.SkyboxShader = Ref<Shader>::Create( "Skybox", "assets/shaders/Skybox.glsl" );

		ShaderWorker::Get().AddShader( m_RendererData.SkyboxShader.Pointer() );
		ShaderWorker::Get().CompileShader( m_RendererData.SkyboxShader.Pointer() );

		// Create uniform buffer.
		VkDeviceSize BufferSize = sizeof( RendererData::SkyboxMatricesObject );

		m_RendererData.SkyboxUniformBuffer = Buffer();

		m_RendererData.SkyboxUniformBuffer.Create( nullptr, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		
		// Create descriptor set layout.
		VkDescriptorSetLayoutBinding BindingLayout = {};
		BindingLayout.binding = 0;
		BindingLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		BindingLayout.descriptorCount = 1;
		BindingLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector< VkDescriptorSetLayoutBinding > Bindings = { BindingLayout };
		
		VkDescriptorSetLayoutCreateInfo LayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		LayoutInfo.bindingCount = Bindings.size();
		LayoutInfo.pBindings = Bindings.data();
		
		VK_CHECK( vkCreateDescriptorSetLayout( VulkanContext::Get().GetDevice(), &LayoutInfo, nullptr, &m_RendererData.SkyboxDescriptorSetLayout ) );

		// Create descriptor pool.
		VkDescriptorPoolSize PoolSize = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 };
		PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo PoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		PoolCreateInfo.poolSizeCount = 1;
		PoolCreateInfo.pPoolSizes = &PoolSize;
		PoolCreateInfo.maxSets = 1;
		
		VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_RendererData.SkyboxDescriptorPool ) );

		// Create descriptor set.
		VkDescriptorSetAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		AllocateInfo.descriptorPool = m_RendererData.SkyboxDescriptorPool;
		AllocateInfo.descriptorSetCount = 1;
		AllocateInfo.pSetLayouts = &m_RendererData.SkyboxDescriptorSetLayout;

		VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &AllocateInfo, &m_RendererData.SkyboxDescriptorSet ) );

		// Create descriptor write.
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = m_RendererData.SkyboxUniformBuffer;
		BufferInfo.offset = 0;
		BufferInfo.range = BufferSize;
		
		VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		Write.dstSet = m_RendererData.SkyboxDescriptorSet;
		Write.dstBinding = 0;
		Write.dstArrayElement = 0;
		Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Write.descriptorCount = 1;
		Write.pBufferInfo = &BufferInfo;
		
		vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &Write, 0, nullptr );
		
		// Gird shader attribute descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;

		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( BaseVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( BaseVertex, Texcoord ) } );
		
		// Grid shader binding descriptions.
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );
		
		// Create pipeline.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = Window::Get().Width();
		PipelineSpec.Height = Window::Get().Height();
		PipelineSpec.Name = "Skybox";
		PipelineSpec.pShader = m_RendererData.SkyboxShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.SkyboxDescriptorSetLayout } };
		PipelineSpec.RenderPass = VulkanContext::Get().GetOffscreenRenderPass();
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		
		m_RendererData.SkyboxPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::DestroySkyboxComponents()
	{
		// Destroy Skybox pipeline.
		m_RendererData.SkyboxPipeline.Terminate();

		// Destroy uniform buffer.
		m_RendererData.SkyboxUniformBuffer.Terminate();

		// Destroy Skybox index buffer.
		m_RendererData.SkyboxIndexBuffer->Terminate();

		// Destroy Skybox vertex buffer.
		m_RendererData.SkyboxVertexBuffer->Terminate();

		// Destroy descriptor set layout.
		vkDestroyDescriptorSetLayout( VulkanContext::Get().GetDevice(), m_RendererData.SkyboxDescriptorSetLayout, nullptr );

		// Destroy descriptor pool.
		vkDestroyDescriptorPool( VulkanContext::Get().GetDevice(), m_RendererData.SkyboxDescriptorPool, nullptr );
	}
	
	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::CreateFullscreenQuad( VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer )
	{
		CreateFullscreenQuad( -1, -1, 2, 2, ppVertexBuffer, ppIndexBuffer );
	}

	void SceneRenderer::CreateFullscreenQuad( float x, float y, float w, float h, 
		VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer )
	{
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[ 4 ];

		data[ 0 ].Position = glm::vec3( x, y, 0.1f );
		data[ 0 ].TexCoord = glm::vec2( 0, 0 );

		data[ 1 ].Position = glm::vec3( x + w, y, 0.1f );
		data[ 1 ].TexCoord = glm::vec2( 1, 0 );

		data[ 2 ].Position = glm::vec3( x + w, y + h, 0.1f );
		data[ 2 ].TexCoord = glm::vec2( 1, 1 );

		data[ 3 ].Position = glm::vec3( x, y + h, 0.1f );
		data[ 3 ].TexCoord = glm::vec2( 0, 1 );

		*ppVertexBuffer = new VertexBuffer( data, 4 * sizeof( QuadVertex ) );

		uint32_t indices[ 6 ] = { 0, 1, 2,
								  2, 3, 0, };

		*ppIndexBuffer = new IndexBuffer( indices, 6 );
	}

	void SceneRenderer::AddDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		m_DrawList.push_back( { entity, mesh, transform } );
	}

	void SceneRenderer::RenderDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		auto& uuid = entity.GetComponent<IdComponent>().ID;

		vkCmdBindPipeline( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanContext::Get().GetPipeline().GetPipeline() );
		
		// Render a draw command.

		// Bind vertex and index buffers.
		mesh->GetVertexBuffer()->Bind( m_RendererData.CommandBuffer );
		mesh->GetIndexBuffer()->Bind( m_RendererData.CommandBuffer );

		mesh->GetMaterial()->Bind( nullptr );

		// Bind the descriptor sets.
		vkCmdBindDescriptorSets( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanContext::Get().GetPipeline().GetPipelineLayout(), 0, 1, &VulkanContext::Get().GetDescriptorSets()[ uuid ], 0, nullptr );

		// No push constants for now.

		// Update uniform buffers.
		VulkanContext::Get().UpdateUniformBuffers( entity.GetComponent<IdComponent>().ID, Application::Get().Time(), entity.GetComponent<TransformComponent>().GetTransform() );

		// Draw.
		mesh->GetIndexBuffer()->Draw( m_RendererData.CommandBuffer );
	}

	void SceneRenderer::FlushDrawList()
	{
		m_DrawList.clear();
	}

	void SceneRenderer::RenderScene()
	{
		if( m_DrawList.size() )
		{
			VulkanContext::Get().CreateDescriptorPool();
			//VulkanContext::Get().CreateDescriptorSets();
		}
		
		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Skybox" );

		RenderSkybox();
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );
		
		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Grid" );

		RenderGrid();
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );
		
		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Static meshes" );

		// Render static meshes.
		for ( auto& Cmd : m_DrawList )
		{
			RenderDrawCommand( Cmd.entity, Cmd.Mesh, Cmd.Transform );	
		}

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		FlushDrawList();
	}
}