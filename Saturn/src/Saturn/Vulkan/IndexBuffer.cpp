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
#include "IndexBuffer.h"

#include "VulkanContext.h"

#include "VulkanDebug.h"

#include <cassert>

namespace Saturn {

	IndexBuffer::IndexBuffer( void* pData, size_t Size )
	{
		m_pData = pData;
		m_Buffer.m_Size = Size;

		CreateBuffer();
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
		vkCmdDrawIndexed( CommandBuffer, m_Buffer.m_Size, 1, 0, 0, 0 );
	}

	void IndexBuffer::CreateBuffer()
	{
		assert( m_Buffer.m_Size >= 3 && "Index count must be above 3!" );

		VkDeviceSize BufferSize = m_Buffer.m_Size * sizeof( uint32_t );

		// Create staging buffer.
		Buffer StagingBuffer;
		
		StagingBuffer.Create( m_pData, BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		
		//////////////////////////////////////////////////////////////////////////

		// Create the index buffer.

		m_Buffer.Create( nullptr, BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

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
	}

	void IndexBuffer::Terminate()
	{
		m_Buffer.Terminate();
	}

}