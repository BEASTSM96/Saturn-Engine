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
		AssetID Asset = 0;
		uint64_t OrginalSize = 0;
		uint64_t CompressedSize = 0;
		uint64_t Offset = 0;
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
		GetBlockingOperation()->SetTitle( "AssetBundle" );

		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();

		if( !std::filesystem::exists( cachePath ) )
			std::filesystem::create_directories( cachePath );

		cachePath /= "AssetBundle.sab";

		Timer timer;

		AssetManager& rAssetManager = AssetManager::Get();
		Ref<Project> ActiveProject = Project::GetActiveProject();

		const std::string& rMountBase = Project::GetActiveConfig().Name;

		Ref<AssetRegistry>& AssetBundleRegistry = rAssetManager.GetAssetRegistry();
		auto& rVFS = VirtualFS::Get();

		// Start by dumping all of the assets
		CreateTempDirIfNeeded();

		std::unordered_map<std::filesystem::path, AssetID> DumpFileToAssetID;

		bool DumpedAllFiles = false;

		// Submit on the main thread because we will get threading issue with vulkan when loading static meshes.
		Application::Get().SubmitOnMainThread( [&]() 
			{
				for( auto& [id, asset] : AssetBundleRegistry->GetAssetMap() )
				{
					RTDumpAsset( asset );

					std::filesystem::path p = ActiveProject->GetTempDir() / std::to_string( id );
					p.replace_extension( ".vfs" );

					DumpFileToAssetID[ p ] = id;
				}

				DumpedAllFiles = true;
			} );

		// Wait for main thread to dump all of the assets.
		while( !DumpedAllFiles )
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for( 1ms );
		}

		SAT_CORE_INFO( "Dumped {0} asset(s)", rAssetManager.GetAssetRegistrySize() );

		GetBlockingOperation()->SetProgress( 10.0f );
		GetBlockingOperation()->SetTitle( "Building AssetBundle" );

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
			std::string status = std::format( "Writing header information for asset: {0} ({1})", (uint64_t)id, asset->Name );

			GetBlockingOperation()->SetStatus( status );

			asset->SerialiseData( fout );

			rVFS.Mount( rMountBase, asset->Path );
		}

		GetBlockingOperation()->SetProgress( 30.0f );

		/////////////////////////////////////
		GetBlockingOperation()->SetStatus( "Writing VFS" );

		VirtualFS::Get().WriteVFS( fout );

		GetBlockingOperation()->SetProgress( 35.0f );

		/////////////////////////////////////

		uint64_t offset = 0;

		// Next, now that we have dumped all of the assets we can now pack and compress the assets.
		// And we also make sure that we write the uncompressed/compressed file data + the header into the VFS.
		for( const auto& rEntry : std::filesystem::directory_iterator( Project::GetActiveProject()->GetTempDir() ) )
		{
			if( !rEntry.is_regular_file() )
				continue;

			std::filesystem::path path = rEntry.path();

			std::vector<char> fileBuffer;
			std::ifstream stream( path, std::ios::binary | std::ios::in | std::ios::ate );

			uint64_t fileSize = stream.tellg();

			DumpFileHeader dfh;
			dfh.Asset = DumpFileToAssetID[ path ];
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
				
				GetBlockingOperation()->SetStatus( std::format( "Compressing file: {0}", path.string() ) );

				// Compress, file over the limit.
				std::vector<char> compressedData;
				compressedData.resize( compressBound( (uLong)fileSize ) );

				uLongf compressedSize = (uLongf)compressedData.size();

				int result = compress( (Bytef*)compressedData.data(), &compressedSize, (Bytef*)fileBuffer.data(), static_cast<uLong>( fileBuffer.size() ) );

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
				dfh.CompressedSize = dfh.OrginalSize;

				SAT_CORE_INFO( "Not compressing file: {0} because file size is less than 500 KB", path.string() );
				GetBlockingOperation()->SetStatus( std::format( "Not Compressing file because file size is less than 500 KB: {0}", path.string() ) );

				RawSerialisation::WriteObject( dfh, fout );
				RawSerialisation::WriteVector( fileBuffer, fout );
			}

			GetBlockingOperation()->AddProgress( ( 1.0 + GetBlockingOperation()->GetProgress() ) / DumpFileToAssetID.size() );
		}

		SAT_CORE_INFO( "Packaged {0} asset(s)", rAssetManager.GetAssetRegistrySize() );
		SAT_CORE_INFO( "Asset bundle built in {0}s", timer.Elapsed() / 1000 );

		fout.close();

		GetBlockingOperation()->SetProgress( 100.0f );
		GetBlockingOperation()->SetStatus( "Done" );

		DumpFileToAssetID.clear();

		// Delete the temp folder as we will no longer be needing it.
		std::filesystem::remove_all( ActiveProject->GetTempDir() );

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
				Ref<Scene> scene = Ref<Scene>::Create();
				Scene* pOldActiveScene = GActiveScene;
				GActiveScene = scene.Get();

				SceneSerialiser serialiser( scene );
				serialiser.Deserialise( rAsset->Path );

				GActiveScene = pOldActiveScene;

				scene->SerialiseData();
			} break;

			case Saturn::AssetType::Prefab:
			{
				Ref<Prefab> prefabAsset = AssetBundleRegistry->GetAssetAs<Prefab>( id );

				if( prefabAsset )
				{
					RawPrefabSerialiser serialiser;
					serialiser.DumpAndWriteToVFS( prefabAsset );
				}
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

		Timer timer;

		std::ifstream stream( cachePath, std::ios::binary | std::ios::in );

		AssetBundleHeader header{};
		RawSerialisation::ReadObject( header, stream );

		if( strcmp( header.Magic, ".AB\0" ) )
		{
			SAT_CORE_ERROR( "Invalid asset bundle file header!" );
			return false;
		}

		AssetManager& rAssetManager = AssetManager::Get();
		Ref<AssetRegistry> rAssetRegistry = rAssetManager.GetAssetRegistry();
		VirtualFS& rVFS = VirtualFS::Get();

		const std::string& rMountBase = Project::GetActiveConfig().Name;
		rVFS.UnmountBase( rMountBase );
		rVFS.MountBase( rMountBase, Project::GetActiveProjectPath() );

		// Read header information
		for( size_t i = 0; i < header.Assets; i++ )
		{
			Ref<Asset> asset = Ref<Asset>::Create();
			asset->DeserialiseData( stream );

			rAssetRegistry->m_Assets[ asset->ID ] = asset;

			SAT_CORE_INFO( "Read asset header info: {0} ({1})", asset->ID, asset->Name );
		}

		// Load the VFS
		rVFS.LoadVFS( stream );

		std::vector<DumpFileHeader> FileEntries( header.Assets );

		// Iterate over all of the assets again. But this time read compressed file.
		for( size_t i = 0; i < header.Assets; i++ )
		{
			DumpFileHeader dfh;
			RawSerialisation::ReadObject( dfh, stream );

			if( !rAssetRegistry->DoesIDExists( dfh.Asset ) )
				continue;

			Ref<Asset>& rAsset = rAssetRegistry->m_Assets[ dfh.Asset ];

			if( strcmp( dfh.Magic, ".PAK\0" ) )
			{
				SAT_CORE_ERROR( "Invalid pack file header!" );
				break;
			}

			if( rAsset->ID != dfh.Asset )
			{
				SAT_CORE_ERROR( "Asset ID's do not match!" );
				break;
			}

			// Find the VFile
			Ref<VFile>& rFile = rVFS.FindFile( rMountBase, rAsset->Path );

			if( dfh.OrginalSize != dfh.CompressedSize )
			{
				SAT_CORE_INFO( "Decompressing file at offset {0}", dfh.Offset );

				// Compression was used, uncompress. 
				std::vector<char> uncompressedData( dfh.OrginalSize );

				std::vector<char> compressedData;
				RawSerialisation::ReadVector( compressedData, stream );

				uLongf uncompSize = (uLongf)uncompressedData.size();
				int result = uncompress( ( Bytef* ) uncompressedData.data(), &uncompSize, ( Bytef* ) compressedData.data(), static_cast<uLong>( compressedData.size() ) );

				if( result != Z_OK )
				{
					SAT_CORE_ERROR( "Failed to uncompress data!" );
					break;
				}

				compressedData.clear();

				rFile->FileContents = uncompressedData;
			}
			else
			{
				SAT_CORE_INFO( "Loading uncompressed file at offset {0}", dfh.Offset );

				std::vector<char> uncompressedData( dfh.OrginalSize );
				RawSerialisation::ReadVector( uncompressedData, stream );
				
				rFile->FileContents = uncompressedData;
			}

			FileEntries[ i ] = dfh;
		}

		SAT_CORE_INFO( "Done reading asset bundle in {0}s", timer.Elapsed() / 1000 );

		stream.close();

		FileEntries.clear();

		return true;
	}

	Ref<BlockingOperation>& AssetBundle::GetBlockingOperation()
	{
		static Ref<BlockingOperation> _ = Ref<BlockingOperation>::Create();
		return _;
	}

}