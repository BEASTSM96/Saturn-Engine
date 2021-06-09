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
#include "OpenGLIndexBuffer.h"

#include <glad/glad.h>

#include "Saturn/Renderer/Renderer.h"

namespace Saturn {

	OpenGLIndexBuffer::OpenGLIndexBuffer( void* data, uint32_t size )
		: m_RendererID( 0 ), m_Size( size )
	{
		m_LocalData = Buffer::Copy( data, size );

		Ref<OpenGLIndexBuffer> instance = this;
		Renderer::Submit( [instance]() mutable
 {
	 glCreateBuffers( 1, &instance->m_RendererID );
	 glNamedBufferData( instance->m_RendererID, instance->m_Size, instance->m_LocalData.Data, GL_STATIC_DRAW );
		} );
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer( uint32_t size )
		: m_Size( size )
	{
		// m_LocalData = Buffer(size);

		Ref<OpenGLIndexBuffer> instance = this;
		Renderer::Submit( [instance]() mutable
 {
	 glCreateBuffers( 1, &instance->m_RendererID );
	 glNamedBufferData( instance->m_RendererID, instance->m_Size, nullptr, GL_DYNAMIC_DRAW );
		} );
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		GLuint rendererID = m_RendererID;
		Renderer::Submit( [rendererID]()
 {
	 glDeleteBuffers( 1, &rendererID );
		} );
	}

	void OpenGLIndexBuffer::SetData( void* data, uint32_t size, uint32_t offset )
	{
		m_LocalData = Buffer::Copy( data, size );
		m_Size = size;
		Ref<OpenGLIndexBuffer> instance = this;
		Renderer::Submit( [instance, offset]()
 {
	 glNamedBufferSubData( instance->m_RendererID, offset, instance->m_Size, instance->m_LocalData.Data );
		} );
	}

	void OpenGLIndexBuffer::Bind() const
	{
		Ref<const OpenGLIndexBuffer> instance = this;
		Renderer::Submit( [instance]()
 {
	 glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, instance->m_RendererID );
		} );
	}

}