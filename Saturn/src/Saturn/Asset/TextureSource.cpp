/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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
#include "TextureSource.h"

#include <stb_image.h>

namespace Saturn {

	TextureSourceAsset::TextureSourceAsset()
	{
	}

	TextureSourceAsset::TextureSourceAsset( std::filesystem::path AbsolutePath, bool Flip )
		: m_AbsolutePath( std::move( AbsolutePath ) ), m_Flipped( Flip )
	{
		LoadRawTexture();
	}

	TextureSourceAsset::~TextureSourceAsset()
	{
		m_TextureBuffer.Free();
	}

	void TextureSourceAsset::LoadRawTexture()
	{
		SAT_CORE_ASSERT( std::filesystem::exists( m_AbsolutePath ), "Path does not exist!" );

		int Width, Height, Channels;

		stbi_uc* pTextureData;

		m_HDR = stbi_is_hdr( m_AbsolutePath.string().c_str() );

		if( m_HDR )
		{
			SAT_CORE_INFO( "Loading HDR texture {0}", m_AbsolutePath.string() );
			pTextureData = ( uint8_t* ) stbi_loadf( m_AbsolutePath.string().c_str(), &Width, &Height, &Channels, 4 );
		}
		else
		{
			SAT_CORE_INFO( "Loading texture {0}", m_AbsolutePath.string() );

			pTextureData = stbi_load( m_AbsolutePath.string().c_str(), &Width, &Height, &Channels, 4 );
		}

		m_Width = Width;
		m_Height = Height;
		m_Channels = Channels;

		uint32_t ImageSize = m_Width * m_Height * 4;

		m_TextureBuffer = Buffer::Copy( pTextureData, static_cast<size_t>( ImageSize ) );

		stbi_image_free( pTextureData );
	}

}