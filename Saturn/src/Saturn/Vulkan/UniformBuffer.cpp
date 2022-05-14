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
#include "UniformBuffer.h"

#include "VulkanContext.h"

namespace Saturn {

	UniformBuffer::UniformBuffer( void* pData, size_t Size )
	{
		m_pData = pData;
		m_Buffer.m_Size = Size;

		CreateBuffer( Size );
	}

	UniformBuffer::~UniformBuffer()
	{
		m_Buffer.Terminate();
	}

	void UniformBuffer::Bind( VkCommandBuffer CommandBuffer, bool RecreateBuffer /*= false */ )
	{
		void* pData;

		m_Buffer.Map( &pData, m_Buffer.m_Size );

		memcpy( pData, m_pData, m_Buffer.m_Size );
		
		m_Buffer.Unmap();
	}

	void UniformBuffer::Update( void* pData, size_t Size )
	{
		m_Buffer.Terminate();

		m_pData = pData;
		m_Buffer.m_Size = Size;

		CreateBuffer( Size );
	}

	void UniformBuffer::CreateBuffer( size_t Size )
	{
		VkDeviceSize BufferSize = Size;

		m_Buffer.Create( m_pData, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	}

}