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
#include "IndexBuffer.h"

#include "xGL.h"

#if !defined( SAT_DONT_USE_GL ) 

namespace Saturn {

	IndexBuffer::IndexBuffer( void* data, uint32_t size, uint32_t offset /* = 0 */ ) : m_Size( size )
	{
		m_Size = size;
		m_Data = data;

		glCreateBuffers( 1, &m_RendererID );
		glNamedBufferData( m_RendererID, m_Size, m_Data, GL_STATIC_DRAW );
	}

	IndexBuffer::~IndexBuffer()
	{
		GLuint rendererID = m_RendererID;
		glDeleteBuffers( 1, &rendererID );
	}

	void IndexBuffer::OverrideData( void* dr, uint32_t size, uint32_t offset )
	{
		m_Data = dr;
		m_Size = size;
		glNamedBufferSubData( m_RendererID, offset, m_Size, m_Data );
	}

	void IndexBuffer::Bind()
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_RendererID );
	}

}

#endif