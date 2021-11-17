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
#include "Texture.h"

#include "xGL.h"

namespace Saturn {

	// Sat -> OpenGL
	static GLenum SaturnToOpenGLTextureFormat( TextureFormat format )
	{
		switch( format )
		{
			case Saturn::TextureFormat::RGB:     return GL_SRGB;
			case Saturn::TextureFormat::RGBA:    return GL_SRGB8_ALPHA8;
			case Saturn::TextureFormat::Float16: return GL_RGBA16F;
		}
		SAT_CORE_ASSERT( false, "Unknown texture format!" );
		return 0;
	}

	// OpenGL -> Sat
	static TextureFormat OpenGLToTextureFormat( GLint format )
	{
		switch( format )
		{
			case GL_RGB:     return Saturn::TextureFormat::RGB;
			case GL_RGBA:    return Saturn::TextureFormat::RGBA;
			case GL_RGBA16F: return Saturn::TextureFormat::Float16;
		}
		SAT_CORE_ASSERT( false, "Unknown texture format!" );
		return TextureFormat::None;
	}

	// Sat -> OpenGL
	static GLenum SaturnToOpenGLTextureWrap( TextureWrap format )
	{
		switch( format )
		{
			case Saturn::TextureWrap::Clamp:    return GL_REPEAT;
			case Saturn::TextureWrap::Repeat:   return GL_CLAMP_TO_EDGE;
		}
		SAT_CORE_ASSERT( false, "Unknown texture wrap!" );
		return 0;
	}

	// OpenGL -> Sat
	static TextureWrap OpenGLToTextureWrap( GLint format )
	{
		switch( format )
		{
			case GL_REPEAT:           return Saturn::TextureWrap::Repeat;
			case GL_CLAMP_TO_EDGE:    return Saturn::TextureWrap::Clamp;
		}
		SAT_CORE_ASSERT( false, "Unknown texture wrap!" );
		return TextureWrap::None;
	}

	uint32_t Texture::BPP( TextureFormat format )
	{
		switch( format )
		{
			case TextureFormat::RGB:    return 3;
			case TextureFormat::RGBA:   return 4;
		}
		return 0;
	}

	uint32_t Texture::MipMapCount( uint32_t width, uint32_t height )
	{
		uint32_t levels = 1;
		while( ( width | height ) >> levels )
			levels++;

		return levels;
	}

	Texture2D::Texture2D( TextureFormat format, uint32_t w, uint32_t h, TextureWrap wrap ) : m_Width( w ), m_Height( h )
	{
		m_FileName = "Created by width and height.";

		m_Format = format;

		GLenum internalFormat = GL_RGBA8;
		GLenum fformat        = GL_RGBA;
		GLenum type           = 0;

		glCreateTextures( GL_TEXTURE_2D, 1, &m_RendererID );
		glTextureStorage2D( m_RendererID, 1, internalFormat, m_Width, m_Height );

		// Set texture filtering parameters

		glTextureParameteri( m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTextureParameteri( m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		// Set texture wrapping parameters

		glTextureParameteri( m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTextureParameteri( m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT );
		//glTextureParameteri( m_RendererID, GL_TEXTURE_WRAP_R, GL_REPEAT );

		//glTextureSubImage2D( m_RendererID, 0, internalFormat, m_Width, m_Height, 0, format, type, m_ImageData );

		glTextureSubImage2D( m_RendererID, 0, 0, 0, m_Width, m_Height, fformat, GL_UNSIGNED_BYTE, m_ImageData );
	}

	Texture2D::Texture2D( const std::string& path, bool srgb, bool specMap )
	{
		size_t found = path.find_last_of( "/\\" );
		m_FileName = found != std::string::npos ? path.substr( found + 1 ) : path;

		found = m_FileName.find_last_of( "." );
		m_FileName = found != std::string::npos ? m_FileName.substr( 0, found ) : m_FileName;

		stbi_set_flip_vertically_on_load( true );

		int w, h, chan = 0;

		// Load image into STBI

		if( !stbi_is_hdr( path.c_str() ) )
		{
			SAT_CORE_INFO( "Loading texture {0}, srgb={1}", path, srgb );

			m_ImageData = stbi_load( path.c_str(), &w, &h, &chan, 0 );
			if( !m_ImageData )
			{
				SAT_CORE_ERROR( "Texture at {0} was not found!", path );
				return;
			}

			m_Format = TextureFormat::RGBA;
		}
		else
			SAT_CORE_ASSERT( false, "HDR Textures are not supported!" );

		if( !m_ImageData )
			return;

		m_Loaded = true;

		m_Width = w;
		m_Height = h;

		// Get OpenGL to make the texture 
		if( srgb )
		{
			glCreateTextures( GL_TEXTURE_2D, 1, &m_RendererID );
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_2D, m_RendererID );

			int levels = Texture::MipMapCount( m_Width, m_Height );
			glTextureStorage2D( m_RendererID, levels, GL_SRGB8_ALPHA8, m_Width, m_Height );
			glTextureParameteri( m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR );
			glTextureParameteri( m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glTextureSubImage2D( m_RendererID, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, m_ImageData );

			glGenerateTextureMipmap( m_RendererID );

			glBindTexture( GL_TEXTURE_2D, 0 );
		}
		else
		{
			// Update 11.7.21 - Use new OpenGL API
			
			GLenum internalFormat = 0;
			GLenum type           = 0;
			GLenum format         = 0;

			if ( chan == 4 )
			{
				internalFormat = GL_RGBA8;
				format = GL_RGBA;
				m_Format = TextureFormat::RGBA;
			}
			else if( chan == 3 ) 
			{
				internalFormat = GL_RGB8;
				format = GL_RGB;
				m_Format = TextureFormat::RGB;
			}
			else if( chan == 1 ) 
			{
				// Yes, i know that a black and white texture prob does not have alpha or green and blue but we'll use this for now.
				internalFormat = GL_RGBA8;
				format = GL_RED;
				m_Format = TextureFormat::RED;
			}

			glCreateTextures( GL_TEXTURE_2D, 1, &m_RendererID );
			glTextureStorage2D( m_RendererID, 1, internalFormat, m_Width, m_Height );
			
			// Set texture filtering parameters

			glTextureParameteri( m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTextureParameteri( m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			// Set texture wrapping parameters

			glTextureParameteri( m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTextureParameteri( m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT );
			//glTextureParameteri( m_RendererID, GL_TEXTURE_WRAP_R, GL_REPEAT );

			//glTextureSubImage2D( m_RendererID, 0, internalFormat, m_Width, m_Height, 0, format, type, m_ImageData );

			glTextureSubImage2D( m_RendererID, 0, 0, 0, m_Width, m_Height, format, GL_UNSIGNED_BYTE, m_ImageData );

			glGenerateMipmap( GL_TEXTURE_2D );
		}
		stbi_image_free( m_ImageData );

	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures( 1, &m_RendererID );
	}

	void Texture2D::Bind( uint32_t slot /*= 0 */ ) const
	{
		glBindTextureUnit( slot, m_RendererID );
	}

	uint32_t Texture2D::MipLevelCount() const
	{
		return 0;
	}

	void Texture2D::Lock()
	{
		m_Locked = true;
	}

	void Texture2D::Unlock()
	{
		m_Locked = false;
	}

	void Texture2D::Resize( uint32_t width, uint32_t height )
	{

	}

}