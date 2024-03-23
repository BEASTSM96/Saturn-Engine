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
#include "NodeEditorCache.h"

#include "NodeEditor.h"

#include "Saturn/Project/Project.h"

namespace Saturn {

	static void CreateDirIfNeeded()
	{
		std::filesystem::path dir = Project::GetActiveProject()->GetFullCachePath();
		dir /= "NodeEdCache";

		if( !std::filesystem::exists( dir ) )
			std::filesystem::create_directories( dir );
	}

	static std::filesystem::path GetNodeEdCacheDir() 
	{
		std::filesystem::path dir = Project::GetActiveProject()->GetFullCachePath();
		dir /= "NodeEdCache";

		return dir;
	}

	struct NodeEditorCacheHeader
	{
		const char Magic[ 5 ] = ".NC\0";
		AssetID AssetID;
		uint32_t Version = SAT_CURRENT_VERSION;
	};

	void NodeEditorCache::WriteNodeEditorCache( Ref<NodeEditor>& NodeEditor )
	{
		std::string filename = std::format( "NodeEdCache-{0}.snodec", (uint64_t)NodeEditor->GetAssetID() );

		std::filesystem::path targetDir = GetNodeEdCacheDir();
		targetDir /= filename;

		CreateDirIfNeeded();

		std::ofstream fout( targetDir, std::ios::binary | std::ios::trunc );

		NodeEditorCacheHeader header{};
		header.AssetID = NodeEditor->GetAssetID();

		RawSerialisation::WriteObject( header, fout );

		NodeEditor->SerialiseData( fout );

		fout.close();
	}

	bool NodeEditorCache::ReadNodeEditorCache( Ref<NodeEditor>& NodeEditor, AssetID id )
	{
		std::string filename = std::format( "NodeEdCache-{0}.snodec", ( uint64_t ) id );
		
		std::filesystem::path targetDir = GetNodeEdCacheDir();
		targetDir /= filename;

		if( !std::filesystem::exists( targetDir ) )
			return false;

		std::ifstream stream( targetDir, std::ios::binary | std::ios::in );

		NodeEditorCacheHeader header{};
		RawSerialisation::ReadObject( header, stream );

		if( strcmp( header.Magic, ".NC\0" ) )
		{
			SAT_CORE_ERROR( "Invalid node editor cache file header or corrupt cache file!" );
			return false;
		}

		if( header.Version != SAT_CURRENT_VERSION )
		{
			std::string decodedAssetBundleVer;
			SAT_DECODE_VER_STRING( header.Version, decodedAssetBundleVer );

			SAT_CORE_ERROR( "Node Editor Cache version mismatch! This should not happen. Cache file version is: {0} while current engine version is: {1}.", decodedAssetBundleVer, SAT_CURRENT_VERSION_STRING );
			SAT_CORE_WARN( "The engine will continue to load however this may result in the cache file not loading!" );
		}

		if( header.AssetID != id )
		{
			SAT_CORE_ERROR( "Node editor cache file asset id mismatch! Saved ID was: {0} however ID passed in was {1}", header.AssetID, id );
			return false;
		}

		NodeEditor->m_AssetID = id;
		NodeEditor->DeserialiseData( stream );

		stream.close();

		return true;
	}

	bool NodeEditorCache::DoesCacheExist( AssetID id )
	{
		std::string filename = std::format( "NodeEdCache-{0}.snodec", ( uint64_t ) id );

		std::filesystem::path targetDir = GetNodeEdCacheDir();
		targetDir /= filename;

		return std::filesystem::exists( targetDir );
	}

}