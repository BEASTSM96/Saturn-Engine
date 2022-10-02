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
#include "MaterialAsset.h"

#include "Saturn/Vulkan/Renderer.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

namespace Saturn {

	MaterialAsset::MaterialAsset( Ref<Material> material )
		: m_Material( material )
	{
		Default();
	}

	MaterialAsset::~MaterialAsset()
	{
	}

	void MaterialAsset::Default()
	{
		m_Material->SetResource( "u_AlbedoTexture", Renderer::Get().GetPinkTexture() );
		m_Material->SetResource( "u_NormalTexture", Renderer::Get().GetPinkTexture() );
		m_Material->SetResource( "u_MetallicTexture", Renderer::Get().GetPinkTexture() );
		m_Material->SetResource( "u_RoughnessTexture", Renderer::Get().GetPinkTexture() );

		m_Material->Set<glm::vec3>( "u_Materials.AlbedoColor", { 1.0f, 1.0f, 1.0f } );
		m_Material->Set<float>( "u_Materials.Metalness", 1.0f );
		m_Material->Set<float>( "u_Materials.Roughness", 1.0f );
		m_Material->Set<float>( "u_Materials.UseNormalMap", 0.0f );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetAlbeoMap()
	{
		return m_Material->GetResource( "u_AlbedoTexture" );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetNormalMap()
	{
		return m_Material->GetResource( "u_NormalTexture" );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetMetallicMap()
	{
		return m_Material->GetResource( "u_MetallicTexture" );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetRoughnessMap()
	{
		return m_Material->GetResource( "u_RoughnessTexture" );
	}

	glm::vec3 MaterialAsset::GetAlbeoColor()
	{
		return m_Material->Get<glm::vec3>( "u_Materials.AlbedoColor" );
	}

	void MaterialAsset::SetAlbeoColor( glm::vec3 color )
	{
		m_ValuesChanged = true;

		m_Material->Set<glm::vec3>( "u_Materials.AlbedoColor", color );
	}

	void MaterialAsset::UseNormalMap( bool val )
	{
		m_ValuesChanged = true;

		m_Material->Set<float>( "u_Materials.UseNormalMap", val );
	}

	void MaterialAsset::SetRoughness( float val )
	{
		m_ValuesChanged = true;

		m_Material->Set<float>( "u_Materials.Roughness", val );
	}

	void MaterialAsset::SetMetalness( float val )
	{
		m_ValuesChanged = true;

		m_Material->Set<float>( "u_Materials.Metalness", val );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetResource( const std::string& rName )
	{
		return m_Material->GetResource( rName );
	}

	void MaterialAsset::SetResource( const std::string& rName, const Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( rName, rTexture );
	}

	void MaterialAsset::Bind( const Ref< Mesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader )
	{
		Ref< DescriptorSet > CurrentSet = rMesh->GetDescriptorSets().at( rSubmsh );

		auto& textures = m_Material->GetTextures();

		for( auto& [name, texture] : textures )
		{
			// Check if the texture even exists in the cache.
			if( m_TextureCache.find( name ) == m_TextureCache.end() )
			{
				m_TextureCache[ name ] = texture->GetDescriptorInfo();
			}
			else
			{
				VkDescriptorImageInfo ImageInfo = m_TextureCache.at( name );

				if( m_TextureCache.at( name ).imageView == ImageInfo.imageView || m_TextureCache.at( name ).sampler == ImageInfo.sampler )
				{
					// No need to update the descriptor set -- its the same.
					continue;
				}
				else // If the image view has changed, update the cache.
				{
					m_TextureCache[ name ] = texture->GetDescriptorInfo();
					Shader->WriteDescriptor( name, ImageInfo, CurrentSet->GetVulkanSet() );

					continue;
				}
			}

			// Fallback, just write the descriptor.
			VkDescriptorImageInfo ImageInfo = {};
			ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			if( textures[ name ] )
			{
				ImageInfo.imageView = textures[ name ]->GetImageView();
				ImageInfo.sampler = textures[ name ]->GetSampler();
			}
			else
			{
				auto PinkTexture = Renderer::Get().GetPinkTexture();

				ImageInfo.imageView = PinkTexture->GetImageView();
				ImageInfo.sampler = PinkTexture->GetSampler();
			}

			Shader->WriteDescriptor( name, ImageInfo, CurrentSet->GetVulkanSet() );
		}

		if( m_ValuesChanged ) 
		{
			MaterialAssetSerialiser mas;
			mas.Serialise( this );

			m_ValuesChanged = false;
		}
	}

	float MaterialAsset::IsUsingNormalMap()
	{
		return m_Material->Get<float>( "u_Materials.UseNormalMap");
	}

	float MaterialAsset::GetRoughness()
	{
		return m_Material->Get<float>( "u_Materials.Roughness");
	}

	float MaterialAsset::GetMetalness()
	{
		return m_Material->Get<float>( "u_Materials.Metalness");
	}

	void MaterialAsset::SetAlbeoMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( "u_AlbedoTexture", rTexture );
	}

	void MaterialAsset::SetNormalMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( "u_NormalTexture", rTexture );
	}

	void MaterialAsset::SetMetallicMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( "u_MetallicTexture", rTexture );
	}

	void MaterialAsset::SetRoughnessMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( "u_RoughnessTexture", rTexture );
	}
}