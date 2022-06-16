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
			pData[ i ] |= 0xffff00ff;
		}

		m_PinkTexture = Ref< Texture2D >::Create( 64, 64, VK_FORMAT_R8G8B8A8_SRGB, pData );
		m_PinkTexture->SetIsRendererTexture( true );

		free( pData );
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

	void Renderer::RenderMeshWithMaterial( VkCommandBuffer CommandBuffer, Ref<Saturn::Pipeline> Pipeline, Ref<Mesh> mesh, const glm::mat4 transform )
	{	
		struct PC_StaticMeshF
		{
			alignas( 4 ) float UseAlbedoTexture;
			alignas( 4 ) float UseMetallicTexture;
			alignas( 4 ) float UseRoughnessTexture;
			alignas( 4 ) float UseNormalTexture;

			alignas( 16 ) glm::vec4 AlbedoColor;
			alignas( 4 ) float Metalness;
			alignas( 4 ) float Roughness;
		};

		//mesh->GetShader()->GetPushConstantData();

		mesh->GetVertexBuffer()->Bind( CommandBuffer );
		mesh->GetIndexBuffer()->Bind( CommandBuffer );

		for ( auto& rSubmesh : mesh->Submeshes() )
		{
			Pipeline->Bind( CommandBuffer );

			glm::mat4 ModelMatrix = transform * rSubmesh.Transform;

			// Always assume that the vertex shader will have a push constant block containing the model matrix.
			vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( glm::mat4 ), &ModelMatrix );
			vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( glm::mat4 ), &ModelMatrix );
		}
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
		struct PC_StaticMesh
		{
			alignas( 4 ) float UseAlbedoTexture;
			alignas( 4 ) float UseMetallicTexture;
			alignas( 4 ) float UseRoughnessTexture;
			alignas( 4 ) float UseNormalTexture;

			alignas( 16 ) glm::vec4 AlbedoColor;
			alignas( 4 ) float Metalness;
			alignas( 4 ) float Roughness;
		};

		Ref<Shader> StaticMeshShader = ShaderLibrary::Get().Find( "shader_new" );

		for( Submesh& rSubmesh : mesh->Submeshes() )
		{
			auto& rMaterial = mesh->GetMaterials()[ rSubmesh.MaterialIndex ];
			Ref< DescriptorSet > Set = mesh->GetDescriptorSets()[ rSubmesh ];

			StaticMeshShader->WriteAllUBs( Set );

			mesh->GetVertexBuffer()->Bind( CommandBuffer );
			mesh->GetIndexBuffer()->Bind( CommandBuffer );

			Pipeline->Bind( CommandBuffer );

			glm::mat4 ModelMatrix = transform * rSubmesh.Transform;

			PC_StaticMesh PushConstantData = {};
			PushConstantData.UseNormalTexture = rMaterial->Get<float>( "u_Materials.UseNormalTexture" ) ? 1.0f : 0.0f;
			PushConstantData.UseMetallicTexture = rMaterial->Get<float>( "u_Materials.UseMetallicTexture" ) ? 1.0f : 0.0f;
			PushConstantData.UseRoughnessTexture = rMaterial->Get<float>( "u_Materials.UseRoughnessTexture" ) ? 1.0f : 0.0f;
			PushConstantData.UseAlbedoTexture = rMaterial->Get<float>( "u_Materials.UseAlbedoTexture" ) ? 1.0f : 0.0f;

			PushConstantData.AlbedoColor = rMaterial->Get<glm::vec4>( "u_Materials.AlbedoColor" );
			PushConstantData.Metalness = 0.0f;
			PushConstantData.Roughness = 0.0f;

			vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( glm::mat4 ), &ModelMatrix );
			vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( PushConstantData ), &PushConstantData );

			rMaterial->Bind( mesh, rSubmesh, StaticMeshShader );

			Set->Bind( CommandBuffer, Pipeline->GetPipelineLayout() );

			vkCmdDrawIndexed( CommandBuffer, rSubmesh.IndexCount, 1, rSubmesh.BaseIndex, rSubmesh.BaseVertex, 0 );
		}
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

	void Renderer::CreateImage( VkImageType Type, VkFormat Format, VkExtent3D Extent, uint32_t ArrayLevels, VkImageUsageFlags Usage, VkImage* pImage, VkDeviceMemory* pMemory )
	{
		VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		ImageCreateInfo.imageType = Type;
		ImageCreateInfo.format = Format;
		ImageCreateInfo.extent = Extent;
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = ArrayLevels;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage = Usage;

		VK_CHECK( vkCreateImage( VulkanContext::Get().GetDevice(), &ImageCreateInfo, nullptr, pImage ) );
		SetDebugUtilsObjectName( "Image", ( uint64_t ) *pImage, VK_OBJECT_TYPE_IMAGE  );

		VkMemoryRequirements MemoryRequirements;
		vkGetImageMemoryRequirements( VulkanContext::Get().GetDevice(), *pImage, &MemoryRequirements );

		VkMemoryAllocateInfo MemoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &MemoryAllocateInfo, nullptr, pMemory ) );
		VK_CHECK( vkBindImageMemory( VulkanContext::Get().GetDevice(), *pImage, *pMemory, 0 ) );
	}

	void Renderer::CreateImageView( VkImageViewType Type, VkImage Image, VkFormat Format, VkImageAspectFlags Aspect, uint32_t LayerCount, VkImageView* pImageView )
	{
		VkImageViewCreateInfo ImageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ImageViewCreateInfo.image = Image;
		ImageViewCreateInfo.viewType = Type;
		ImageViewCreateInfo.format = Format;
		ImageViewCreateInfo.subresourceRange.aspectMask = Aspect;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = LayerCount;

		VK_CHECK( vkCreateImageView( VulkanContext::Get().GetDevice(), &ImageViewCreateInfo, nullptr, pImageView ) );
		SetDebugUtilsObjectName( "Image View", ( uint64_t ) *pImageView, VK_OBJECT_TYPE_IMAGE_VIEW );
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
		SetDebugUtilsObjectName( "Image View 1", ( uint64_t ) *pSampler, VK_OBJECT_TYPE_SAMPLER );
	}
}