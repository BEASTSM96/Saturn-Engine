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

	AssetBundleResult AssetBundle::BundleAssets( Ref<JobProgress>& jobProgress )
	{
		jobProgress->Reset();
		jobProgress->SetTitle( "AssetBundle" );

		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();

		if( !std::filesystem::exists( cachePath ) )
			std::filesystem::create_directories( cachePath );

		cachePath /= "AssetBundle.sab";

		Timer timer;

		AssetManager& rAssetManager = AssetManager::Get();
		Ref<Project> ActiveProject = Project::GetActiveProject();

		const std::string& rMountBase = Project::GetActiveConfig().Name;

		Ref<AssetRegistry> AssetBundleRegistry = Ref<AssetRegistry>::Create();
		AssetBundleRegistry->CopyFrom( rAssetManager.GetAssetRegistry() );

		auto& rVFS = VirtualFS::Get();

		// Start by dumping all of the assets
		CreateTempDirIfNeeded();

		std::unordered_map<std::filesystem::path, AssetID> DumpFileToAssetID;

		jobProgress->SetTitle( "Loading assets..." );

		// THREAD-TRANSTION, Block main thread
		Application::Get().SuspendMainThreadCV();

		for( auto& [id, asset] : AssetBundleRegistry->GetAssetMap() )
		{
			SAT_CORE_INFO( "Dumping asset to disk: {0}", asset->Name );

			RTDumpAsset( asset, AssetBundleRegistry );

			std::filesystem::path p = ActiveProject->GetTempDir() / std::to_string( id );
			p.replace_extension( ".vfs" );

			DumpFileToAssetID[ p ] = id;
		}

		for( auto& [id, asset] : AssetBundleRegistry->GetAssetMap() )
		{
			if( asset->Type != AssetType::Scene )
				continue;

			Ref<Scene> scene = Ref<Scene>::Create();
			scene->Path = asset->Path;
			scene->ID = asset->ID;

			SceneSerialiser serialiser( scene );
			serialiser.Deserialise();

			scene->SerialiseData();
		}

		// THREAD-TRANSTION, Resume main thread
		Application::Get().ResumeMainThreadCV();

		SAT_CORE_INFO( "Dumped {0} asset(s)", rAssetManager.GetAssetRegistrySize() );
				
		jobProgress->SetProgress( 10.0f );
		jobProgress->SetTitle( "Building AssetBundle" );
		
		/////////////////////////////////////

		std::ofstream fout( cachePath, std::ios::binary | std::ios::trunc );
	
		AssetBundleHeader header{};
		header.Assets = rAssetManager.GetAssetRegistrySize();
		header.Version = SAT_CURRENT_VERSION;

		RawSerialisation::WriteObject( header, fout );

		// Write asset header data.
		for( auto& [id, asset] : AssetBundleRegistry->GetAssetMap() )
		{
			SAT_CORE_INFO( "Writing header information for asset: {0} ({1})", id, asset->Name );
			std::string status = std::format( "Writing header information for asset: {0} ({1})", (uint64_t)id, asset->Name );

			jobProgress->SetStatus( status );

			asset->SerialiseData( fout );

			rVFS.Mount( rMountBase, asset->Path );
		}

		jobProgress->SetProgress( 35.0f );
		jobProgress->SetStatus( "Writing VFS" );
		
		/////////////////////////////////////
		
		VirtualFS::Get().WriteVFS( fout );

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
			dfh.Asset = DumpFileToAssetID.at( path );
			dfh.Version = SAT_CURRENT_VERSION;
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
				SAT_CORE_WARN( "Compressing file: {0} because file is {1} KB", path.string(), fileSize / 1000 );
				
				jobProgress->SetStatus( std::format( "Compressing file: {0}", path.string() ) );

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

				SAT_CORE_WARN( "Not compressing file: {0} because file size is less than 500 KB", path.string() );
				jobProgress->SetStatus( std::format( "Not Compressing file because file size is less than 500 KB: {0}", path.string() ) );

				RawSerialisation::WriteObject( dfh, fout );
				RawSerialisation::WriteVector( fileBuffer, fout );
			}

			jobProgress->AddProgress( ( 1.0f + jobProgress->GetProgress() ) / DumpFileToAssetID.size() );
		}

		SAT_CORE_INFO( "Packaged {0} asset(s)", rAssetManager.GetAssetRegistrySize() );
		SAT_CORE_INFO( "Asset bundle built in {0}s", timer.Elapsed() / 1000 );

		fout.close();

		jobProgress->SetProgress( 100.0f );
		jobProgress->SetStatus( "Done" );

		DumpFileToAssetID.clear();

		// Delete the temp folder as we will no longer be needing it.
		std::filesystem::remove_all( ActiveProject->GetTempDir() );

		AssetBundleRegistry = nullptr;

		jobProgress->OnComplete();

		return AssetBundleResult::Success;
	}

	void AssetBundle::RTDumpAsset( const Ref<Asset>& rAsset, Ref<AssetRegistry>& AssetBundleRegistry )
	{
		UUID id = rAsset->ID;
		AssetManager& rAssetManager = AssetManager::Get();

		// Load the asset and dump memory into it's temporary file.
		// NOTE: We use the asset manager but ask it to load the asset into our asset registry.
		// NOTE: Asset manager will use it own importer which is fine as it will be the YAML importer.
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
				Ref<StaticMesh> mesh = rAssetManager.GetAssetAs<StaticMesh>( AssetBundleRegistry, id );

				RawStaticMeshAssetSerialiser serialiser;
				serialiser.DumpAndWriteToVFS( mesh );
			} break;

			case Saturn::AssetType::Material:
			{
				Ref<MaterialAsset> materialAsset = rAssetManager.GetAssetAs<MaterialAsset>( AssetBundleRegistry, id );

				if( materialAsset )
				{
					RawMaterialAssetSerialiser serialiser;
					serialiser.DumpAndWriteToVFS( materialAsset );
				}
			} break;

			case Saturn::AssetType::PhysicsMaterial:
			{
				Ref<PhysicsMaterialAsset> physAsset = rAssetManager.GetAssetAs<PhysicsMaterialAsset>( AssetBundleRegistry, id );

				RawPhysicsMaterialAssetSerialiser serialiser;
				serialiser.DumpAndWriteToVFS( physAsset );
			} break;

			case Saturn::AssetType::Prefab:
			{
				Ref<Prefab> prefabAsset = rAssetManager.GetAssetAs<Prefab>( AssetBundleRegistry, id );

				if( prefabAsset )
				{
					RawPrefabSerialiser serialiser;
					serialiser.DumpAndWriteToVFS( prefabAsset );
				}
			} break;

			case Saturn::AssetType::Scene:
			case Saturn::AssetType::SkeletalMesh:
			case Saturn::AssetType::MaterialInstance:
			case Saturn::AssetType::Sound:
			case Saturn::AssetType::Script:
			case Saturn::AssetType::MeshCollider:
			case Saturn::AssetType::Unknown:
			default:
				break;
		}
	}

	AssetBundleResult AssetBundle::ReadBundle()
	{
		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();
		cachePath /= "AssetBundle.sab";

		if( !std::filesystem::exists( cachePath ) )
			return AssetBundleResult::FileNotFound;

		Timer timer;

		std::ifstream stream( cachePath, std::ios::binary | std::ios::in );

		AssetBundleHeader header{};
		RawSerialisation::ReadObject( header, stream );

		if( strcmp( header.Magic, ".AB\0" ) )
		{
			SAT_CORE_ERROR( "Invalid asset bundle file header or corrupt asset bundle file!" );
			return AssetBundleResult::InvalidFileHeader;
		}

		if( header.Version != SAT_CURRENT_VERSION )
		{
			std::string decodedAssetBundleVer;
			SAT_DECODE_VER_STRING( header.Version, decodedAssetBundleVer );
			
			SAT_CORE_ERROR( "Asset bundle version mismatch! This should not happen. Asset bundle version is: {0} while current Engine version is: {1}.", decodedAssetBundleVer, SAT_CURRENT_VERSION_STRING );
			SAT_CORE_WARN( "The engine will continue to load however this may result in the asset bundle not loading! Please rebuild the asset bundle!");
		}

		AssetManager& rAssetManager = AssetManager::Get();
		Ref<AssetRegistry> rAssetRegistry = rAssetManager.GetAssetRegistry();
		VirtualFS& rVFS = VirtualFS::Get();

		const std::string& rMountBase = Project::GetActiveConfig().Name;

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

				return AssetBundleResult::InvalidPakFileHeader;
			}

			if( rAsset->ID != dfh.Asset )
			{
				SAT_CORE_ERROR( "Asset ID's do not match!" );
			
				return AssetBundleResult::AssetIDMismatch;
			}

			if( dfh.Version != SAT_CURRENT_VERSION ) 
			{
				SAT_CORE_ERROR( "Pack file version mismatch!" );
				// For now continue loading this current file however in future maybe try to upgrade this file to the current version.
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
				
					return AssetBundleResult::FailedToUncompress;
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

		return AssetBundleResult::Success;
	}
}