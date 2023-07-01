/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "EditorAssetRegistry.h"

#include "Saturn/Serialisation/EditorAssetRegistrySerialiser.h"

namespace Saturn {

	EditorAssetRegistry::EditorAssetRegistry()
		: AssetRegistryBase( AssetRegistryType::Editor )
	{
		SingletonStorage::Get().AddSingleton( this );
	}

	EditorAssetRegistry::~EditorAssetRegistry()
	{
	}

	AssetID EditorAssetRegistry::CreateAsset( AssetType type )
	{
		Ref<Asset> asset = Ref<Asset>::Create();
		asset->Type = type;
		asset->ID = UUID();

		m_Assets[ asset->GetAssetID() ] = asset;

		return asset->GetAssetID();
	}

	Ref<Asset> EditorAssetRegistry::FindAsset( AssetID id )
	{
		return m_Assets.at( id );
	}

	Ref<Asset> EditorAssetRegistry::FindAsset( const std::filesystem::path& rPath )
	{
		for( const auto& [id, asset] : m_Assets )
		{
			if( asset->GetPath() == rPath )
				return asset;
		}

		return nullptr;
	}

	Ref<Asset> EditorAssetRegistry::FindAsset( const std::string& rName, AssetType type )
	{
		for( const auto& [id, asset] : m_Assets )
		{
			if( asset->Name == rName && asset->GetAssetType() == type )
				return asset;
		}

		return nullptr;
	}

	std::vector<AssetID> EditorAssetRegistry::FindAssetsWithType( AssetType type ) const
	{
		std::vector<AssetID> result;

		// There is a better way of doing this however we'll just keep it for now.
		for( const auto& [id, asset] : m_Assets )
		{
			if( asset->GetAssetType() == type )
				result.push_back( id );
		}

		return result;
	}

	AssetID EditorAssetRegistry::PathToID( const std::filesystem::path& rPath )
	{
		for( const auto& [id, asset] : m_Assets )
		{
			if( asset->GetPath() == rPath )
				return id;
		}

		return 0;
	}

	static std::vector<std::string> s_DisallowedAssetExtensions
	{
		{ ".lib"       }, 
		{ ".hdr"       }, 
		{ ".eng"       }, 
		{ ".lua"       }, 
		{ ".cpp"       }, 
		{ ".h"         },
		{ ".cs"        },
		{ ".sproject"  },
		{ ".ttf"       },
		{ ".fbx"       },  // Already in the static mesh asset
		{ ".gltf"      }, // Already in the static mesh asset
		{ ".bin"       },  // Already in the static mesh asset
		{ ".glb"       },  // Already in the static mesh asset
		{ ".wav"       },  // Already in the sound 2d asset
		{ ".glsl"	   }
	};

	void EditorAssetRegistry::CheckMissingAssetRefs()
	{
		bool WriteAssetRegistry = false;

		std::filesystem::path path = "content/AssetRegistry.sreg";
		auto rRootPath = std::filesystem::current_path();
		rRootPath /= "content";

		for( auto& rEntry : std::filesystem::recursive_directory_iterator( rRootPath ) )
		{
			if( rEntry.is_directory() )
				continue;

			std::filesystem::path filepath = std::filesystem::relative( rEntry.path(), rRootPath );
			auto filepathString = filepath.extension().string();

			if( filepath.extension() == ".sreg" )
				continue;

			Ref<Asset> asset = EditorAssetRegistry::Get().FindAsset( filepath );

			// Extension is forbidden.
			if( std::find( s_DisallowedAssetExtensions.begin(), s_DisallowedAssetExtensions.end(), filepathString ) != s_DisallowedAssetExtensions.end() )
				continue;

			const auto& assetReg = EditorAssetRegistry::Get().GetAssetMap();
			if( asset == nullptr )
			{
				auto type = AssetTypeFromExtension( filepathString );
				auto id = EditorAssetRegistry::Get().CreateAsset( type );
				asset = EditorAssetRegistry::Get().FindAsset( id );

				asset->Path = filepath;	
				asset->Name = filepath.replace_extension().filename().string();
				
				WriteAssetRegistry = true;
			}
		}

		if( WriteAssetRegistry )
		{
			EditorAssetRegistrySerialiser edrs;
			edrs.Serialise();
		}
	}

	void EditorAssetRegistry::AddAsset( AssetID id )
	{
		SAT_CORE_ASSERT( m_Assets.find( id ) == m_Assets.end(), "Asset already exists!" );

		m_Assets[ id ] = Ref<Asset>::Create();
		m_Assets[ id ]->ID = id;
	}

}
