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


#pragma once

#include "Saturn/Renderer/RendererAPI.h"
#include "Saturn/Renderer/Texture.h"

namespace Saturn {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D( TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap );
		OpenGLTexture2D( const std::string& path, bool srgb );
		virtual ~OpenGLTexture2D();

		virtual void Bind( uint32_t slot = 0 ) const;

		virtual TextureFormat GetFormat() const override { return m_Format; }
		virtual uint32_t GetWidth( void )  const override { return m_Width; }
		virtual uint32_t GetHeight( void )  const override { return m_Height; }
		// This function currently returns the expected number of mips based on image size,
		// not present mips in data
		virtual uint32_t GetMipLevelCount() const override;

		virtual void Lock() override;
		virtual void Unlock() override;

		virtual void Resize( uint32_t width, uint32_t height ) override;
		virtual Buffer GetWriteableBuffer() override;

		virtual const std::string& GetPath() const override { return m_FilePath; }

		virtual bool Loaded() const override { return m_Loaded; }

		virtual RendererID GetRendererID() const override { return m_RendererID; }

		virtual bool operator==( const Texture& other ) const override
		{
			return m_RendererID == ( ( OpenGLTexture2D& )other ).m_RendererID;
		}
	private:
		RendererID m_RendererID;
		TextureFormat m_Format;
		TextureWrap m_Wrap = TextureWrap::Clamp;
		uint32_t m_Width, m_Height;

		Buffer m_ImageData;
		bool m_IsHDR = false;

		bool m_Locked = false;
		bool m_Loaded = false;

		std::string m_FilePath;
	};

	class OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube( TextureFormat format, uint32_t width, uint32_t height );
		OpenGLTextureCube( const std::string& path );
		virtual ~OpenGLTextureCube();

		virtual void Bind( uint32_t slot = 0 ) const;

		virtual TextureFormat GetFormat() const { return m_Format; }
		virtual uint32_t GetWidth( void )  const { return m_Width; }
		virtual uint32_t GetHeight( void )  const { return m_Height; }
		// This function currently returns the expected number of mips based on image size,
		// not present mips in data
		virtual uint32_t GetMipLevelCount() const override;

		virtual const std::string& GetPath() const override { return m_FilePath; }

		virtual RendererID GetRendererID() const override { return m_RendererID; }

		virtual bool operator==( const Texture& other ) const override
		{
			return m_RendererID == ( ( OpenGLTextureCube& )other ).m_RendererID;
		}
	private:
		RendererID m_RendererID;
		TextureFormat m_Format;
		uint32_t m_Width, m_Height;

		unsigned char* m_ImageData;

		std::string m_FilePath;
	};

}


