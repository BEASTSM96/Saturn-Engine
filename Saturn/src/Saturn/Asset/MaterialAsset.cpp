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

#include "AssetRegistry.h"

#include "Saturn/Vulkan/Renderer.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

namespace Saturn {

	static Ref<Material> s_ViewingMaterial;
	static bool s_IsInViewingMode = false;
	
	static std::vector< AssetID > s_Materials;

	MaterialAsset::MaterialAsset( Ref<Material> material )
	{
		if( material == nullptr ) 
		{
			m_Material = Ref<Material>::Create( ShaderLibrary::Get().Find( "shader_new" ), "New Material" );
			Default();
		}
		else
			m_Material->Copy( material );
	}

	MaterialAsset::~MaterialAsset()
	{
	}

	void MaterialAsset::Default()
	{
		if( m_Material == nullptr )
			return;

		m_Material->SetResource( "u_AlbedoTexture", Renderer::Get().GetPinkTexture() );
		m_Material->SetResource( "u_NormalTexture", Renderer::Get().GetPinkTexture() );
		m_Material->SetResource( "u_MetallicTexture", Renderer::Get().GetPinkTexture() );
		m_Material->SetResource( "u_RoughnessTexture", Renderer::Get().GetPinkTexture() );

		m_Material->Set<glm::vec3>( "u_Materials.AlbedoColor", { 1.0f, 1.0f, 1.0f } );
		m_Material->Set<float>( "u_Materials.Metalness", 1.0f );
		m_Material->Set<float>( "u_Materials.Roughness", 1.0f );
		m_Material->Set<float>( "u_Materials.UseNormalMap", 0.0f );

		s_ViewingMaterial = m_Material;
	}

	void MaterialAsset::BeginViewingSession()
	{
		auto& StaticMeshShader = ShaderLibrary::Get().Find( "shader_new" );

		s_ViewingMaterial = Ref<Material>::Create( StaticMeshShader, "Viewing material" );

		s_ViewingMaterial->SetResource( "u_AlbedoTexture", Renderer::Get().GetPinkTexture() );
		s_ViewingMaterial->SetResource( "u_NormalTexture", Renderer::Get().GetPinkTexture() );
		s_ViewingMaterial->SetResource( "u_MetallicTexture", Renderer::Get().GetPinkTexture() );
		s_ViewingMaterial->SetResource( "u_RoughnessTexture", Renderer::Get().GetPinkTexture() );

		s_ViewingMaterial->Set<glm::vec3>( "u_Materials.AlbedoColor", { 1.0f, 1.0f, 1.0f } );
		s_ViewingMaterial->Set<float>( "u_Materials.Metalness", 1.0f );
		s_ViewingMaterial->Set<float>( "u_Materials.Roughness", 1.0f );
		s_ViewingMaterial->Set<float>( "u_Materials.UseNormalMap", 0.0f );

		s_IsInViewingMode = true;
	}

	void MaterialAsset::SaveViewingSession()
	{
		s_IsInViewingMode = false;
	}

	void MaterialAsset::EndViewingSession()
	{
		s_IsInViewingMode = false;

		if( s_ViewingMaterial == nullptr )
			return;

		s_ViewingMaterial->SetResource( "u_AlbedoTexture", Renderer::Get().GetPinkTexture() );
		s_ViewingMaterial->SetResource( "u_NormalTexture", Renderer::Get().GetPinkTexture() );
		s_ViewingMaterial->SetResource( "u_MetallicTexture", Renderer::Get().GetPinkTexture() );
		s_ViewingMaterial->SetResource( "u_RoughnessTexture", Renderer::Get().GetPinkTexture() );

		s_ViewingMaterial->Set<glm::vec3>( "u_Materials.AlbedoColor", { 1.0f, 1.0f, 1.0f } );
		s_ViewingMaterial->Set<float>( "u_Materials.Metalness", 1.0f );
		s_ViewingMaterial->Set<float>( "u_Materials.Roughness", 1.0f );
		s_ViewingMaterial->Set<float>( "u_Materials.UseNormalMap", 0.0f );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetAlbeoMap()
	{
		return s_IsInViewingMode ? s_ViewingMaterial->GetResource( "u_AlbedoTexture" ) : m_Material->GetResource( "u_AlbedoTexture" );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetNormalMap()
	{
		return s_IsInViewingMode ? s_ViewingMaterial->GetResource( "u_NormalTexture" ) : m_Material->GetResource( "u_NormalTexture" );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetMetallicMap()
	{
		return s_IsInViewingMode ? s_ViewingMaterial->GetResource( "u_MetallicTexture" ) : m_Material->GetResource( "u_MetallicTexture" );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetRoughnessMap()
	{
		return s_IsInViewingMode ? s_ViewingMaterial->GetResource( "u_RoughnessTexture" ) : m_Material->GetResource( "u_RoughnessTexture" );
	}

	glm::vec3 MaterialAsset::GetAlbeoColor()
	{
		return s_IsInViewingMode ? s_ViewingMaterial->Get<glm::vec3>( "u_Materials.AlbedoColor" ) : m_Material->Get<glm::vec3>( "u_Materials.AlbedoColor" );
	}

	void MaterialAsset::SetAlbeoColor( glm::vec3 color )
	{
		m_ValuesChanged = true;

		if( s_IsInViewingMode )
			s_ViewingMaterial->Set<glm::vec3>( "u_Materials.AlbedoColor", color );
		else
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
			//MaterialAssetSerialiser mas;
			//mas.Serialise( this );

			m_ValuesChanged = false;
		}
	}

	bool MaterialAsset::IsInViewingMode()
	{
		return s_IsInViewingMode;
	}

	void MaterialAsset::ApplyChanges()
	{
		// Load texture (auto assume we have not loaded them).
		Ref<Texture2D> texture = nullptr;

		if( m_PendingTextureChanges.size() > 1)
		{
			// Albedo
			texture = Ref<Texture2D>::Create( m_PendingTextureChanges[ 0 ], AddressingMode::Repeat, false );
			m_Material->SetResource( "u_AlbedoTexture", texture );

			// Normal
			texture = Ref<Texture2D>::Create( m_PendingTextureChanges[ 1 ], AddressingMode::Repeat, false );
			m_Material->SetResource( "u_NormalTexture", texture );

			// Normal
			texture = Ref<Texture2D>::Create( m_PendingTextureChanges[ 2 ], AddressingMode::Repeat, false );
			m_Material->SetResource( "u_MetallicTexture", texture );

			// Normal
			texture = Ref<Texture2D>::Create( m_PendingTextureChanges[ 3 ], AddressingMode::Repeat, false );
			m_Material->SetResource( "u_RoughnessTexture", texture );

		}

		m_Material->Set<glm::vec3>( "u_Materials.AlbedoColor", s_ViewingMaterial->Get<glm::vec3>( "u_Materials.AlbedoColor" ) );

		m_Material->Set<float>( "u_Materials.Metalness", s_ViewingMaterial->Get<float>( "u_Materials.Metalness" ) );
		m_Material->Set<float>( "u_Materials.Roughness", s_ViewingMaterial->Get<float>( "u_Materials.Roughness" ) );
		m_Material->Set<float>( "u_Materials.UseNormalMap", s_ViewingMaterial->Get<float>( "u_Materials.UseNormalMap" ) );

		// MAYBE??
		//s_ViewingMaterial = nullptr;
	}

	float MaterialAsset::IsUsingNormalMap()
	{
		return s_IsInViewingMode ? s_ViewingMaterial->Get<float>( "u_Materials.UseNormalMap" ) : m_Material->Get<float>( "u_Materials.UseNormalMap" );
	}

	float MaterialAsset::GetRoughness()
	{
		return s_IsInViewingMode ? s_ViewingMaterial->Get<float>( "u_Materials.Roughness" ) : m_Material->Get<float>( "u_Materials.Roughness" );
	}

	float MaterialAsset::GetMetalness()
	{
		return s_IsInViewingMode ? s_ViewingMaterial->Get<float>( "u_Materials.Metalness" ) : m_Material->Get<float>( "u_Materials.Metalness" );
	}

	void MaterialAsset::SetAlbeoMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( "u_AlbedoTexture", rTexture );
	}

	void MaterialAsset::SetAlbeoMap( const std::filesystem::path& rPath )
	{
		m_PendingTextureChanges[ 0 ] = rPath;
	}

	void MaterialAsset::SetNormalMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( "u_NormalTexture", rTexture );
	}

	void MaterialAsset::SetNormalMap( const std::filesystem::path& rPath )
	{
		m_PendingTextureChanges[ 1 ] = rPath;
	}

	void MaterialAsset::SetMetallicMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( "u_MetallicTexture", rTexture );
	}

	void MaterialAsset::SetMetallicMap( const std::filesystem::path& rPath )
	{
		m_PendingTextureChanges[ 2 ] = rPath;
	}

	void MaterialAsset::SetRoughnessMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_Material->SetResource( "u_RoughnessTexture", rTexture );
	}

	void MaterialAsset::SetRoughnessMap( const std::filesystem::path& rPath )
	{
		m_PendingTextureChanges[ 3 ] = rPath;
	}

}