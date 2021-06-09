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

#include "sppch.h"
#include "OpenGLVertexBuffer.h"

#include <glad/glad.h>

#include "Saturn/Renderer/Renderer.h"

namespace Saturn {

	static GLenum OpenGLUsage( VertexBufferUsage usage )
	{
		switch( usage )
		{
			case VertexBufferUsage::Static:    return GL_STATIC_DRAW;
			case VertexBufferUsage::Dynamic:   return GL_DYNAMIC_DRAW;
		}
		SAT_CORE_ASSERT( false, "Unknown vertex buffer usage" );
		return 0;
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer( void* data, uint32_t size, VertexBufferUsage usage )
		: m_Size( size ), m_Usage( usage )
	{
		m_LocalData = Buffer::Copy( data, size );

		Ref<OpenGLVertexBuffer> instance = this;
		Renderer::Submit( [instance]() mutable
		{
			glCreateBuffers( 1, &instance->m_RendererID );
			glNamedBufferData( instance->m_RendererID, instance->m_Size, instance->m_LocalData.Data, OpenGLUsage( instance->m_Usage ) );
		} );
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer( uint32_t size, VertexBufferUsage usage )
		: m_Size( size ), m_Usage( usage )
	{
		Ref<OpenGLVertexBuffer> instance = this;
		Renderer::Submit( [instance]() mutable
		{
			glCreateBuffers( 1, &instance->m_RendererID );
			glNamedBufferData( instance->m_RendererID, instance->m_Size, nullptr, OpenGLUsage( instance->m_Usage ) );
		} );
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		GLuint rendererID = m_RendererID;
		Renderer::Submit( [rendererID]()
 {
	 glDeleteBuffers( 1, &rendererID );
		} );
	}

	void OpenGLVertexBuffer::SetData( void* data, uint32_t size, uint32_t offset )
	{
		m_LocalData = Buffer::Copy( data, size );
		m_Size = size;
		Ref<OpenGLVertexBuffer> instance = this;
		Renderer::Submit( [instance, offset]()
 {
	 glNamedBufferSubData( instance->m_RendererID, offset, instance->m_Size, instance->m_LocalData.Data );
		} );
	}

	void OpenGLVertexBuffer::Bind() const
	{
		Ref<const OpenGLVertexBuffer> instance = this;
		Renderer::Submit( [instance]()
 {
	 glBindBuffer( GL_ARRAY_BUFFER, instance->m_RendererID );
		} );
	}
}