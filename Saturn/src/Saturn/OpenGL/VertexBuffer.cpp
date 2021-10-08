#include "sppch.h"
#include "VertexBuffer.h"

#include <glad/glad.h>

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

	VertexBuffer::VertexBuffer( void* data, uint32_t size, VertexBufferUsage type ) : m_Size( size ), m_Usage( type )
	{
		m_Data = data;

		glCreateBuffers( 1, &m_RendererID );
		glNamedBufferData( m_RendererID, m_Size, m_Data, OpenGLUsage( m_Usage ) );
	}

	VertexBuffer::VertexBuffer( uint32_t size, VertexBufferUsage type /*= VertexBufferUsage::Dynamic */ ) : m_Size( size ), m_Usage( type )
	{
		m_Data = nullptr;

		glCreateBuffers( 1, &m_RendererID );
		glNamedBufferData( m_RendererID, m_Size, nullptr, OpenGLUsage( m_Usage ) );
	}

	VertexBuffer::~VertexBuffer()
	{
		GLuint rendererID = m_RendererID;
		glDeleteBuffers( 1, &rendererID );
	}

	void VertexBuffer::Bind()
	{
		glBindBuffer( GL_ARRAY_BUFFER, m_RendererID );
	}

	void VertexBuffer::OverrideData( void* data, uint32_t size, uint32_t offset /*= 0 */ )
	{
		m_Data = data;
		m_Size = size;

		glNamedBufferSubData( m_RendererID, offset, m_Size, m_Data );
	}
}