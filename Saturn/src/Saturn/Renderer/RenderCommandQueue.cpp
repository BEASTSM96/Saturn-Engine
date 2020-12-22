/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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
#include "RenderCommandQueue.h"

namespace Saturn {

	//TODO: just redo

	RenderCommandQueue::RenderCommandQueue()
	{
		m_CommandBuffer = new uint8_t[ 10 * 1024 * 1024 ];
		m_CommandBufferPtr = m_CommandBuffer;
		memset( m_CommandBuffer, 0, 10 * 1024 * 1024 );
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] m_CommandBuffer;
	}

	void* RenderCommandQueue::Allocate( RenderCommandFn fn, uint32_t size )
	{
		*( RenderCommandFn* )m_CommandBufferPtr = fn;
		m_CommandBufferPtr += sizeof( RenderCommandFn );

		*( uint32_t* )m_CommandBufferPtr = size;
		m_CommandBufferPtr += sizeof( uint32_t );

		void* memory = m_CommandBufferPtr;
		m_CommandBufferPtr += size;

		m_CommandCount++;
		return memory;
	}

	void RenderCommandQueue::Execute()
	{
		uint8_t* buffer = m_CommandBuffer;

		for( uint32_t i = 0; i < m_CommandCount; i++ )
		{
			RenderCommandFn function = *( RenderCommandFn* )buffer;
			buffer += sizeof( RenderCommandFn );

			uint32_t size = *( uint32_t* )buffer;
			buffer += sizeof( uint32_t );
			function( buffer );
			buffer += size;
		}

		m_CommandBufferPtr = m_CommandBuffer;
		m_CommandCount = 0;
	}
}