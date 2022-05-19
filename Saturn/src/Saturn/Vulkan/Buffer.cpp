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
#include "Buffer.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

namespace Saturn {

	Buffer::~Buffer()
	{
		Terminate();
	}
	
	//////////////////////////////////////////////////////////////////////////

	void Buffer::CopyBuffer( VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size )
	{
		VkCommandBuffer CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		VkBufferCopy CopyRegion = {};
		CopyRegion.srcOffset = 0;
		CopyRegion.dstOffset = 0;
		CopyRegion.size = Size;

		vkCmdCopyBuffer( CommandBuffer, SrcBuffer, DstBuffer, 1, &CopyRegion );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}

	void Buffer::CopyBuffer( VkBuffer DstBuffer, VkDeviceSize Size )
	{
		VkCommandBuffer CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		VkBufferCopy CopyRegion ={};
		CopyRegion.srcOffset = 0;
		CopyRegion.dstOffset = 0;
		CopyRegion.size = Size;

		vkCmdCopyBuffer( CommandBuffer, m_Buffer, DstBuffer, 1, &CopyRegion );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}

	void Buffer::CopyBuffer( Buffer DstBuffer )
	{
		VkCommandBuffer CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		VkBufferCopy CopyRegion ={};
		CopyRegion.srcOffset = 0;
		CopyRegion.dstOffset = 0;
		CopyRegion.size = DstBuffer.m_Size;

		vkCmdCopyBuffer( CommandBuffer, m_Buffer, DstBuffer, 1, &CopyRegion );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}
	
	//////////////////////////////////////////////////////////////////////////

	void Buffer::Terminate()
	{
		if( m_Buffer )
			vkDestroyBuffer( VulkanContext::Get().GetDevice(), m_Buffer, nullptr );

		if( m_Memory )
			vkFreeMemory( VulkanContext::Get().GetDevice(), m_Memory, nullptr );

		m_Buffer = nullptr;
		m_Memory = nullptr;
	}

	void Buffer::Create( void* pData, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemProperties )
	{
		VkMemoryRequirements MemoryRequirements;

		VkBufferCreateInfo BufferCreateInfo ={ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = Size;
		BufferCreateInfo.usage = Usage;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK( vkCreateBuffer( VulkanContext::Get().GetDevice(), &BufferCreateInfo, nullptr, &m_Buffer ) );
		SetDebugUtilsObjectName( "Internal Buffer", ( uint64_t )m_Buffer, VK_OBJECT_TYPE_BUFFER );

		vkGetBufferMemoryRequirements( VulkanContext::Get().GetDevice(), m_Buffer, &MemoryRequirements );

		VkMemoryAllocateInfo MemoryAllocateInfo ={ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemoryRequirements.memoryTypeBits, MemProperties );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &MemoryAllocateInfo, nullptr, &m_Memory ) );

		VK_CHECK( vkBindBufferMemory( VulkanContext::Get().GetDevice(), m_Buffer, m_Memory, 0 ) );

		if( pData != nullptr )
		{
			void* pMappedMemory = nullptr;
			VK_CHECK( vkMapMemory( VulkanContext::Get().GetDevice(), m_Memory, 0, Size, 0, &pMappedMemory ) );
			memcpy( pMappedMemory, pData, Size );
			vkUnmapMemory( VulkanContext::Get().GetDevice(), m_Memory );
		}
	}

	void Buffer::Create( void* pData, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemProperties, VkDeviceMemory& rMemory )
	{
		VkMemoryRequirements MemoryRequirements;

		VkBufferCreateInfo BufferCreateInfo ={ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = Size;
		BufferCreateInfo.usage = Usage;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK( vkCreateBuffer( VulkanContext::Get().GetDevice(), &BufferCreateInfo, nullptr, &m_Buffer ) );
		SetDebugUtilsObjectName( "Internal Buffer", ( uint64_t )m_Buffer, VK_OBJECT_TYPE_BUFFER );

		vkGetBufferMemoryRequirements( VulkanContext::Get().GetDevice(), m_Buffer, &MemoryRequirements );

		VkMemoryAllocateInfo MemoryAllocateInfo ={ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemoryRequirements.memoryTypeBits, MemProperties );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &MemoryAllocateInfo, nullptr, &rMemory ) );

		VK_CHECK( vkBindBufferMemory( VulkanContext::Get().GetDevice(), m_Buffer, rMemory, 0 ) );

		if( pData != nullptr )
		{
			void* pMappedMemory = nullptr;
			VK_CHECK( vkMapMemory( VulkanContext::Get().GetDevice(), rMemory, 0, Size, 0, &pMappedMemory ) );
			memcpy( pMappedMemory, pData, Size );
			vkUnmapMemory( VulkanContext::Get().GetDevice(), rMemory );
		}

		m_Memory = rMemory;
		m_Size = Size;
	}

	void Buffer::Map( void** ppData, VkDeviceSize Size )
	{
		VK_CHECK( vkMapMemory( VulkanContext::Get().GetDevice(), m_Memory, 0, Size, 0, ppData ) );
	}

	void Buffer::Unmap()
	{
		vkUnmapMemory( VulkanContext::Get().GetDevice(), m_Memory );
	}

}