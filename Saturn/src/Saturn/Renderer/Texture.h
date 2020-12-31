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
#include "Saturn/Core/Buffer.h"
#include "RendererAPI.h"

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


	class Texture : public RefCounted
	{
	public:
		virtual ~Texture() { }

		virtual void Bind( uint32_t slot = 0 ) const = 0;

		virtual TextureFormat GetFormat( void ) const = 0;

		virtual uint32_t GetWidth( void ) const = 0;
		virtual uint32_t GetHeight( void ) const = 0;
		virtual uint32_t GetMipLevelCount( void ) const = 0;

		virtual RendererID GetRendererID( void ) const = 0;

		static uint32_t GetBPP( TextureFormat format );
		static uint32_t CalculateMipMapCount( uint32_t width, uint32_t height );

		virtual bool operator==( const Texture& other ) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create( TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap = TextureWrap::Clamp );
		static Ref<Texture2D> Create( const std::string& path, bool srgb = false );

		virtual void Lock( void ) = 0;
		virtual void Unlock( void ) = 0;

		virtual void Resize( uint32_t width, uint32_t height ) = 0;
		virtual Buffer GetWriteableBuffer( void ) = 0;

		virtual bool Loaded( void ) const = 0;

		virtual const std::string& GetPath() const = 0;
	};

	class TextureCube : public Texture
	{
	public:
		static Ref<TextureCube> Create( TextureFormat format, uint32_t width, uint32_t height );
		static Ref<TextureCube> Create( const std::string& path );

		virtual const std::string& GetPath( void ) const = 0;
	};
}

