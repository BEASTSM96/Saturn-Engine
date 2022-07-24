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
#include "Renderer.h"

#include "VulkanDebug.h"
#include "DescriptorSet.h"
#include "MaterialInstance.h"
#include "Shader.h"

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////

	Renderer::Renderer()
	{
		Init();
	}

	Renderer::~Renderer()
	{
		//Terminate();
	}

	void Renderer::Init()
	{
		// Create Sync objects.
		VkSemaphoreCreateInfo SemaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo     FenceCreateInfo     = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		FenceCreateInfo.flags                     = VK_FENCE_CREATE_SIGNALED_BIT;

		m_FlightFences.resize( MAX_FRAMES_IN_FLIGHT );

		for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			VK_CHECK( vkCreateFence( VulkanContext::Get().GetDevice(), &FenceCreateInfo, nullptr, &m_FlightFences[ i ] ) );
		}

		VK_CHECK( vkCreateSemaphore( VulkanContext::Get().GetDevice(), &SemaphoreCreateInfo, nullptr, &m_AcquireSemaphore ) );
		VK_CHECK( vkCreateSemaphore( VulkanContext::Get().GetDevice(), &SemaphoreCreateInfo, nullptr, &m_SubmitSemaphore ) );

		SetDebugUtilsObjectName( "Acquire Semaphore", ( uint64_t ) m_AcquireSemaphore, VK_OBJECT_TYPE_SEMAPHORE );
		SetDebugUtilsObjectName( "Submit Semaphore", ( uint64_t ) m_SubmitSemaphore, VK_OBJECT_TYPE_SEMAPHORE );

		uint32_t* pData = new uint32_t[ 64 * 64 ];

		for( uint32_t i = 0; i < 64 * 64; i++ )
		{
			pData[ i ] |= 0xffffffff;
		}
		
		m_PinkTexture = Ref< Texture2D >::Create( 64, 64, VK_FORMAT_R8G8B8A8_SRGB, pData );
		m_PinkTexture->SetIsRendererTexture( true );

		delete[] pData;

		if( Application::Get().GetSpecification().CreateSceneRenderer )
		{
			Ref<Shader> shader = ShaderLibrary::Get().Find( "shader_new" );
			// Set 1 is for enviroment data.
			m_RendererDescriptorSet = shader->CreateDescriptorSet( 1 );
		}
	}
	
	void Renderer::Terminate()
	{
		// Terminate Semaphores.
		SubmitTerminateResource( [&]()
		{
			if( m_AcquireSemaphore )
				vkDestroySemaphore( VulkanContext::Get().GetDevice(), m_AcquireSemaphore, nullptr );
			
			if( m_SubmitSemaphore )
				vkDestroySemaphore( VulkanContext::Get().GetDevice(), m_SubmitSemaphore, nullptr );

			m_AcquireSemaphore = nullptr;
			m_SubmitSemaphore = nullptr;
		} );

		SubmitTerminateResource( [&]() 
		{
			m_RendererDescriptorSet = nullptr;
		} );

		if( m_FlightFences.size() )
		{
			for( int i = 0; i < m_FlightFences.size(); i++ )
			{
				vkDestroyFence( VulkanContext::Get().GetDevice(), m_FlightFences[ i ], nullptr );
			}
		}

		for ( auto& rFunc : m_TerminateResourceFuncs )
			rFunc();

		m_PinkTexture->SetForceTerminate( true );
		m_PinkTexture = nullptr;
	}

	void Renderer::SubmitFullscreenQuad(
		VkCommandBuffer CommandBuffer, Ref<Saturn::Pipeline> Pipeline, 
		Ref< DescriptorSet >& rDescriptorSet, 
		IndexBuffer* pIndexBuffer, VertexBuffer* pVertexBuffer )
	{
		Pipeline->Bind( CommandBuffer );
		
		if( rDescriptorSet )
			rDescriptorSet->Bind( CommandBuffer, Pipeline->GetPipelineLayout() );

		pVertexBuffer->Bind( CommandBuffer );
		pIndexBuffer->Bind( CommandBuffer );

		pIndexBuffer->Draw( CommandBuffer );
	}

	void Renderer::BeginRenderPass( VkCommandBuffer CommandBuffer, Pass& rPass )
	{
	}

	void Renderer::EndRenderPass( VkCommandBuffer CommandBuffer )
	{
		vkCmdEndRenderPass( CommandBuffer );
	}
	
	void Renderer::RenderMeshWithoutMaterial( VkCommandBuffer CommandBuffer, Ref<Saturn::Pipeline> Pipeline, Ref<Mesh> mesh, const glm::mat4 transform, Buffer additionalData )
	{	
		Buffer PushConstant;
		// sizeof glm::mat4 becuase we have the model matrix in the push constant plus any additional data.
		PushConstant.Allocate( sizeof( glm::mat4 ) + additionalData.Size );
		if( additionalData.Size > 0 )
			PushConstant.Write( additionalData.Data, additionalData.Size, sizeof( glm::mat4 ) ); // Make sure the additional data is always after the model matrix.

		for ( auto& rSubmesh : mesh->Submeshes() )
		{
			mesh->GetVertexBuffer()->Bind( CommandBuffer );
			mesh->GetIndexBuffer()->Bind( CommandBuffer );

			Pipeline->Bind( CommandBuffer );

			glm::mat4 ModelMatrix = transform * rSubmesh.Transform;
			
			PushConstant.Write( &ModelMatrix, sizeof( glm::mat4 ), 0 );

			// Always assume that the vertex shader will have a push constant block containing the model matrix.
			vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, PushConstant.Size, PushConstant.Data );
			
			Pipeline->GetDescriptorSet( ShaderType::Vertex, 0 )->Bind( CommandBuffer, Pipeline->GetPipelineLayout() );

			vkCmdDrawIndexed( CommandBuffer, rSubmesh.IndexCount, 1, rSubmesh.BaseIndex, rSubmesh.BaseVertex, 0 );
		}

		PushConstant.Free();
		PushConstant.Zero_Memory();
	}

	void Renderer::RenderSubmesh(
		VkCommandBuffer CommandBuffer, 
		Ref< Saturn::Pipeline > Pipeline,
		Ref< Mesh > mesh,
		Submesh& rSubmsh, const glm::mat4 transform )
	{
		auto& materials = mesh->GetMaterials();
		auto& material = materials[ rSubmsh.MaterialIndex ];

		// Bind material.
		material->Bind( mesh, rSubmsh, mesh->GetShader() );

		// Draw.
		vkCmdDrawIndexed( CommandBuffer, rSubmsh.IndexCount, 1, rSubmsh.BaseIndex, rSubmsh.BaseVertex, 0 );
	}
	
	void Renderer::SubmitMesh( VkCommandBuffer CommandBuffer, Ref< Saturn::Pipeline > Pipeline, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		Ref<Shader> Shader = Pipeline->GetShader();

		for( Submesh& rSubmesh : mesh->Submeshes() )
		{
			auto& rMaterial = mesh->GetMaterials()[ rSubmesh.MaterialIndex ];
			Ref< DescriptorSet > Set = mesh->GetDescriptorSets()[ rSubmesh ];

			Shader->WriteAllUBs( Set );

			mesh->GetVertexBuffer()->Bind( CommandBuffer );
			mesh->GetIndexBuffer()->Bind( CommandBuffer );

			Pipeline->Bind( CommandBuffer );

			glm::mat4 ModelMatrix = transform * rSubmesh.Transform;

			vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( glm::mat4 ), &ModelMatrix );
						
			// Set the offset to be the size of the vertex push constant.
			vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof( glm::mat4 ), rMaterial->GetPushConstantData().Size, rMaterial->GetPushConstantData().Data );

			rMaterial->Bind( mesh, rSubmesh, Shader );

			// DescriptorSet 0, for material texture data.
			// DescriptorSet 1, for environment data.
			std::array<VkDescriptorSet, 2> DescriptorSets = {
				Set->GetVulkanSet(),
				m_RendererDescriptorSet->GetVulkanSet()
			};

			vkCmdBindDescriptorSets( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				Pipeline->GetPipelineLayout(), 0, DescriptorSets.size(), DescriptorSets.data(), 0, nullptr );

			vkCmdDrawIndexed( CommandBuffer, rSubmesh.IndexCount, 1, rSubmesh.BaseIndex, rSubmesh.BaseVertex, 0 );
		}
	}

	void Renderer::Begin( Ref<Image2D> ShadowMap )
	{
		Ref<Shader> shader = ShaderLibrary::Get().Find( "shader_new" );

		shader->WriteDescriptor( "u_ShadowMap", ShadowMap->GetDescriptorInfo(), m_RendererDescriptorSet->GetVulkanSet() );
	}

	VkCommandBuffer Renderer::AllocateCommandBuffer( VkCommandPool CommandPool )
	{
		VkCommandBufferAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		AllocateInfo.commandPool = VulkanContext::Get().GetCommandPool();
		AllocateInfo.commandBufferCount = 1;
		AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		
		VkCommandBuffer CommandBuffer;
		VK_CHECK( vkAllocateCommandBuffers( VulkanContext::Get().GetDevice(), &AllocateInfo, &CommandBuffer ) );

		VkCommandBufferBeginInfo CommandPoolBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		CommandPoolBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK( vkBeginCommandBuffer( CommandBuffer, &CommandPoolBeginInfo ) );

		return CommandBuffer;
	}

	VkCommandBuffer Renderer::AllocateCommandBuffer( VkCommandBufferLevel CmdLevel )
	{
		VkCommandBufferAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		AllocateInfo.commandPool = VulkanContext::Get().GetCommandPool();
		AllocateInfo.commandBufferCount = 1;
		AllocateInfo.level = CmdLevel;

		VkCommandBuffer CommandBuffer;
		VK_CHECK( vkAllocateCommandBuffers( VulkanContext::Get().GetDevice(), &AllocateInfo, &CommandBuffer ) );

		return CommandBuffer;
	}

	void Renderer::BeginFrame()
	{
		m_BeginFrameTimer.Reset();

		m_CommandBuffer = AllocateCommandBuffer( VulkanContext::Get().GetCommandPool() );

		VkDevice LogicalDevice = VulkanContext::Get().GetDevice();

		// Wait for last frame.
		VK_CHECK( vkWaitForFences( LogicalDevice, 1, &m_FlightFences[ m_FrameCount ], VK_TRUE, UINT32_MAX ) );

		// Reset current fence.
		VK_CHECK( vkResetFences( LogicalDevice, 1, &m_FlightFences[ m_FrameCount ] ) );

		// Acquire next image.
		uint32_t ImageIndex;
		if( !VulkanContext::Get().GetSwapchain().AcquireNextImage( UINT64_MAX, m_AcquireSemaphore, VK_NULL_HANDLE, &ImageIndex ) )
			SAT_CORE_ASSERT( false );

		m_ImageIndex = ImageIndex;

		if( ImageIndex == UINT32_MAX || ImageIndex == 3435973836 )
			SAT_CORE_ASSERT( false );

		m_BeginFrameTime = m_BeginFrameTimer.ElapsedMilliseconds();
	}

	void Renderer::EndFrame()
	{
		m_EndFrameTimer.Reset();

		VkDevice LogicalDevice = VulkanContext::Get().GetDevice();

		VK_CHECK( vkEndCommandBuffer( m_CommandBuffer ) );

		// Rendering Queue
		VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &m_CommandBuffer;
		SubmitInfo.pWaitDstStageMask = &WaitStage;

		// SIGNAL the SubmitSemaphore
		SubmitInfo.pSignalSemaphores = &m_SubmitSemaphore;
		SubmitInfo.signalSemaphoreCount = 1;

		// WAIT for AcquireSemaphore
		SubmitInfo.pWaitSemaphores = &m_AcquireSemaphore;
		SubmitInfo.waitSemaphoreCount = 1;

		VK_CHECK( vkResetFences( LogicalDevice, 1, &m_FlightFences[ m_FrameCount ] ) );

		// Use current fence to be signaled.
		VK_CHECK( vkQueueSubmit( VulkanContext::Get().GetGraphicsQueue(), 1, &SubmitInfo, m_FlightFences[ m_FrameCount ] ) );

		// Present info.
		VkPresentInfoKHR PresentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		PresentInfo.pSwapchains = &VulkanContext::Get().GetSwapchain().GetSwapchain();
		PresentInfo.swapchainCount = 1;
		PresentInfo.pImageIndices = &m_ImageIndex;

		// WAIT for SubmitSemaphore
		PresentInfo.pWaitSemaphores = &m_SubmitSemaphore;
		PresentInfo.waitSemaphoreCount = 1;
		
		m_QueuePresentTimer.Reset();

		VkResult Result = vkQueuePresentKHR( VulkanContext::Get().GetGraphicsQueue(), &PresentInfo );

		if( Result == VK_ERROR_OUT_OF_DATE_KHR ) 
		{
			SAT_CORE_INFO( "VK_ERROR_OUT_OF_DATE_KHR, Swapchain will be re-created" );

			VulkanContext::Get().GetSwapchain().Recreate();

			PresentInfo.pSwapchains = &VulkanContext::Get().GetSwapchain().GetSwapchain();

			VK_CHECK( vkQueuePresentKHR( VulkanContext::Get().GetGraphicsQueue(), &PresentInfo ) );
		}

		m_QueuePresentTime = m_QueuePresentTimer.ElapsedMilliseconds();

		VK_CHECK( vkQueueWaitIdle( VulkanContext::Get().GetPresentQueue() ) );

		vkFreeCommandBuffers( LogicalDevice, VulkanContext::Get().GetCommandPool(), 1, &m_CommandBuffer );

		m_FrameCount = ( m_FrameCount + 1 ) % MAX_FRAMES_IN_FLIGHT;

		m_EndFrameTime = m_EndFrameTimer.ElapsedMilliseconds() - m_QueuePresentTime;
	}

	void Renderer::SubmitTerminateResource( std::function<void()>&& rrFunction )
	{
		m_TerminateResourceFuncs.push_back( rrFunction );
	}

	void Renderer::CreateFullscreenQuad( VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer )
	{
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[ 4 ];

		data[ 0 ].Position = glm::vec3( -1, -1, 0.1f );
		data[ 0 ].TexCoord = glm::vec2( 0, 0 );

		data[ 1 ].Position = glm::vec3( -1 + 2, -1, 0.1f );
		data[ 1 ].TexCoord = glm::vec2( 1, 0 );

		data[ 2 ].Position = glm::vec3( -1 + 2, -1 + 2, 0.1f );
		data[ 2 ].TexCoord = glm::vec2( 1, 1 );

		data[ 3 ].Position = glm::vec3( -1, -1 + 2, 0.1f );
		data[ 3 ].TexCoord = glm::vec2( 0, 1 );

		*ppVertexBuffer = new VertexBuffer( data, 4 * sizeof( QuadVertex ) );

		uint32_t indices[ 6 ] = { 0, 1, 2,
								  2, 3, 0, };

		*ppIndexBuffer = new IndexBuffer( indices, 6 );
	}
}