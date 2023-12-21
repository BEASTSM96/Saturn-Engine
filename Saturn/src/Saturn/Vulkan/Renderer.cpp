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

#include "sppch.h"
#include "Renderer.h"

#include "Saturn/Core/Renderer/RenderThread.h"

#include "VulkanDebug.h"
#include "DescriptorSet.h"
#include "MaterialInstance.h"
#include "Shader.h"

#include "Saturn/Core/OptickProfiler.h"

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////

	Renderer::Renderer()
	{
		SingletonStorage::AddSingleton<Renderer>( this );
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Init()
	{
		SAT_PF_EVENT();

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

		uint32_t* pData = new uint32_t[ 1 * 1 ];
		memset( pData, 0, sizeof( uint32_t ) * 1 * 1 );

		for( uint32_t i = 0; i < 1 * 1; i++ )
		{
			pData[ i ] |= 0xFFFFFFFF;
		}
		
		// It's really a white texture...
		m_PinkTexture = Ref< Texture2D >::Create( ImageFormat::RGBA8, 1, 1, pData );
		m_PinkTexture->SetIsRendererTexture( true );

		m_PinkTextureCube = Ref< TextureCube >::Create( ImageFormat::BGRA8, 1, 1, pData );

		delete[] pData;

		std::vector<VkDescriptorPoolSize> PoolSizes;

		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 } );

		for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			m_RendererDescriptorPools[i] = Ref<DescriptorPool>::Create( PoolSizes, 100000 );
		}
	}
	
	void Renderer::Terminate()
	{
		// Terminate Semaphores.
		if( m_AcquireSemaphore )
			vkDestroySemaphore( VulkanContext::Get().GetDevice(), m_AcquireSemaphore, nullptr );
			
		if( m_SubmitSemaphore )
			vkDestroySemaphore( VulkanContext::Get().GetDevice(), m_SubmitSemaphore, nullptr );

		m_AcquireSemaphore = nullptr;
		m_SubmitSemaphore = nullptr;

		for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			m_RendererDescriptorSets[ i ] = nullptr;
			m_RendererDescriptorPools[ i ] = nullptr;
		}

		if( m_FlightFences.size() )
		{
			for( int i = 0; i < m_FlightFences.size(); i++ )
			{
				vkDestroyFence( VulkanContext::Get().GetDevice(), m_FlightFences[ i ], nullptr );
			}
		}

		for ( auto& rFunc : m_TerminateResourceFuncs )
			rFunc();

		m_PinkTextureCube->Terminate();
		m_PinkTextureCube = nullptr;

		m_PinkTexture->SetForceTerminate( true );
		m_PinkTexture = nullptr;
	}

	void Renderer::SubmitFullscreenQuad(
		VkCommandBuffer CommandBuffer, Ref<Saturn::Pipeline> Pipeline, 
		Ref< DescriptorSet >& rDescriptorSet, 
		Ref<IndexBuffer> IndexBuffer, Ref<VertexBuffer> VertexBuffer )
	{
		SAT_PF_EVENT();

		Pipeline->Bind( CommandBuffer );
		
		if( rDescriptorSet )
			rDescriptorSet->Bind( CommandBuffer, Pipeline->GetPipelineLayout() );

		VertexBuffer->Bind( CommandBuffer );
		IndexBuffer->Bind( CommandBuffer );

		IndexBuffer->Draw( CommandBuffer );
	}

	void Renderer::BeginRenderPass( VkCommandBuffer CommandBuffer, Pass& rPass )
	{
	}

	void Renderer::EndRenderPass( VkCommandBuffer CommandBuffer )
	{
		vkCmdEndRenderPass( CommandBuffer );
	}
	
	void Renderer::RenderMeshWithoutMaterial( VkCommandBuffer CommandBuffer, Ref<Saturn::Pipeline> Pipeline, Ref<StaticMesh> mesh, uint32_t count, Ref<VertexBuffer> transformVB, uint32_t TransformOffset, uint32_t SubmeshIndex, Buffer additionalData )
	{	
		SAT_PF_EVENT();

		Buffer PushConstant;
		PushConstant.Allocate( additionalData.Size );
		if( additionalData.Size > 0 )
			PushConstant.Write( additionalData.Data, additionalData.Size, 0 );

		auto& rSubmesh = mesh->Submeshes()[ SubmeshIndex ];
		{
			mesh->GetVertexBuffer()->Bind( CommandBuffer );

			VkDeviceSize offset[ 1 ] = { TransformOffset };
			transformVB->Bind( CommandBuffer, 1, offset );

			mesh->GetIndexBuffer()->Bind( CommandBuffer );

			Pipeline->Bind( CommandBuffer );

			if( PushConstant.Size > 0 )
			{
				vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, ( uint32_t ) PushConstant.Size, PushConstant.Data );
			}
			
			Pipeline->GetDescriptorSet( ShaderType::Vertex, 0 )->Bind( CommandBuffer, Pipeline->GetPipelineLayout() );

			vkCmdDrawIndexed( CommandBuffer, rSubmesh.IndexCount, count, rSubmesh.BaseIndex, rSubmesh.BaseVertex, 0 );
		}

		PushConstant.Free();
		PushConstant.Zero_Memory();
	}

	void Renderer::RenderSubmesh(
		VkCommandBuffer CommandBuffer, 
		Ref< Saturn::Pipeline > Pipeline,
		Ref< StaticMesh > mesh,
		Submesh& rSubmsh, const glm::mat4 transform )
	{
	}
	
	const std::vector<VkWriteDescriptorSet>& Renderer::GetStorageBufferWriteDescriptors( Ref<StorageBufferSet>& rStorageBufferSet, Ref<MaterialAsset>& rMaterialAsset )
	{
		SAT_PF_EVENT();

		Ref<Shader> shader = rMaterialAsset->GetMaterial()->GetShader();
		std::string shaderName = shader->GetName();
		
		if( m_StorageBufferSets.find( m_FrameCount ) != m_StorageBufferSets.end() ) 
		{
			const auto& buffersInFrame = m_StorageBufferSets.at( m_FrameCount );

			if( buffersInFrame.find( shaderName ) != buffersInFrame.end() )
			{
				const auto& wd = buffersInFrame.at( shaderName );
				return wd;
			}
		}

		// Does not exist, add and create.
		auto& descriptorSet = shader->GetShaderDescriptorSet( 0 );

		for( auto&& [binding, sb] : descriptorSet.StorageBuffers )
		{
			auto& wd = m_StorageBufferSets[ m_FrameCount ][ shaderName ];

			for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
			{
				Ref<StorageBuffer> sb = rStorageBufferSet->Get( 0, binding, m_FrameCount );
				
				VkWriteDescriptorSet wds = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				wds.descriptorCount = 1;
				wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				wds.pBufferInfo = &sb->GetBufferInfo();
				wds.dstBinding = sb->GetBinding();
				// We don't know the descriptor set yet so we don't set it. It will be updated when we bind the material.

				wd.push_back( wds );
			}
		}

		return m_StorageBufferSets[ m_FrameCount ][ shaderName ];
	}

	void Renderer::SubmitMesh( 
		VkCommandBuffer CommandBuffer, Ref< Saturn::Pipeline > Pipeline, Ref< StaticMesh > mesh, 
		Ref<StorageBufferSet>& rStorageBufferSet, Ref< MaterialRegistry > materialRegistry, 
		uint32_t SubmeshIndex, uint32_t count, Ref<VertexBuffer> transformData, uint32_t transformOffset )
	{
		SAT_PF_EVENT();

		Ref<Shader> Shader = Pipeline->GetShader();

		VkDeviceSize transformOffsets[ 1 ] = { transformOffset };

		mesh->GetVertexBuffer()->Bind( CommandBuffer );
		transformData->Bind( CommandBuffer, 1, transformOffsets );

		mesh->GetIndexBuffer()->Bind( CommandBuffer );
		Pipeline->Bind( CommandBuffer );

		Submesh& rSubmesh = mesh->Submeshes()[ SubmeshIndex ];
		{
			auto& rMaterialAsset = materialRegistry->GetMaterials()[ rSubmesh.MaterialIndex ];

			const auto& StorageWriteDescriptors = GetStorageBufferWriteDescriptors( rStorageBufferSet, rMaterialAsset );

			rMaterialAsset->Bind( mesh, rSubmesh, Shader, StorageWriteDescriptors[ m_FrameCount ] );

			VkDescriptorSet Set = rMaterialAsset->GetMaterial()->GetDescriptorSet( m_FrameCount );

			vkCmdPushConstants( CommandBuffer, Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, ( uint32_t ) rMaterialAsset->GetPushConstantData().Size, rMaterialAsset->GetPushConstantData().Data );

			// Descriptor set 0, for material texture data.
			// Descriptor set 1, for environment data.
			std::array<VkDescriptorSet, 2> DescriptorSets = {
				Set,
				m_RendererDescriptorSets[ m_FrameCount ]
			};

			vkCmdBindDescriptorSets( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				Pipeline->GetPipelineLayout(), 0, ( uint32_t ) DescriptorSets.size(), DescriptorSets.data(), 0, nullptr );

			vkCmdDrawIndexed( CommandBuffer, rSubmesh.IndexCount, count, rSubmesh.BaseIndex, rSubmesh.BaseVertex, 0 );
		}
	}

	void Renderer::SetSceneEnvironment( Ref<Image2D> ShadowMap, Ref<EnvironmentMap> Environment, Ref<Texture2D> BDRF )
	{
		SAT_PF_EVENT();

		Ref<Shader> shader = ShaderLibrary::Get().Find( "shader_new" );

		m_RendererDescriptorSets[ m_FrameCount ] = shader->AllocateDescriptorSet( 1, true );

		shader->WriteDescriptor( "u_ShadowMap", ShadowMap->GetDescriptorInfo(), m_RendererDescriptorSets[ m_FrameCount ] );
		shader->WriteDescriptor( "u_BRDFLUTTexture", BDRF->GetDescriptorInfo(), m_RendererDescriptorSets[ m_FrameCount ] );

		if( Environment && Environment->RadianceMap && Environment->IrradianceMap )
		{
			shader->WriteDescriptor( "u_EnvRadianceTex", Environment->RadianceMap->GetDescriptorInfo(), m_RendererDescriptorSets[ m_FrameCount ] );
			shader->WriteDescriptor( "u_EnvIrradianceTex", Environment->IrradianceMap->GetDescriptorInfo(), m_RendererDescriptorSets[ m_FrameCount ] );
		}
		else
		{
			shader->WriteDescriptor( "u_EnvRadianceTex", m_PinkTextureCube->GetDescriptorInfo(), m_RendererDescriptorSets[ m_FrameCount ] );
			shader->WriteDescriptor( "u_EnvIrradianceTex", m_PinkTextureCube->GetDescriptorInfo(), m_RendererDescriptorSets[ m_FrameCount ] );
		}
	}

	VkCommandBuffer Renderer::AllocateCommandBuffer( VkCommandPool CommandPool )
	{
		SAT_PF_EVENT();

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
		SAT_PF_EVENT();

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
		SAT_PF_EVENT();

		VkDevice LogicalDevice = VulkanContext::Get().GetDevice();

		m_BeginFrameTimer.Reset();

		VK_CHECK( vkResetDescriptorPool( LogicalDevice, m_RendererDescriptorPools[ m_FrameCount ]->GetVulkanPool(), 0 ) );

		m_CommandBuffer = AllocateCommandBuffer( VulkanContext::Get().GetCommandPool() );

		// Wait for last frame.
		VK_CHECK( vkWaitForFences( LogicalDevice, 1, &m_FlightFences[ m_FrameCount ], VK_TRUE, UINT32_MAX ) );

		// Reset current fence.
		VK_CHECK( vkResetFences( LogicalDevice, 1, &m_FlightFences[ m_FrameCount ] ) );

		// Acquire next image.
		uint32_t ImageIndex = -1;
		SAT_CORE_ASSERT( VulkanContext::Get().GetSwapchain().AcquireNextImage( UINT32_MAX, m_AcquireSemaphore, VK_NULL_HANDLE, &ImageIndex ), "AcquireNextImage failed" );

		m_ImageIndex = ImageIndex;

		SAT_CORE_ASSERT( ImageIndex != UINT32_MAX || ImageIndex != 3435973836 );

		m_BeginFrameTime = m_BeginFrameTimer.ElapsedMilliseconds();
	}

	void Renderer::EndFrame()
	{
		SAT_PF_EVENT();

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
			SAT_CORE_INFO( "Result was VK_ERROR_OUT_OF_DATE_KHR, Swapchain will be re-created!" );

			VulkanContext::Get().GetSwapchain().Recreate();

			PresentInfo.pSwapchains = &VulkanContext::Get().GetSwapchain().GetSwapchain();

			VK_CHECK( vkQueuePresentKHR( VulkanContext::Get().GetGraphicsQueue(), &PresentInfo ) );
		}

		m_QueuePresentTime = m_QueuePresentTimer.ElapsedMilliseconds();

		VK_CHECK( vkQueueWaitIdle( VulkanContext::Get().GetPresentQueue() ) );

		vkFreeCommandBuffers( LogicalDevice, VulkanContext::Get().GetCommandPool(), 1, &m_CommandBuffer );

		m_FrameCount = ( m_FrameCount + 1 ) % MAX_FRAMES_IN_FLIGHT;

		m_EndFrameTime = m_EndFrameTimer.ElapsedMilliseconds() - m_QueuePresentTime;

		// This is a hack, but for now we will do this as in the LightCulling pass we resize the buffer every frame meaning we have to update out cache.
		m_StorageBufferSets.clear();
	}

	void Renderer::SubmitTerminateResource( std::function<void()>&& rrFunction )
	{
		m_TerminateResourceFuncs.push_back( rrFunction );
	}

	std::pair< Ref<VertexBuffer>, Ref<IndexBuffer> > Renderer::CreateFullscreenQuad()
	{
		Ref<VertexBuffer> vertex = nullptr;
		Ref<IndexBuffer> index = nullptr;

		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		float x = -1;
		float y = -1;

		float width = 2, height = 2;

		QuadVertex* data = new QuadVertex[ 4 ];

		data[ 0 ].Position = glm::vec3( x, y, 0.0f );
		data[ 0 ].TexCoord = glm::vec2( 0, 0 );

		data[ 1 ].Position = glm::vec3( x + width, y, 0.0f );
		data[ 1 ].TexCoord = glm::vec2( 1, 0 );

		data[ 2 ].Position = glm::vec3( x + width, y + height, 0.0f );
		data[ 2 ].TexCoord = glm::vec2( 1, 1 );

		data[ 3 ].Position = glm::vec3( x, y + height, 0.0f );
		data[ 3 ].TexCoord = glm::vec2( 0, 1 );

		vertex = Ref<VertexBuffer>::Create( data, 4 * sizeof( QuadVertex ) );

		uint32_t indices[ 6 ] = { 0, 1, 2,
								  2, 3, 0, };

		index = Ref<IndexBuffer>::Create( indices, 6 * sizeof( uint32_t ) );

		return { vertex, index };
	}
}