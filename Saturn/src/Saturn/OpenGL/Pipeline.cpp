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
#include "Pipeline.h"

#include "xGL.h"

#if !defined( SAT_DONT_USE_GL ) 

namespace Saturn {

	static GLenum ShaderDataTypeToOpenGLBaseType( ShaderDataType type )
	{
		switch( type )
		{
			case ShaderDataType::Float:    return GL_FLOAT;
			case ShaderDataType::Float2:   return GL_FLOAT;
			case ShaderDataType::Float3:   return GL_FLOAT;
			case ShaderDataType::Float4:   return GL_FLOAT;
			case ShaderDataType::Mat3:     return GL_FLOAT;
			case ShaderDataType::Mat4:     return GL_FLOAT;
			case ShaderDataType::Int:      return GL_INT;
			case ShaderDataType::Int2:     return GL_INT;
			case ShaderDataType::Int3:     return GL_INT;
			case ShaderDataType::Int4:     return GL_INT;
			case ShaderDataType::Bool:     return GL_BOOL;
		}

		SAT_CORE_ASSERT( false, "Unknown ShaderDataType!" );
		return 0;
	}

	Pipeline::Pipeline( const PipelineSpecification& spec ) : m_Specification( spec )
	{
		Invalidate();
	}

	Pipeline::~Pipeline()
	{
		GLuint rendererID = m_VertexArrayRendererID;
		glDeleteVertexArrays( 1, &rendererID );
	}

	void Pipeline::Invalidate( void )
	{
		auto& vertexArrayRendererID = m_VertexArrayRendererID;

		if( vertexArrayRendererID )
			glDeleteVertexArrays( 1, &vertexArrayRendererID );

		glGenVertexArrays( 1, &vertexArrayRendererID );
		glBindVertexArray( vertexArrayRendererID );

		glBindVertexArray( 0 );
	}

	void Pipeline::Bind( void )
	{
		glBindVertexArray( m_VertexArrayRendererID );

		const auto& layout = m_Specification.Layout;
		uint32_t attribIndex = 0;
		for( const auto& element : layout )
		{
			auto glBaseType = ShaderDataTypeToOpenGLBaseType( element.Type );
			glEnableVertexAttribArray( attribIndex );
			if( glBaseType == GL_INT )
			{
				glVertexAttribIPointer( attribIndex,
					element.GetComponentCount(),
					glBaseType,
					layout.GetStride(),
					( const void* )( intptr_t )element.Offset );
			}
			else
			{
				glVertexAttribPointer( attribIndex,
					element.GetComponentCount(),
					glBaseType,
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					( const void* )( intptr_t )element.Offset );
			}
			attribIndex++;
		}
	}

}

#endif