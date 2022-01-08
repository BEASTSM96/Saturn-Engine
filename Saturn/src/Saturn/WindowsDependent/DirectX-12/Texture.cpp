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
#include "Texture.h"

#include "Renderer.h"

namespace Saturn {

	Texture2D::Texture2D( TextureFormat format, uint32_t w, uint32_t h, TextureWrap wrap )
	{
		ComPtr<ID3D12Resource> textureUploadHeap;

		// Create Texture desc - D3D12_RESOURCE_DESC
		m_TextureDesc.MipLevels = 1;
		m_TextureDesc.Format = SaturnToDXTextureFormat( format );
		m_TextureDesc.Width = w;
		m_TextureDesc.Height = h;
		m_TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		m_TextureDesc.DepthOrArraySize = 1;
		m_TextureDesc.SampleDesc.Count = 1;
		m_TextureDesc.SampleDesc.Quality = 0;
		m_TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ThrowIfFailed( Renderer::Get().Device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ), D3D12_HEAP_FLAG_NONE, &m_TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS( &m_Texture ) ) );

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize( m_Texture.Get(), 0, 1 );

		// Create GPU buffer
		ThrowIfFailed( Renderer::Get().Device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer( uploadBufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( &textureUploadHeap ) ) );
	}

	Texture2D::Texture2D( const std::string& path, bool srgb, bool secpMap /*= false */ )
	{
		size_t found = path.find_last_of( "/\\" );
		m_FileName = found != std::string::npos ? path.substr( found + 1 ) : path;

		found = m_FileName.find_last_of( "." );
		m_FileName = found != std::string::npos ? m_FileName.substr( 0, found ) : m_FileName;

		stbi_set_flip_vertically_on_load( false );

		int w, h, chan = 0;
		ComPtr<ID3D12Resource> textureUploadHeap;

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

		// Create Texture desc - D3D12_RESOURCE_DESC
		m_TextureDesc.MipLevels = 1;
		m_TextureDesc.Format = SaturnToDXTextureFormat( m_Format );
		m_TextureDesc.Width = m_Width;
		m_TextureDesc.Height = m_Height;
		m_TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		m_TextureDesc.DepthOrArraySize = 1;
		m_TextureDesc.SampleDesc.Count = 1;
		m_TextureDesc.SampleDesc.Quality = 0;
		m_TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ThrowIfFailed( Renderer::Get().Device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ), D3D12_HEAP_FLAG_NONE, &m_TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS( &m_Texture ) ) );

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize( m_Texture.Get(), 0, 1 );

		// Create GPU buffer
		ThrowIfFailed( Renderer::Get().Device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer( uploadBufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( &textureUploadHeap ) ) );

		m_TextureData.pData = &m_ImageData;
		m_TextureData.RowPitch = m_Width * 4;
		m_TextureData.SlicePitch = m_TextureData.RowPitch * m_Height;

		UpdateSubresources( Renderer::Get().CommandList().Get(), m_Texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &m_TextureData );
		Renderer::Get().CommandList()->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ) );

		m_ResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		m_ResourceViewDesc.Format = m_TextureDesc.Format;
		m_ResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		m_ResourceViewDesc.Texture2D.MipLevels = 1;
		Renderer::Get().Device()->CreateShaderResourceView( m_Texture.Get(), &m_ResourceViewDesc, Renderer::Get().RTVHeap()->GetCPUDescriptorHandleForHeapStart() );

		stbi_image_free( m_ImageData );
	}

	Texture2D::~Texture2D()
	{
		m_ResourceViewDesc = {};
		m_TextureData = {};
		m_TextureDesc = {};
	}

	uint32_t Texture2D::MipLevelCount() const
	{
		return m_TextureDesc.MipLevels;
	}

	void Texture2D::Lock()
	{

	}

	void Texture2D::Unlock()
	{

	}

	void Texture2D::Resize( uint32_t width, uint32_t height )
	{
		m_TextureDesc.Width = width;
		m_TextureDesc.Height = height;
	}

}