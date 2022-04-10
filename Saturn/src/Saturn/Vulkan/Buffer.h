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

#include <vulkan.h>

namespace Saturn {

	class Buffer
	{
	public:
		 Buffer() {}
		~Buffer();
		
		static void CopyBuffer( VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size );
		void CopyBuffer( VkBuffer DstBuffer, VkDeviceSize Size );
		void CopyBuffer( Buffer DstBuffer );

		void Create( void* pData, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemProperties );
		void Create( void* pData, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemProperties, VkDeviceMemory& rMemory );
		
		void Terminate();

		void Map( void** ppData, VkDeviceSize Size );
		void Unmap();

		VkBuffer& GetBuffer() { return m_Buffer; }

		operator VkBuffer() const { return m_Buffer; }
		operator VkBuffer&() { return m_Buffer; }

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
		VkDeviceSize m_Size = 0;

	private:

		friend class VertexBuffer;
		friend class IndexBuffer;
	};
}