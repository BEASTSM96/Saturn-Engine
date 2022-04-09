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
#include "VertexBuffer.h"
#include "VulkanContext.h"

#include "VulkanDebug.h"

#include <cassert>

namespace Saturn {

	VertexBuffer::VertexBuffer( const std::vector< Vertex >& Vertices )
	{
		m_Vertices = Vertices;
		m_Buffer.m_Size = m_Vertices.size();
		m_pData = m_Vertices.data();
	}

	VertexBuffer::~VertexBuffer()
	{
		Terminate();
	}

	void VertexBuffer::BindAndDraw( VkCommandBuffer CommandBuffer )
	{
		Bind( CommandBuffer );

		vkCmdDraw( CommandBuffer, ( size_t )m_Buffer.m_Size, 1, 0, 0 );
	}

	void VertexBuffer::Bind( VkCommandBuffer CommandBuffer )
	{
		VkBuffer Buffers[] ={ m_Buffer };
		VkDeviceSize Offsets[] ={ 0 };

		vkCmdBindVertexBuffers( CommandBuffer, 0, 1, Buffers, Offsets );
	}

	void VertexBuffer::Draw( VkCommandBuffer CommandBuffer )
	{
		vkCmdDraw( CommandBuffer, ( size_t )m_Buffer.m_Size, 1, 0, 0 );
	}

	std::vector<VkVertexInputBindingDescription> VertexBuffer::GetBindingDescriptions()
	{
		return { { .binding = 0, .stride = sizeof( Vertex ), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX } };
	}

	std::vector<VkVertexInputAttributeDescription> VertexBuffer::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> AttributeDescriptions;
		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Normal ) } );
		AttributeDescriptions.push_back( { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Tangent ) } );
		AttributeDescriptions.push_back( { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Binormal ) } );
		AttributeDescriptions.push_back( { 4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( Vertex, Texcoord ) } );
		
		return AttributeDescriptions;
	}

	void VertexBuffer::CreateBuffer()
	{
		assert( m_Buffer.m_Size >= 3 && "Vertex count must be above 3!" );

		VkDeviceSize BufferSize = sizeof( m_Vertices[ 0 ] ) * m_Buffer.m_Size;

		Buffer StagingBuffer;
		StagingBuffer.Create( m_Vertices.data(), BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

		//////////////////////////////////////////////////////////////////////////
		
		// Create the vertex buffer.

		m_Buffer.Create( nullptr, BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		
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

	}

	void VertexBuffer::Terminate()
	{
		m_pData = nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
}