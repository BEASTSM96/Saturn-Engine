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
#include "OpenGLPipeline.h"

#include "Saturn/Renderer/Renderer.h"

#include <glad/glad.h>

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

	OpenGLPipeline::OpenGLPipeline( const PipelineSpecification& spec )
		: m_Specification( spec )
	{
		Invalidate();
	}


	OpenGLPipeline::~OpenGLPipeline()
	{
		GLuint rendererID = m_VertexArrayRendererID;
		Renderer::Submit( [rendererID]()
			{
				glDeleteVertexArrays( 1, &rendererID );
			} );
	}


	void OpenGLPipeline::Invalidate()
	{
		SAT_CORE_ASSERT( m_Specification.Layout.GetElements().size(), "Layout is empty!" );

		Ref<OpenGLPipeline> instance = this;
		Renderer::Submit( [instance]() mutable
			{
				auto& vertexArrayRendererID = instance->m_VertexArrayRendererID;

				if( vertexArrayRendererID )
					glDeleteVertexArrays( 1, &vertexArrayRendererID );

				glGenVertexArrays( 1, &vertexArrayRendererID );
				glBindVertexArray( vertexArrayRendererID );

				glBindVertexArray( 0 );
			} );
	}

	void OpenGLPipeline::Bind()
	{
		Ref<OpenGLPipeline> instance = this;
		Renderer::Submit( [instance]()
			{
				glBindVertexArray( instance->m_VertexArrayRendererID );

				const auto& layout = instance->m_Specification.Layout;
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
			} );
	}
}