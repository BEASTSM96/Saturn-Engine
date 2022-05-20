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
	}
	
	void Renderer::Terminate()
	{
		// Terminate Semaphores.
		SubmitTerminateResource( [AcquireSemaphore = m_AcquireSemaphore, SubmitSemaphore = m_SubmitSemaphore]()
		{
			if( AcquireSemaphore )
				vkDestroySemaphore( VulkanContext::Get().GetDevice(), AcquireSemaphore, nullptr );
			
			if( SubmitSemaphore )
				vkDestroySemaphore( VulkanContext::Get().GetDevice(), SubmitSemaphore, nullptr );
		} );

		m_AcquireSemaphore = nullptr;
		m_SubmitSemaphore = nullptr;

		if( m_FlightFences.size() )
		{
			for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
			{
				SubmitTerminateResource( [ FlightFences = m_FlightFences, Index = i ]()
				{
					vkDestroyFence( VulkanContext::Get().GetDevice(), FlightFences.at( Index ), nullptr );
				} );
			}
		}

		for ( auto& rFunc : m_TerminateResourceFuncs )
			rFunc();
	}

	void Renderer::SubmitFullscrenQuad( VkCommandBuffer CommandBuffer, Saturn::Pipeline Pipeline )
	{
		// TODO
	}

	void Renderer::SubmitFullscrenQuad( VkCommandBuffer CommandBuffer, Saturn::Pipeline Pipeline, VkDescriptorSet DescriptorSet, void* UBO )
	{
		// TODO
	}

	void Renderer::SubmitFullscrenQuad( VkCommandBuffer CommandBuffer, Saturn::Pipeline Pipeline, VkDescriptorSet DescriptorSet, UniformBuffer* UBO, IndexBuffer* pIndexBuffer, VertexBuffer* pVertexBuffer )
	{
		vkCmdBindPipeline( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipeline() );

		vkCmdBindDescriptorSets( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipelineLayout(), 0, 1, &DescriptorSet, 0, nullptr );
		
		// Update UBO
		{
			UBO->Map( CommandBuffer );
		}

		pIndexBuffer->Bind( CommandBuffer );
		pVertexBuffer->Bind( CommandBuffer );

		pIndexBuffer->Draw( CommandBuffer );
	}

	void Renderer::BeginRenderPass( VkCommandBuffer CommandBuffer, Pass& rPass )
	{
	}

	void Renderer::BeginRenderPass( VkCommandBuffer CommandBuffer )
	{
		vkCmdEndRenderPass( CommandBuffer );
	}

	void Renderer::RenderMeshWithMaterial()
	{
		// TODO:
	}

	void Renderer::RenderStaticMesh(
		VkCommandBuffer CommandBuffer,
		Saturn::Pipeline Pipeline,
		UUID uuid, 
		Ref< Mesh > mesh, 
		const glm::mat4 transform, 
		UniformBuffer& rUBO )
	{
		// Bind pipeline.
		Pipeline.Bind( CommandBuffer );

		mesh->GetVertexBuffer()->Bind( CommandBuffer );
		mesh->GetIndexBuffer()->Bind( CommandBuffer );

		// Bind UBO
		rUBO.Map( CommandBuffer );
		
		// Bind material.
		mesh->GetMaterial()->Bind( nullptr );

		/*
		for ( Submesh& rSubmesh : mesh->Submeshes() )
		{
			auto mat = mesh->GetMaterial();

			// Bind material.
			
			// Draw.
			vkCmdDrawIndexed( CommandBuffer, rSubmesh.IndexCount, 1, rSubmesh.BaseIndex, 0, 0 );
		}
		*/

		mesh->GetIndexBuffer()->Draw( CommandBuffer );
	}

	void Renderer::RenderSubmesh(
		VkCommandBuffer CommandBuffer, 
		Saturn::Pipeline Pipeline, 
		Ref< Mesh > mesh,
		Submesh& rSubmsh, const glm::mat4 transform, UniformBuffer& rUBO )
	{

		// Bind UBO
		rUBO.Map( CommandBuffer );

		// Bind material.
		mesh->GetMaterial()->Bind( nullptr );

		// Draw.
		vkCmdDrawIndexed( CommandBuffer, rSubmsh.IndexCount, 1, rSubmsh.BaseIndex, 0, 0 );
	}

	void Renderer::RenderSubmesh(
		VkCommandBuffer CommandBuffer,
		Saturn::Pipeline Pipeline,
		Ref< Mesh > mesh,
		Submesh& rSubmsh, const glm::mat4 transform, Ref< UniformBuffer > UBO )
	{

		// Bind UBO
		UBO->Map( CommandBuffer );

		// Bind material.
		mesh->GetMaterial()->Bind( nullptr );

		// Draw.
		vkCmdDrawIndexed( CommandBuffer, rSubmsh.IndexCount, 1, rSubmsh.BaseIndex, 0, 0 );
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

		VK_CHECK( vkQueuePresentKHR( VulkanContext::Get().GetGraphicsQueue(), &PresentInfo ) );

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

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////

	void Renderer::CreateFramebuffer( VkRenderPass RenderPass, VkExtent2D Extent, std::vector< VkImageView > Attachments, VkFramebuffer* pFramebuffer )
	{
		VkFramebufferCreateInfo FramebufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		FramebufferCreateInfo.renderPass = RenderPass;
		FramebufferCreateInfo.attachmentCount = Attachments.size();
		FramebufferCreateInfo.pAttachments = Attachments.data();
		FramebufferCreateInfo.width = Extent.width;
		FramebufferCreateInfo.height = Extent.height;
		FramebufferCreateInfo.layers = 1;
		
		VK_CHECK( vkCreateFramebuffer( VulkanContext::Get().GetDevice(), &FramebufferCreateInfo, nullptr, pFramebuffer ) );
	}

	void Renderer::CreateImage( VkImageType Type, VkFormat Format, VkExtent3D Extent, VkImageUsageFlags Usage, VkImage* pImage, VkDeviceMemory* pMemory )
	{
		VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		ImageCreateInfo.imageType = Type;
		ImageCreateInfo.format = Format;
		ImageCreateInfo.extent = Extent;
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage = Usage;

		VK_CHECK( vkCreateImage( VulkanContext::Get().GetDevice(), &ImageCreateInfo, nullptr, pImage ) );

		VkMemoryRequirements MemoryRequirements;
		vkGetImageMemoryRequirements( VulkanContext::Get().GetDevice(), *pImage, &MemoryRequirements );

		VkMemoryAllocateInfo MemoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &MemoryAllocateInfo, nullptr, pMemory ) );
		VK_CHECK( vkBindImageMemory( VulkanContext::Get().GetDevice(), *pImage, *pMemory, 0 ) );
	}

	void Renderer::CreateImageView( VkImage Image, VkFormat Format, VkImageAspectFlags Aspect, VkImageView* pImageView )
	{
		VkImageViewCreateInfo ImageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ImageViewCreateInfo.image = Image;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = Format;
		ImageViewCreateInfo.subresourceRange.aspectMask = Aspect;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;
		
		VK_CHECK( vkCreateImageView( VulkanContext::Get().GetDevice(), &ImageViewCreateInfo, nullptr, pImageView ) );
	}

	void Renderer::CreateSampler( VkFilter Filter, VkSampler* pSampler )
	{
		VkSamplerCreateInfo SamplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		SamplerCreateInfo.magFilter = Filter;
		SamplerCreateInfo.minFilter = Filter;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		SamplerCreateInfo.mipLodBias = 0.0f;
		SamplerCreateInfo.maxAnisotropy = 1.0f;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		SamplerCreateInfo.minLod = 0.0f;
		SamplerCreateInfo.maxLod = 1.0f;
		SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VK_CHECK( vkCreateSampler( VulkanContext::Get().GetDevice(), &SamplerCreateInfo, nullptr, pSampler ) );
	}

	void Resource::Terminate()
	{
		if( Image )
		{
			vkDestroyImage( VulkanContext::Get().GetDevice(), Image, nullptr );
			Image = nullptr;
		}

		if( Memory )
		{
			vkFreeMemory( VulkanContext::Get().GetDevice(), Memory, nullptr );
			Memory = nullptr;
		}

		if( ImageView )
		{
			vkDestroyImageView( VulkanContext::Get().GetDevice(), ImageView, nullptr );
			ImageView = nullptr;
		}

		if( Sampler )
		{
			vkDestroySampler( VulkanContext::Get().GetDevice(), Sampler, nullptr );
			Sampler = nullptr;
		}
	}

}