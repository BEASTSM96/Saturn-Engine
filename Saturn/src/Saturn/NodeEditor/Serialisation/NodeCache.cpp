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
#include "NodeCache.h"

namespace Saturn {

	struct SettingsFileHeader
	{
		char Magic[ 5 ] = { '.', 'N', 'C', 'S', '\0' };
		size_t SettingsCount = 0;
		uint64_t Version = SAT_CURRENT_VERSION;
	};
	
	// Header for each node editor
	struct NodeCacheSettingsHeader
	{
		char Magic[ 5 ] = { 'x', 'N', 'C', 'S', '\0' };
		AssetID ID = 0;
		uint64_t Version = SAT_CURRENT_VERSION;
	};

	bool NodeCacheSettings::WriteEditorSettings( Ref<NodeEditorBase> rNodeEditor )
	{
		std::filesystem::path filepath = Project::GetActiveProject()->GetAppDataFolder();

		if( !std::filesystem::exists( filepath ) )
			std::filesystem::create_directories( filepath );

		filepath /= "NodeCache-Settings";
		filepath.replace_extension( ".ncs" );

		if( CanAppendFile( filepath ) )
			AppendFile( filepath, rNodeEditor );
		else
			OverrideFile( filepath, rNodeEditor );

		return true;
	}

	bool NodeCacheSettings::CanAppendFile( const std::filesystem::path& rFilepath )
	{
		std::ifstream stream( rFilepath, std::ios::binary | std::ios::in | std::ios::ate );

		auto end = stream.tellg();
		stream.seekg( 0, std::ios::beg );
		auto size = end - stream.tellg();

		stream.close();

		return size != 0;
	}

	void NodeCacheSettings::OverrideFile( const std::filesystem::path& rFilepath, Ref<NodeEditorBase> rNodeEditor )
	{
		SettingsFileHeader fileHeader;
		fileHeader.SettingsCount++;

		std::ofstream fout( rFilepath, std::ios::binary | std::ios::trunc );

		std::map< uint64_t, std::string > stateMap;
		stateMap[ rNodeEditor->GetAssetID() ] = rNodeEditor->m_ActiveNodeEditorState;

		RawSerialisation::WriteObject( fileHeader, fout );
		RawSerialisation::WriteMap( stateMap, fout );

		fout.close();
	}

	void NodeCacheSettings::AppendFile( const std::filesystem::path& rFilepath, Ref<NodeEditorBase> rNodeEditor )
	{
		std::ifstream stream( rFilepath, std::ios::binary | std::ios::in );

		SettingsFileHeader fileHeader{};
		RawSerialisation::ReadObject( fileHeader, stream );

		if( strcmp( fileHeader.Magic, ".NCS\0" ) )
		{
			OverrideFile( rFilepath, rNodeEditor );
			return;
		}

		// Get all currently saved states
		std::map<uint64_t, std::string> stateMap;
		
		RawSerialisation::ReadMap( stateMap, stream );

		stream.close();

		const auto Itr = stateMap.find( rNodeEditor->GetAssetID() );

		// New node editor is being cached so update file header
		if( Itr == stateMap.end() )
		{
			fileHeader.SettingsCount++;

			stateMap[ rNodeEditor->GetAssetID() ] = rNodeEditor->m_ActiveNodeEditorState;
		}
		else
		{
			Itr->second = rNodeEditor->m_ActiveNodeEditorState;
		}

		std::ofstream fout( rFilepath, std::ios::binary | std::ios::trunc );

		RawSerialisation::WriteObject( fileHeader, fout );

		RawSerialisation::WriteMap( stateMap, fout );

		fout.close();
	}
}