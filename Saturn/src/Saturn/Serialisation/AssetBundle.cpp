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

#include <zlib.h>

#define PACK_ASSET( rAsset, rVFS ) \
std::filesystem::path out = tempDir; \
out /= std::to_string( rAsset->ID ); \
out.replace_extension( ".vfs" );\
rVFS.RT_PackFile( Project::GetActiveConfig().Name, rAsset->Path, out );

namespace Saturn {

	struct AssetBundleHeader
	{
		const char Magic[ 5 ] = ".AB\0";
		size_t Assets;
		uint32_t Version;
	};

	struct DumpFileHeader
	{
		char Magic[ 5 ] = { '.', 'P', 'A', 'K' };
		AssetID Filename = 0;
		uint32_t OrginalSize = 0;
		uint32_t CompressedSize = 0;
		uint32_t Offset = 0;
		uint32_t Version = 0;
	};

	static void CreateTempDirIfNeeded()
	{
		std::filesystem::path tempDir = Project::GetActiveProject()->GetRootDir();
		tempDir /= "Temp";

		if( !std::filesystem::exists( tempDir ) )
			std::filesystem::create_directories( tempDir );
	}

	bool AssetBundle::BundleAssets()
	{
		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();

		if( !std::filesystem::exists( cachePath ) )
			std::filesystem::create_directories( cachePath );

		cachePath /= "AssetBundle.sab";

		Timer timer;

		AssetManager& rAssetManager = AssetManager::Get();
		const std::string& rMountBase = Project::GetActiveConfig().Name;

		Ref<AssetRegistry>& AssetBundleRegistry = rAssetManager.GetAssetRegistry();

		// Start by dumping all of the assets
		CreateTempDirIfNeeded();

		for( auto& [id, asset] : AssetBundleRegistry->GetAssetMap() )
		{
			SAT_CORE_INFO( "Dumping loaded asset into file: {0} ({1})", id, asset->Name );

			RTDumpAsset( asset );
		}

		SAT_CORE_INFO( "Dumped {0} asset(s)", rAssetManager.GetAssetRegistrySize() );

		// This might not be needed, however, I just want to be nice and give the I/O a time to rest.
		using namespace std::chrono_literals;
		std::this_thread::sleep_for( 1ms );

		/////////////////////////////////////

		std::ofstream fout( cachePath, std::ios::binary | std::ios::trunc );
		
		AssetBundleHeader header{};
		header.Assets = rAssetManager.GetAssetRegistrySize();
		header.Version = 1;

		RawSerialisation::WriteObject( header, fout );

		// Write asset header data.
		for( auto& [id, asset] : AssetBundleRegistry->GetAssetMap() )
		{
			SAT_CORE_INFO( "Writing header information for asset: {0} ({1})", id, asset->Name );

			asset->SerialiseData( fout );

			VirtualFS::Get().Mount( rMountBase, asset->Path );
		}

		/////////////////////////////////////

		uint32_t offset = 0;

		// Next, now that we have dumped all of the assets we can now pack and compress the assets.
		for( const auto& rEntry : std::filesystem::directory_iterator( Project::GetActiveProject()->GetTempDir() ) )
		{
			if( !rEntry.is_regular_file() )
				continue;

			std::filesystem::path path = rEntry.path();

			std::vector<char> fileBuffer;
			std::ifstream stream( path, std::ios::binary | std::ios::in | std::ios::ate );

			uint64_t fileSize = stream.tellg();

			DumpFileHeader dfh;
			dfh.Filename = 0;
			dfh.Version = 1;
			dfh.OrginalSize = fileSize;
			dfh.Offset = offset;

			offset += fileSize;

			stream.seekg( 0 );

			fileBuffer.resize( fileSize );
			stream.read( fileBuffer.data(), fileSize );

			stream.close();

			// Compression, allow for files under 500KB (0.5MB) to not be compressed.
			if( fileSize > 500 * 1024 )
			{
				SAT_CORE_INFO( "Compressing file: {0} because file is {1} KB", path.string(), fileSize / 1000 );
				
				// Compress, file over the limit.
				std::vector<char> compressedData;
				compressedData.resize( compressBound( fileSize ) );

				uLongf compressedSize = compressedData.size();

				int result = compress( (Bytef*)compressedData.data(), &compressedSize, (Bytef*)fileBuffer.data(), fileBuffer.size() );

				if( result != Z_OK )
				{
					SAT_CORE_ERROR( "Failed to compress {0}! zlib error is: {1}. Writing uncompressed data.", path.string(
					), result );

					RawSerialisation::WriteObject( dfh, fout );
					RawSerialisation::WriteVector( fileBuffer, fout );
				}

				SAT_CORE_INFO( "Compressed file: {0} new file size is {1} KB", path.string(), compressedSize / 1000 );

				compressedData.resize( compressedSize );
				dfh.CompressedSize = compressedSize;

				RawSerialisation::WriteObject( dfh, fout );
				RawSerialisation::WriteVector( compressedData, fout );
			}
			else
			{
				SAT_CORE_INFO( "Not compressing file: {0} because file size is less than 500 KB", path.string() );

				RawSerialisation::WriteObject( dfh, fout );
				RawSerialisation::WriteVector( fileBuffer, fout );
			}
		}

		VirtualFS::Get().WriteVFS( fout );

		SAT_CORE_INFO( "Packaged {0} asset(s)", rAssetManager.GetAssetRegistrySize() );
		SAT_CORE_INFO( "Asset bundle built in {0}s", timer.Elapsed() );

		fout.close();

		// Delete the temp folder as we will no longer be needing it.
		std::filesystem::remove_all( Project::GetActiveProject()->GetTempDir() );

		AssetBundleRegistry = nullptr;

		return true;
	}

	void AssetBundle::RTDumpAsset( const Ref<Asset>& rAsset )
	{
		std::filesystem::path tempDir = Project::GetActiveProject()->GetRootDir();
		tempDir /= "Temp";

		Ref<AssetRegistry>& AssetBundleRegistry = AssetManager::Get().GetAssetRegistry();
		auto& rVFS = VirtualFS::Get();

		UUID id = rAsset->ID;

		// Load the asset and dump memory into our temporary file.
		switch( rAsset->Type )
		{
			case Saturn::AssetType::Texture:
			{
				// Read the raw texture file into the virtual FS.
				// To do this we can use our TextureSourceAsset class.
				auto AbsolutePath = Project::GetActiveProject()->FilepathAbs( rAsset->Path );
				Ref<TextureSourceAsset> sourceAsset = Ref<TextureSourceAsset>::Create( AbsolutePath );
				sourceAsset->Path = rAsset->Path;
				sourceAsset->ID = rAsset->ID;

				sourceAsset->WriteToVFS();
			} break;

			case Saturn::AssetType::StaticMesh:
			{
				Ref<StaticMesh> mesh = AssetBundleRegistry->GetAssetAs<StaticMesh>( id );

				RawStaticMeshAssetSerialiser serialiser;
				serialiser.DumpAndWriteToVFS( mesh );
			} break;

			case Saturn::AssetType::Material:
			{
				Ref<MaterialAsset> materialAsset = AssetBundleRegistry->GetAssetAs<MaterialAsset>( id );

				if( materialAsset )
				{
					RawMaterialAssetSerialiser serialiser;
					serialiser.DumpAndWriteToVFS( materialAsset );
				}
			} break;

			case Saturn::AssetType::PhysicsMaterial:
			{
				Ref<PhysicsMaterialAsset> physAsset = AssetBundleRegistry->GetAssetAs<PhysicsMaterialAsset>( id );

				RawPhysicsMaterialAssetSerialiser serialiser;
				serialiser.DumpAndWriteToVFS( physAsset );
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

	bool AssetBundle::ReadBundle()
	{
		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();
		cachePath /= "AssetBundle.sab";

		if( !std::filesystem::exists( cachePath ) )
			return false;

		std::ifstream stream( cachePath, std::ios::binary | std::ios::in );

		AssetBundleHeader header{};
		stream.read( reinterpret_cast< char* >( &header ), sizeof( AssetBundleHeader ) );

		if( strcmp( header.Magic, ".AB\0" ) )
		{
			SAT_CORE_ERROR( "Invalid shader bundle file header!" );
			return false;
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

			rAssetRegistry->m_Assets[ asset->ID ] = asset;

			SAT_CORE_INFO( "Unpacking asset: {0} ({1})", asset->ID, asset->Name );
		}

		// Load the VFS
		VirtualFS::Get().LoadVFS( stream );

		stream.close();

		return true;

	}
}