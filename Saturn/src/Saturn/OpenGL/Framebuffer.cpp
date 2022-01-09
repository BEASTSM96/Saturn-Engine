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
#include "Framebuffer.h"

#include "xGL.h"

#if !defined( SAT_DONT_USE_GL ) 

namespace Saturn::FramebufferUtills {

	static GLenum TextureTarget( bool sample )
	{
		return sample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	}

	static void CreateTextures( bool sample, RendererID* IDOut, uint32_t count )
	{
		glCreateTextures( TextureTarget( sample ), 1, IDOut );
	}

	static void BindTexture( bool sample, RendererID ID )
	{
		glBindTexture( TextureTarget( sample ), ID );
	}

	static GLenum DataType( GLenum format )
	{
		switch( format )
		{
			case GL_RGBA8: return GL_UNSIGNED_BYTE;
			case GL_RGB16F:
			case GL_RGB32F:
			case GL_RGBA16F:
			case GL_RGBA32F: return GL_FLOAT;
			case GL_DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
		}

		SAT_CORE_ASSERT( false, "Unknown DataType format!" );
		return 0;
	}

	static bool IsDepthFormat( FramebufferTextureFormat format )
	{
		switch( format )
		{
			case FramebufferTextureFormat::DEPTH32F:
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				return true;
		}
		return false;
	}

	static void AttachColorTexture( RendererID id, int sampes, GLenum format, uint32_t width, uint32_t height, int index )
	{
		bool sampled = sampes > 1;
		if( sampled )
		{
			glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, sampes, format, width, height, GL_FALSE );
		}
		else
		{
			glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, DataType( format ), nullptr );

			glTexParameteri( TextureTarget( sampled ), GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( TextureTarget( sampled ), GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( TextureTarget( sampled ), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( TextureTarget( sampled ), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}

		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget( sampled ), id, 0 );

	}

	static void AttachDepthTexture( RendererID id, int sampes, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height )
	{
		bool sampled = sampes > 1;
		if( sampled )
		{
			glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, sampes, format, width, height, GL_FALSE );
		}
		else
		{
			glTexStorage2D( GL_TEXTURE_2D, 1, format, width, height );

			glTexParameteri( TextureTarget( sampled ), GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( TextureTarget( sampled ), GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( TextureTarget( sampled ), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( TextureTarget( sampled ), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}

		glFramebufferTexture2D( GL_FRAMEBUFFER, attachmentType, TextureTarget( sampled ), id, 0 );

	}
}

namespace Saturn {

	Framebuffer::Framebuffer( const FramebufferSpecification& spec ) : m_Specification( spec )
	{
		Resize( spec.Width, spec.Height, true );
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers( 1, &m_RendererID );
		glDeleteTextures( 1, &m_TextureID );
		glDeleteTextures( 1, &m_DepthTextureID );
	}

	void Framebuffer::Resize( uint32_t width, uint32_t height, bool forceRecreate /*= false */ )
	{
		if( !forceRecreate && ( m_Specification.Width == width && m_Specification.Height == height ) )
			return;

		if( width == 0 && height == 0 ) 
		{
			SAT_CORE_WARN( "[Framebuffer] Width and Height are 0 no resize." );
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		m_Width = width;
		m_Height = height;

		if( m_RendererID )
		{
			glDeleteFramebuffers( 1, &m_RendererID );
			glDeleteTextures( 1, &m_TextureID );
			glDeleteTextures( 1, &m_DepthTextureID );
		}

		glCreateFramebuffers( 1, &m_RendererID );
		glBindFramebuffer( GL_FRAMEBUFFER, m_RendererID );

		if( m_Specification.Samples == 8 )
		{
			glCreateTextures( GL_TEXTURE_2D_MULTISAMPLE, 1, &m_TextureID );
			glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, m_TextureID );

			glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, m_Specification.Samples, GL_RGBA8, m_Width, m_Height, GL_TRUE );

			glTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_TextureID, 0 );
		}
		else
		{
			glCreateTextures( GL_TEXTURE_2D, 1, &m_TextureID );
			glBindTexture( GL_TEXTURE_2D, m_TextureID );

			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0 );
		}

		glCreateTextures( GL_TEXTURE_2D, 1, &m_DepthTextureID );
		glBindTexture( GL_TEXTURE_2D, m_DepthTextureID );

		glTexStorage2D( GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Width, m_Height );

		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthTextureID, 0 );

		SAT_CORE_ASSERT( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE, "The Framebuffer is incomplete" );

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	void Framebuffer::Bind()
	{
		glBindFramebuffer( GL_FRAMEBUFFER, m_RendererID );
		glViewport( 0, 0, m_Specification.Width, m_Specification.Height );
	}

	void Framebuffer::Unbind()
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	void Framebuffer::BindTexture( uint32_t index, uint32_t slot )
	{
		glBindTextureUnit( slot, m_TextureID );
	}

	RendererID Framebuffer::ColorAttachmentRendererID( int index /*= 0 */ )
	{
		return m_TextureID;
	}

	RendererID Framebuffer::DepthAttachmentRendererID( void ) const
	{
		return m_DepthTextureID;
	}

}

#endif