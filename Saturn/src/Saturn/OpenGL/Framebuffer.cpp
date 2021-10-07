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
#include "Framebuffer.h"

#include <glad/glad.h>

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
		for ( auto format : m_Specification.Attachments.Attachments )
		{
			if( FramebufferUtills::IsDepthFormat( format.TextureFormat ) )
				m_DepthAttachmentFormat = format.TextureFormat;
			else
				m_ColorAttachmentsFormat.emplace_back( format.TextureFormat );
		}

		Resize( spec.Width, spec.Height, true );
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers( 1, &m_RendererID );
	}

	void Framebuffer::Resize( uint32_t width, uint32_t height, bool forceRecreate /*= false */ )
	{
		if( !forceRecreate && ( m_Specification.Width == width && m_Specification.Height == height ) )
			return;

		m_Specification.Width = width;
		m_Specification.Height = height;

		m_Width = width;
		m_Height = height;

		if( m_RendererID )
		{
			glDeleteFramebuffers( 1, &m_RendererID );
			glDeleteTextures( m_ColorAttachments.size(), m_ColorAttachments.data() );
			glDeleteTextures( 1, &m_DepthAttachment );

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glGenFramebuffers( 1, &m_RendererID );
		glBindFramebuffer( GL_FRAMEBUFFER, m_RendererID );

		bool multisample = m_Specification.Samples > 1;

		if( m_ColorAttachmentsFormat.size() )
		{
			m_ColorAttachments.resize( m_ColorAttachmentsFormat.size() );
			FramebufferUtills::CreateTextures( multisample, m_ColorAttachments.data(), m_ColorAttachments.size() );

			for( int i = 0; i < m_ColorAttachmentsFormat.size(); i++ )
			{
				FramebufferUtills::BindTexture( multisample, m_ColorAttachments[ i ] );
				switch( m_ColorAttachmentsFormat[ i ] )
				{
					case FramebufferTextureFormat::RGBA8:
						FramebufferUtills::AttachColorTexture( m_ColorAttachments[ i ], m_Specification.Samples, GL_RGBA8, m_Width, m_Height, i );
						break;
					case FramebufferTextureFormat::RGBA16F:
						FramebufferUtills::AttachColorTexture( m_ColorAttachments[ i ], m_Specification.Samples, GL_RGBA16F, m_Width, m_Height, i );
						break;
					case FramebufferTextureFormat::RGBA32F:
						FramebufferUtills::AttachColorTexture( m_ColorAttachments[ i ], m_Specification.Samples, GL_RGBA32F, m_Width, m_Height, i );
						break;
					case FramebufferTextureFormat::RGB32F:
						FramebufferUtills::AttachColorTexture( m_ColorAttachments[ i ], m_Specification.Samples, GL_RGB32F, m_Width, m_Height, i );
						break;
				}
			}
		}

		if( m_DepthAttachmentFormat != FramebufferTextureFormat::None )
		{
			FramebufferUtills::CreateTextures( multisample, &m_DepthAttachment, 1 );
			FramebufferUtills::BindTexture( multisample, m_DepthAttachment );
			switch( m_DepthAttachmentFormat )
			{
				case FramebufferTextureFormat::DEPTH32F:
					FramebufferUtills::AttachDepthTexture( m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_ATTACHMENT, m_Width, m_Height );
					break;
				case FramebufferTextureFormat::DEPTH24STENCIL8:
					FramebufferUtills::AttachDepthTexture( m_DepthAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Width, m_Height );
					break;
			}
		}

		if( m_ColorAttachments.size() > 1 )
		{
			SAT_CORE_ASSERT( m_ColorAttachments.size() <= 4 );
			GLenum buffers[ 4 ] ={ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers( m_ColorAttachments.size(), buffers );
		}
		else if( m_ColorAttachments.size() == 0 )
		{
			glDrawBuffer( GL_NONE );
		}

		SAT_CORE_ASSERT( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!" );

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
		glBindTextureUnit( slot, m_ColorAttachments[ index ] );
	}
}