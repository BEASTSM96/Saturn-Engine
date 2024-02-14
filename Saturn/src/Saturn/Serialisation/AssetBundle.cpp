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
#include "AssetBundle.h"

#include "Saturn/Asset/Asset.h"
#include "Saturn/Asset/AssetManager.h"

#include "Saturn/Core/VirtualFS.h"

#include "Saturn/Project/Project.h"
#include "Saturn/Serialisation/RawSerialisation.h"
#include "Saturn/Serialisation/RawAssetSerialisers.h"

#include "Saturn/Asset/TextureSourceAsset.h"
#include "Saturn/Asset/Prefab.h"
#include "Saturn/Asset/MaterialAsset.h"

#include "Saturn/Asset/PhysicsMaterialAsset.h"

#include "Saturn/Serialisation/SceneSerialiser.h"

namespace Saturn {

	struct AssetBundleHeader
	{
		const char Magic[ 5 ] = ".AB\0";
		size_t Assets;
	};

	AssetBundle::AssetBundle()
	{
	}

	AssetBundle::~AssetBundle()
	{
	}

	bool AssetBundle::BundleAssets()
	{
		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();

		if( !std::filesystem::exists( cachePath ) )
			std::filesystem::create_directories( cachePath );

		cachePath /= "AssetBundle.sab";

		std::ofstream fout( cachePath, std::ios::binary | std::ios::trunc );

		AssetManager& rAssetManager = AssetManager::Get();
		const std::string& rMountBase = Project::GetActiveConfig().Name;

		// Construct a temporary asset registry to hold all of our assets in.
		Ref<AssetRegistry> AssetBundleRegistry = Ref<AssetRegistry>::Create( *rAssetManager.GetAssetRegistry().Get() );

		AssetBundleHeader header{};
		header.Assets = rAssetManager.GetAssetRegistrySize();

		fout.write( reinterpret_cast<char*>( &header ), sizeof( AssetBundleHeader ) );

		// Steps:
		// 1), write the normal asset data, such as ID, Name, Path. (So basically write the AssetRegistry)
		// 2), write into the VFS loaded data.
		// 3), write the VFS.

		for( auto& [id, asset] : AssetBundleRegistry->GetAssetMap() )
		{
			SAT_CORE_INFO( "Packaging asset: {0} ({1})", id, asset->Name );
			
			asset->SerialiseData( fout );

			VirtualFS::Get().Mount( rMountBase, asset->Path );
		}

		auto& rVFS = VirtualFS::Get();

		for( auto& [id, asset] : AssetBundleRegistry->GetAssetMap() )
		{
			SAT_CORE_INFO( "Writing loaded asset data into VFS: {0} ({1})", id, asset->Name );

			switch( asset->Type )
			{
				case Saturn::AssetType::Texture:
				{
					// Read the raw texture file into the virtual FS.
					// To do this we can use our TextureSourceAsset class.
					auto AbsolutePath = Project::GetActiveProject()->FilepathAbs( asset->Path );
					Ref<TextureSourceAsset> sourceAsset = Ref<TextureSourceAsset>::Create( AbsolutePath );
					sourceAsset->Path = asset->Path;

					sourceAsset->WriteToVFS();
				} break;

				case Saturn::AssetType::StaticMesh:
				{
					Ref<StaticMesh> mesh = AssetBundleRegistry->GetAssetAs<StaticMesh>( id );

					RawStaticMeshAssetSerialiser serialiser;
					//serialiser.Serialise( mesh, fout );
				} break;

				case Saturn::AssetType::Material:
				{
					Ref<MaterialAsset> materialAsset = AssetBundleRegistry->GetAssetAs<MaterialAsset>( id );

					if( materialAsset )
					{
						RawMaterialAssetSerialiser serialiser;
						serialiser.WriteToVFS( materialAsset );
					}
				} break;

				case Saturn::AssetType::PhysicsMaterial:
				{
					Ref<PhysicsMaterialAsset> physAsset = AssetBundleRegistry->GetAssetAs<PhysicsMaterialAsset>( id );
					
					RawPhysicsMaterialAssetSerialiser serialiser;
					serialiser.WriteToVFS( physAsset );
				} break;

				case Saturn::AssetType::Scene:
				{
					/*
					Ref<Scene> scene = Ref<Scene>::Create();
					Scene* pOldActiveScene = GActiveScene;
					GActiveScene = scene.Get();

					SceneSerialiser serialiser( scene );
					serialiser.Deserialise( asset->Path );

					GActiveScene = pOldActiveScene;

					scene->SerialiseData( fout );
					*/
				} break;

				case Saturn::AssetType::Prefab:
				{
					/*
					Ref<Prefab> asset = AssetBundleRegistry->GetAssetAs<Prefab>( id );

					if( asset )
					{
						RawPrefabSerialiser serialiser;
						serialiser.Serialise( asset, fout );
					}
					*/
				} break;

				case Saturn::AssetType::SkeletalMesh:
				case Saturn::AssetType::MaterialInstance:
				case Saturn::AssetType::Audio:
				case Saturn::AssetType::Script:
				case Saturn::AssetType::MeshCollider:
				case Saturn::AssetType::Unknown:
				default:
					break;
			}
		}

		VirtualFS::Get().WriteVFS( fout );

		SAT_CORE_INFO( "Packaged {0} asset(s)", rAssetManager.GetAssetRegistrySize() );

		fout.close();

		AssetBundleRegistry = nullptr;

		return true;
	}

	void AssetBundle::ReadBundle()
	{
		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();
		cachePath /= "AssetBundle.sab";

		if( !std::filesystem::exists( cachePath ) )
			return;

		std::ifstream stream( cachePath, std::ios::binary | std::ios::in );

		AssetBundleHeader header{};
		stream.read( reinterpret_cast< char* >( &header ), sizeof( AssetBundleHeader ) );

		if( strcmp( header.Magic, ".AB\0" ) )
		{
			SAT_CORE_ERROR( "Invalid shader bundle file header!" );
			return;
		}

		AssetManager& rAssetManager = AssetManager::Get();
		Ref<AssetRegistry> rAssetRegistry = Ref<AssetRegistry>::Create();

		// Steps:
		// 1) Read the asset registry
		// 2) Read the loaded data and the VFS

		const std::string& rMountBase = Project::GetActiveConfig().Name;
		VirtualFS::Get().UnmountBase( rMountBase );
		VirtualFS::Get().MountBase( rMountBase, Project::GetActiveProjectPath() );

		for( size_t i = 0; i < header.Assets; i++ )
		{
			Ref<Asset> asset = Ref<Asset>::Create();
			asset->DeserialiseData( stream );

			VirtualFS::Get().Mount( rMountBase, asset->Path );

			rAssetRegistry->m_Assets[ asset->ID ] = asset;

			SAT_CORE_INFO( "Unpacking asset: {0} ({1})", asset->ID, asset->Name );
		}

		// Load the VFS
		VirtualFS::Get().LoadVFS( stream );

		stream.close();
	}

}