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
#include "MaterialAsset.h"

#include "AssetManager.h"

#include "Saturn/Vulkan/Renderer.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

namespace Saturn {

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
		m_Material->Set<float>( "u_Materials.Emissive", 0.0f );
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

	void MaterialAsset::SetEmissive( float val )
	{
		m_ValuesChanged = true;

		m_Material->Set<float>( "u_Materials.Emissive", val );
	}

	Saturn::Ref<Saturn::Texture2D> MaterialAsset::GetResource( const std::string& rName )
	{
		return m_Material->GetResource( rName );
	}

	void MaterialAsset::SetResource( const std::string& rName, const Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_PendingTextureChanges[ rName ] = rTexture;
	}

	void MaterialAsset::RT_Bind( const std::vector<std::vector<VkWriteDescriptorSet>>& rStorageBufferWDS )
	{
		m_Material->RN_Update();
	}

	void MaterialAsset::Reset()
	{
		m_TextureCache.clear();
		m_VPendingTextureChanges.clear();
		m_PendingTextureChanges.clear();

		// We don't want to default the texture because what if the user has only changed the normal map. And we'd be reseting all of the textures.
	}

	void MaterialAsset::Bind( const Ref< StaticMesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader, const VkWriteDescriptorSet& rStorageBufferWDS )
	{
		if( m_PendingMaterialChange )
		{
			m_Material = nullptr;

			m_Material = m_PendingMaterialChange;

			m_PendingMaterialChange = nullptr;
		}

		for( auto& [name, texture] : m_PendingTextureChanges )
		{
			if( m_TextureCache[ name ].imageView == texture->GetDescriptorInfo().imageView )
			{
				continue;
			}

			// Does not exists, add and update
			m_TextureCache[ name ] = texture->GetDescriptorInfo();
			m_Material->SetResource( name, texture );
		}

		if( m_PendingTextureChanges.size() )
			m_PendingTextureChanges.clear();

		m_Material->RN_Update();

		uint32_t frame = Renderer::Get().GetCurrentFrame();

		VkDescriptorSet CurrentSet = m_Material->GetDescriptorSet( frame );

		// Update material textures.
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
					Shader->WriteDescriptor( name, ImageInfo, m_Material->m_DescriptorSets[ frame ] );

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

			Shader->WriteDescriptor( name, ImageInfo, m_Material->m_DescriptorSets[ frame ] );
		}

		if( rStorageBufferWDS.dstBinding != 0 )
		{
			auto wds = rStorageBufferWDS;
			m_Material->WriteDescriptor( wds );
		}
		
		if( m_ValuesChanged ) 
		{
			m_ValuesChanged = false;
		}
	}

	void MaterialAsset::Clean()
	{
		m_Material->RN_Clean();
	}

	void MaterialAsset::ApplyChanges()
	{
		// Load texture (auto assume we have not loaded them).
		Ref<Texture2D> texture = nullptr;

		std::unordered_map<uint32_t, std::string> IndexToTextureIndex =
		{
			{ 0, "u_AlbedoTexture" },
			{ 1, "u_NormalTexture" },
			{ 2, "u_MetallicTexture" },
			{ 3, "u_RoughnessTexture" }
		};

		for( auto&& [index, path] : m_VPendingTextureChanges )
		{
			auto fullPath = Project::GetActiveProject()->FilepathAbs( path );
			texture = Ref<Texture2D>::Create( fullPath, AddressingMode::Repeat, false );

			m_Material->SetResource( IndexToTextureIndex[ index ], texture );
		}
	}

	void MaterialAsset::SetMaterial( const Ref<Material>& rMaterial )
	{
		m_PendingMaterialChange = rMaterial;
	}

	float MaterialAsset::IsUsingNormalMap()
	{
		return m_Material->Get<float>( "u_Materials.UseNormalMap" );
	}

	float MaterialAsset::GetRoughness()
	{
		return m_Material->Get<float>( "u_Materials.Roughness" );
	}

	float MaterialAsset::GetMetalness()
	{
		return m_Material->Get<float>( "u_Materials.Metalness" );
	}

	float MaterialAsset::GetEmissive()
	{
		return m_Material->Get<float>( "u_Materials.Emissive" );
	}

	void MaterialAsset::SetAlbeoMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_PendingTextureChanges[ "u_AlbedoTexture" ] = rTexture;
	}

	void MaterialAsset::SetAlbeoMap( const std::filesystem::path& rPath )
	{
		m_VPendingTextureChanges[ 0 ] = rPath;
	}

	void MaterialAsset::SetNormalMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_PendingTextureChanges[ "u_NormalTexture" ] = rTexture;
	}

	void MaterialAsset::SetNormalMap( const std::filesystem::path& rPath )
	{
		m_VPendingTextureChanges[ 1 ] = rPath;
	}

	void MaterialAsset::SetMetallicMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_PendingTextureChanges[ "u_MetallicTexture" ] = rTexture;
	}

	void MaterialAsset::SetMetallicMap( const std::filesystem::path& rPath )
	{
		m_VPendingTextureChanges[ 2 ] = rPath;
	}


	void MaterialAsset::SetRoughnessMap( Ref<Texture2D>& rTexture )
	{
		m_ValuesChanged = true;

		m_PendingTextureChanges[ "u_RoughnessTexture" ] = rTexture;
	}

	void MaterialAsset::SetRoughnessMap( const std::filesystem::path& rPath )
	{
		m_VPendingTextureChanges[ 3 ] = rPath;
	}

	void MaterialAsset::ForceUpdate()
	{
		for( auto& [name, texture] : m_PendingTextureChanges )
		{
			if( m_TextureCache[ name ].imageView == texture->GetDescriptorInfo().imageView )
			{
				continue;
			}

			// Does not exists, add and update
			m_TextureCache[ name ] = texture->GetDescriptorInfo();
			m_Material->SetResource( name, texture );
		}
	}

	void MaterialAsset::SetAlbeoMap( UUID AssetID )
	{
		// TEMP: because this will not work when using VFS
		// Find and load the texture

		m_PendingTextureChanges[ "u_AlbedoTexture" ] = Renderer::Get().GetPinkTexture();
	}

	void MaterialAsset::SetNormalMap( UUID AssetID )
	{
		m_PendingTextureChanges[ "u_NormalTexture" ] = Renderer::Get().GetPinkTexture();
	}

	void MaterialAsset::SetMetallicMap( UUID AssetID )
	{
		m_PendingTextureChanges[ "u_MetallicTexture" ] = Renderer::Get().GetPinkTexture();
	}

	void MaterialAsset::SetRoughnessMap( UUID AssetID )
	{
		m_PendingTextureChanges[ "u_RoughnessTexture" ] = Renderer::Get().GetPinkTexture();
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL REGISTRY
	//////////////////////////////////////////////////////////////////////////

	MaterialRegistry::MaterialRegistry()
	{
	}

	MaterialRegistry::MaterialRegistry( const Ref<StaticMesh>& mesh )
		: m_Mesh( mesh )
	{
		Copy( mesh->GetMaterialRegistry() );
	}

	MaterialRegistry::~MaterialRegistry()
	{
	}

	void MaterialRegistry::Copy( const Ref<MaterialRegistry>& rSrc )
	{
		m_Materials.clear();

		m_Materials = rSrc->m_Materials;
		m_HasOverridden = rSrc->m_HasOverridden;
	}

	void MaterialRegistry::AddAsset( uint32_t index )
	{
		m_Materials[ index ] = nullptr;
	}

	void MaterialRegistry::AddAsset( const Ref<MaterialAsset>& rAsset )
	{
		m_Materials.push_back( rAsset );
		m_HasOverridden.push_back( false );
	}

	Saturn::Ref<Saturn::MaterialAsset> MaterialRegistry::GetAsset( AssetID id )
	{
		return m_Materials.at( id );
	}

	void MaterialRegistry::SetMaterial( uint32_t index, AssetID id )
	{
		m_HasOverridden[ index ] = true;
		m_Materials[ index ] = AssetManager::Get().GetAssetAs<MaterialAsset>( id );
	}

	void MaterialRegistry::ResetMaterial( uint32_t index ) 
	{
		m_HasOverridden[ index ] = false;
		m_Materials[ index ] = m_Mesh->GetMaterialRegistry()->GetMaterials()[ index ];
	}

	bool MaterialRegistry::HasAnyOverrides()
	{
		return std::any_of( m_HasOverridden.begin(), m_HasOverridden.end(), []( bool x ) { return x; } );
	}

	void MaterialRegistry::Serialise( const MaterialRegistry& rRegistry, std::ofstream& rStream )
	{
		RawSerialisation::WriteVector( rRegistry.m_HasOverridden, rStream );
	}

	void MaterialRegistry::Deserialise( MaterialRegistry& rRegistry, std::ifstream& rStream )
	{
		RawSerialisation::ReadVector( rRegistry.m_HasOverridden, rStream );
	}

}