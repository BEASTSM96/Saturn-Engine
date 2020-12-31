/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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

#include "Saturn/Renderer/VertexBuffer.h"

#include "Saturn/Core/Buffer.h"

namespace Saturn {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer( void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static );
		OpenGLVertexBuffer( uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic );
		virtual ~OpenGLVertexBuffer();

		virtual void SetData( void* data, uint32_t size, uint32_t offset = 0 );
		virtual void Bind( void ) const;

		virtual const VertexBufferLayout& GetLayout( void ) const override { return m_Layout; }
		virtual void SetLayout( const VertexBufferLayout& layout ) override { m_Layout = layout; }

		virtual uint32_t GetSize( void ) const { return m_Size; }
		virtual RendererID GetRendererID( void ) const { return m_RendererID; }
	private:
		RendererID m_RendererID = 0;
		uint32_t m_Size;
		VertexBufferUsage m_Usage;
		VertexBufferLayout m_Layout;

		Buffer m_LocalData;
	};
}