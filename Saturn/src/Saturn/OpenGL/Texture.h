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

#pragma once

#include "Saturn/Core/Base.h"
#include "Common.h"

#include <string>

#include <stdint.h>

namespace Saturn {

	enum class TextureFormat
	{
		None = 0,
		RGB = 1,
		RGBA = 2,
		Float16 = 3
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp = 1,
		Repeat = 2
	};

	class Texture
	{
	public:

		virtual ~Texture() {  }

		virtual void Bind() const = 0;

		virtual TextureFormat Format() const = 0;

		virtual uint32_t Width( void ) const = 0;
		virtual uint32_t Height( void ) const = 0;
		virtual uint32_t MipLevelCount( void ) const = 0;

		virtual RendererID GetRendererID( void ) const = 0;

		static uint32_t BPP( TextureFormat format );
		static uint32_t MipMapCount( uint32_t width, uint32_t height );

		virtual bool operator==( const Texture& other ) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D( TextureFormat format, uint32_t w, uint32_t h, TextureWrap wrap );
		Texture2D( const std::string& path, bool srgb );
		virtual ~Texture2D();

		virtual void Bind( uint32_t slot ) const;
		virtual void Bind() const override { Bind( 0 ); }

		virtual TextureFormat Format()   const override { return m_Format; }
		virtual uint32_t Width( void )   const override { return m_Width;  }
		virtual uint32_t Height( void )  const override { return m_Height; }

		virtual uint32_t MipLevelCount() const override;

		virtual void Lock();
		virtual void Unlock();

		virtual void Resize( uint32_t width, uint32_t height );
		
		virtual const std::string& GetPath() const  { return m_FilePath;   }
		virtual bool Loaded() const                 { return m_Loaded;     }
		virtual RendererID GetRendererID() const    { return m_RendererID; }

		virtual bool operator==( const Texture& other ) const override
		{
			return m_RendererID == ( ( Texture2D& )other ).m_RendererID;
		}

	private:

		RendererID m_RendererID;
		TextureFormat m_Format;
		TextureWrap m_Wrap = TextureWrap::Clamp;
		uint32_t m_Width, m_Height;

		unsigned char* m_ImageData;

		bool m_Locked = false;
		bool m_Loaded = false;

		std::string m_FilePath;
	};

}