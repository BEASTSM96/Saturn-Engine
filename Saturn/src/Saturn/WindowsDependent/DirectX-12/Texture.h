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

#pragma once

#include "Common.h"

#include <stb_image.h>
#include <stdint.h>

namespace Saturn {

	enum class TextureFormat
	{
		None = 0,
		RGB = 1,
		RGBA = 2,
		Float16 = 3,
		RED = 4
	};

	// Not use in DX?
	enum class TextureWrap
	{
		None = 0,
		Clamp = 1,
		Repeat = 2
	};

	// Sat -> DX
	static DXGI_FORMAT SaturnToDXTextureFormat( TextureFormat format )
	{
		switch( format )
		{
			case Saturn::TextureFormat::RGB:     return DXGI_FORMAT_R8G8B8A8_UNORM;
			case Saturn::TextureFormat::RGBA:    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case Saturn::TextureFormat::Float16: return DXGI_FORMAT_UNKNOWN;
		}
		SAT_CORE_ASSERT( false, "Unknown texture format!" );
		return DXGI_FORMAT_UNKNOWN;
	}

	// DX -> Sat
	static TextureFormat DXToTextureFormat( DXGI_FORMAT format )
	{
		switch( format )
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM:      return Saturn::TextureFormat::RGB;
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return Saturn::TextureFormat::RGBA;
			case DXGI_FORMAT_UNKNOWN:             return TextureFormat::None;
		}
		SAT_CORE_ASSERT( false, "Unknown texture format!" );
		return TextureFormat::None;
	}

	/*
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
	*/

	class Texture
	{
	public:

		virtual ~Texture() { }

		virtual TextureFormat Format() const = 0;

		virtual uint32_t Width( void ) const = 0;
		virtual uint32_t Height( void ) const = 0;
		virtual uint32_t MipLevelCount( void ) const = 0;

		static uint32_t BPP( TextureFormat format );
		static uint32_t MipMapCount( uint32_t width, uint32_t height );

		virtual bool operator==( const Texture& other ) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D( TextureFormat format, uint32_t w, uint32_t h, TextureWrap wrap );
		Texture2D( const std::string& path, bool srgb, bool secpMap = false );
		virtual ~Texture2D();

		virtual TextureFormat Format()   const override { return m_Format; }
		virtual uint32_t Width( void )   const override { return m_Width; }
		virtual uint32_t Height( void )  const override { return m_Height; }

		virtual uint32_t MipLevelCount() const override;

		virtual void Lock();
		virtual void Unlock();

		virtual void Resize( uint32_t width, uint32_t height );

		virtual const std::string& GetPath() const { return m_FilePath; }
		virtual bool Loaded() const { return m_Loaded; }
		virtual std::string& Filename() { return m_FileName; }

	private:

		TextureFormat m_Format;
		TextureWrap m_Wrap = TextureWrap::Clamp;
		uint32_t m_Width, m_Height;

		stbi_uc* m_ImageData;

		D3D12_SHADER_RESOURCE_VIEW_DESC m_ResourceViewDesc = {};
		D3D12_SUBRESOURCE_DATA m_TextureData = {};
		D3D12_RESOURCE_DESC m_TextureDesc = {};

		ComPtr<ID3D12Resource> m_Texture;

		bool m_Locked = false;
		bool m_Loaded = false;

		std::string m_FilePath;
		std::string m_FileName;
	};

}