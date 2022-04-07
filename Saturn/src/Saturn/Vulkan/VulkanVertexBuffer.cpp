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
#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"

#include <cassert>

namespace Saturn {

	VertexBuffer::VertexBuffer( const std::vector< Vertex >& Vertices )
	{
		m_Vertices = Vertices;
		m_Size = m_Vertices.size();
		m_pData = m_Vertices.data();

		m_Buffer = VK_NULL_HANDLE;
		m_Memory = VK_NULL_HANDLE;
	}

	VertexBuffer::~VertexBuffer()
	{
		Terminate();
	}

	void VertexBuffer::BindAndDraw( VkCommandBuffer CommandBuffer )
	{
		Bind( CommandBuffer );

		// print the vertices
		for( int i = 0; i < m_Size; ++i )
		{
			printf( "Vertex %d: %f %f\n", i, m_Vertices[ i ].Position.x, m_Vertices[ i ].Position.y );
		}

		vkCmdDraw( CommandBuffer, ( size_t )m_Size, 1, 0, 0 );
	}

	void VertexBuffer::Bind( VkCommandBuffer CommandBuffer )
	{
		VkBuffer Buffers[] ={ m_Buffer };
		VkDeviceSize Offsets[] ={ 0 };

		vkCmdBindVertexBuffers( CommandBuffer, 0, 1, Buffers, Offsets );
	}

	void VertexBuffer::Draw( VkCommandBuffer CommandBuffer )
	{
		vkCmdDraw( CommandBuffer, ( size_t )m_Size, 1, 0, 0 );
	}

	std::vector<VkVertexInputBindingDescription> VertexBuffer::GetBindingDescriptions()
	{
		return { {.binding = 0, .stride = sizeof( Vertex ), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX } };
	}

	std::vector<VkVertexInputAttributeDescription> VertexBuffer::GetAttributeDescriptions()
	{
		return {
			{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof( Vertex, Position ) },
			{.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof( Vertex, Color ) }
		};
	}

	void VertexBuffer::CreateBuffer()
	{
	#if 1
		assert( m_Size >= 3 && "Vertex count must be above 3!" );

		VkDeviceSize BufferSize = sizeof( m_Vertices[ 0 ] ) * m_Size;

		// Create staging buffer.
		VkBuffer StagingBuffer;
		VkDeviceMemory StagingBufferMemory;
		VkMemoryRequirements StagingMemoryRequirements;

		VkBufferCreateInfo BufferCreateInfo ={ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = BufferSize;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		VK_CHECK( vkCreateBuffer( VulkanContext::Get().GetDevice(), &BufferCreateInfo, nullptr, &StagingBuffer ) );

		vkGetBufferMemoryRequirements( VulkanContext::Get().GetDevice(), StagingBuffer, &StagingMemoryRequirements );
		
		VkMemoryAllocateInfo StagingMemoryAllocateInfo ={ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		StagingMemoryAllocateInfo.allocationSize = StagingMemoryRequirements.size;
		StagingMemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( StagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		
		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &StagingMemoryAllocateInfo, nullptr, &StagingBufferMemory ) );
		
		VK_CHECK( vkBindBufferMemory( VulkanContext::Get().GetDevice(), StagingBuffer, StagingBufferMemory, 0 ) );
		
		void* pData;
		// Create a region of host data mapped to the device.
		VK_CHECK( vkMapMemory( VulkanContext::Get().GetDevice(), StagingBufferMemory, 0, BufferSize, 0, &pData ) );
		memcpy( pData, m_Vertices.data(), ( size_t )BufferSize );
		vkUnmapMemory( VulkanContext::Get().GetDevice(), StagingBufferMemory );

		//////////////////////////////////////////////////////////////////////////
		
		// Create the vertex buffer.

		VkBufferCreateInfo VertexBufferCreateInfo ={ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		VertexBufferCreateInfo.size = BufferSize;
		VertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK( vkCreateBuffer( VulkanContext::Get().GetDevice(), &VertexBufferCreateInfo, nullptr, &m_Buffer ) );

		// A VkMemoryRequirement is a structure that contains the memory type index and the memory heap index.
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements( VulkanContext::Get().GetDevice(), m_Buffer, &MemoryRequirements );

		VkMemoryAllocateInfo MemoryAllocateInfo ={ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		// Tell Vulkan that we want to allocate from the host visible memory heap. 
		// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is a flag that tells the Vulkan implementation that the host cache is coherent with the device's cache.
		MemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &MemoryAllocateInfo, nullptr, &m_Memory ) );

		VK_CHECK( vkBindBufferMemory( VulkanContext::Get().GetDevice(), m_Buffer, m_Memory, 0 ) );
		
		// Copy buffer
		{
			VkCommandBuffer CommandBuffer;

			{
				VkCommandBufferAllocateInfo BufferAllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
				BufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				BufferAllocInfo.commandPool = VulkanContext::Get().GetCommandPool();
				BufferAllocInfo.commandBufferCount = 1;

				VK_CHECK( vkAllocateCommandBuffers( VulkanContext::Get().GetDevice(), &BufferAllocInfo, &CommandBuffer ) );
				
				VkCommandBufferBeginInfo CommandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
				CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				
				VK_CHECK( vkBeginCommandBuffer( CommandBuffer, &CommandBufferBeginInfo ) );
			}

			VkBufferCopy CopyRegion{};
			CopyRegion.size = BufferSize;
			
			vkCmdCopyBuffer( CommandBuffer, StagingBuffer, m_Buffer, 1, &CopyRegion );

			{
				vkEndCommandBuffer( CommandBuffer );

				VkSubmitInfo SubmitInfo ={ VK_STRUCTURE_TYPE_SUBMIT_INFO };
				SubmitInfo.pCommandBuffers = &CommandBuffer;
				SubmitInfo.commandBufferCount = 1;
				
				VK_CHECK( vkQueueSubmit( VulkanContext::Get().GetGraphicsQueue(), 1, &SubmitInfo, VK_NULL_HANDLE ) );
				VK_CHECK( vkQueueWaitIdle( VulkanContext::Get().GetGraphicsQueue() ) );

				vkFreeCommandBuffers( VulkanContext::Get().GetDevice(), VulkanContext::Get().GetCommandPool(), 1, &CommandBuffer );
			}
			
		}
		
		vkDestroyBuffer( VulkanContext::Get().GetDevice(), StagingBuffer, nullptr );
		vkFreeMemory( VulkanContext::Get().GetDevice(), StagingBufferMemory, nullptr );

	#else

		// Create the buffer.
		VkBufferCreateInfo BufferCreateInfo ={ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = m_Size;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK( vkCreateBuffer( VulkanContext::Get().GetDevice(), &BufferCreateInfo, nullptr, &m_Buffer ) );

		// Create the memory.
		// A VkMemoryRequirement is a structure that contains the memory type index and the memory heap index.
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements( VulkanContext::Get().GetDevice(), m_Buffer, &MemoryRequirements );

		VkMemoryAllocateInfo MemoryAllocateInfo ={ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &MemoryAllocateInfo, nullptr, &m_Memory ) );

		// Bind the buffer to the memory.
		VK_CHECK( vkBindBufferMemory( VulkanContext::Get().GetDevice(), m_Buffer, m_Memory, 0 ) );

		VK_CHECK( vkMapMemory( VulkanContext::Get().GetDevice(), m_Memory, 0, m_Size, 0, &m_pMappedMemory ) );
		memcpy( m_pMappedMemory, m_pData, m_Size );
		vkUnmapMemory( VulkanContext::Get().GetDevice(), m_Memory );

	#endif
	}

	void VertexBuffer::Terminate()
	{
		if( m_Buffer )
			vkDestroyBuffer( VulkanContext::Get().GetDevice(), m_Buffer, nullptr );

		if( m_Memory )
			vkFreeMemory( VulkanContext::Get().GetDevice(), m_Memory, nullptr );

		m_Memory = nullptr;
		m_Buffer = nullptr;

		//free( m_pData );
		m_pData = nullptr;
	}

	//////////////////////////////////////////////////////////////////////////


	IndexBuffer::IndexBuffer( std::vector<uint32_t> Indices ) : m_Indices( Indices )
	{
		m_Size = m_Indices.size();
	}

	IndexBuffer::IndexBuffer( void* pIndicesData, size_t IndicesSize )
	{
		//m_Indices( static_cast<uint32_t>( pIndicesData ) );
		m_Size = IndicesSize;
	}

	IndexBuffer::~IndexBuffer()
	{
		Terminate();
	}

	void IndexBuffer::Bind( VkCommandBuffer CommandBuffer )
	{
		vkCmdBindIndexBuffer( CommandBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT32 );
	}

	void IndexBuffer::Draw( VkCommandBuffer CommandBuffer )
	{
		vkCmdDrawIndexed( CommandBuffer, m_Size, 1, 0, 0, 0 );
	}

	void IndexBuffer::CreateBuffer()
	{
		assert( m_Size >= 3 && "Index count must be above 3!" );

		VkDeviceSize BufferSize = sizeof( m_Indices[ 0 ] ) * m_Size;

		// Create staging buffer.
		VkBuffer StagingBuffer;
		VkDeviceMemory StagingBufferMemory;
		VkMemoryRequirements StagingMemoryRequirements;

		VkBufferCreateInfo BufferCreateInfo ={ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = BufferSize;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK( vkCreateBuffer( VulkanContext::Get().GetDevice(), &BufferCreateInfo, nullptr, &StagingBuffer ) );

		vkGetBufferMemoryRequirements( VulkanContext::Get().GetDevice(), StagingBuffer, &StagingMemoryRequirements );

		VkMemoryAllocateInfo StagingMemoryAllocateInfo ={ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		StagingMemoryAllocateInfo.allocationSize = StagingMemoryRequirements.size;
		StagingMemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( StagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &StagingMemoryAllocateInfo, nullptr, &StagingBufferMemory ) );

		VK_CHECK( vkBindBufferMemory( VulkanContext::Get().GetDevice(), StagingBuffer, StagingBufferMemory, 0 ) );

		void* pData;
		// Create a region of host data mapped to the device.
		VK_CHECK( vkMapMemory( VulkanContext::Get().GetDevice(), StagingBufferMemory, 0, BufferSize, 0, &pData ) );
		memcpy( pData, m_Indices.data(), ( size_t )BufferSize );
		vkUnmapMemory( VulkanContext::Get().GetDevice(), StagingBufferMemory );

		//////////////////////////////////////////////////////////////////////////

		// Create the index buffer.

		VkBufferCreateInfo IndexBufferCreateInfo ={ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		IndexBufferCreateInfo.size = BufferSize;
		IndexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		IndexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK( vkCreateBuffer( VulkanContext::Get().GetDevice(), &IndexBufferCreateInfo, nullptr, &m_Buffer ) );

		// A VkMemoryRequirement is a structure that contains the memory type index and the memory heap index.
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements( VulkanContext::Get().GetDevice(), m_Buffer, &MemoryRequirements );

		VkMemoryAllocateInfo MemoryAllocateInfo ={ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		// Tell Vulkan that we want to allocate from the host visible memory heap. 
		// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is a flag that tells the Vulkan implementation that the host cache is coherent with the device's cache.
		MemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &MemoryAllocateInfo, nullptr, &m_Memory ) );

		VK_CHECK( vkBindBufferMemory( VulkanContext::Get().GetDevice(), m_Buffer, m_Memory, 0 ) );

		// Copy buffer
		{
			VkCommandBuffer CommandBuffer;

			{
				VkCommandBufferAllocateInfo BufferAllocInfo ={ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
				BufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				BufferAllocInfo.commandPool = VulkanContext::Get().GetCommandPool();
				BufferAllocInfo.commandBufferCount = 1;

				VK_CHECK( vkAllocateCommandBuffers( VulkanContext::Get().GetDevice(), &BufferAllocInfo, &CommandBuffer ) );

				VkCommandBufferBeginInfo CommandBufferBeginInfo ={ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
				CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

				VK_CHECK( vkBeginCommandBuffer( CommandBuffer, &CommandBufferBeginInfo ) );
			}

			VkBufferCopy CopyRegion{};
			CopyRegion.size = BufferSize;

			vkCmdCopyBuffer( CommandBuffer, StagingBuffer, m_Buffer, 1, &CopyRegion );

			{
				vkEndCommandBuffer( CommandBuffer );

				VkSubmitInfo SubmitInfo ={ VK_STRUCTURE_TYPE_SUBMIT_INFO };
				SubmitInfo.pCommandBuffers = &CommandBuffer;
				SubmitInfo.commandBufferCount = 1;

				VK_CHECK( vkQueueSubmit( VulkanContext::Get().GetGraphicsQueue(), 1, &SubmitInfo, VK_NULL_HANDLE ) );
				VK_CHECK( vkQueueWaitIdle( VulkanContext::Get().GetGraphicsQueue() ) );

				vkFreeCommandBuffers( VulkanContext::Get().GetDevice(), VulkanContext::Get().GetCommandPool(), 1, &CommandBuffer );
			}

		}

		vkDestroyBuffer( VulkanContext::Get().GetDevice(), StagingBuffer, nullptr );
		vkFreeMemory( VulkanContext::Get().GetDevice(), StagingBufferMemory, nullptr );
	}

	void IndexBuffer::Terminate()
	{
		if( m_Buffer )
			vkDestroyBuffer( VulkanContext::Get().GetDevice(), m_Buffer, nullptr );

		if( m_Memory )
			vkFreeMemory( VulkanContext::Get().GetDevice(), m_Memory, nullptr );

		m_Memory = nullptr;
		m_Buffer = nullptr;

		//free( m_pData );
		m_pData = nullptr;
	}
}