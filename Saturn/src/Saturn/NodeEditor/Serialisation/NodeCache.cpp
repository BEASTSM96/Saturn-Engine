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

	bool NodeCacheSettings::WriteEditorSettings( Ref<NodeEditor>& rNodeEditor )
	{
		std::filesystem::path filepath = Project::GetActiveProject()->GetAppDataFolder();

		if( !std::filesystem::exists( filepath ) )
			std::filesystem::create_directories( filepath );

		Buffer fileBuffer = TryReadFileForWriting( filepath );

		if( fileBuffer.Size == 0 )
			OverrideFile( filepath, rNodeEditor );
		else
			AppendFile( fileBuffer, filepath, rNodeEditor );

		fileBuffer.Free();

		return true;
	}

	Buffer NodeCacheSettings::TryReadFileForWriting( const std::filesystem::path& rFilepath )
	{
		Buffer fileBuffer;
	
		std::ifstream stream( rFilepath, std::ios::binary | std::ios::in | std::ios::ate );

		auto end = stream.tellg();
		stream.seekg( 0, std::ios::beg );
		auto size = end - stream.tellg();

		if( size == 0 )
			return fileBuffer;

		fileBuffer.Allocate( static_cast< size_t >( size ) );
		stream.read( reinterpret_cast< char* >( fileBuffer.Data ), fileBuffer.Size );

		stream.close();

		return fileBuffer;
	}

	void NodeCacheSettings::OverrideFile( const std::filesystem::path& rFilepath, Ref<NodeEditor>& rNodeEditor )
	{
		SettingsFileHeader fileHeader;
		fileHeader.SettingsCount++;

		std::ofstream fout( rFilepath, std::ios::binary | std::ios::trunc );

		RawSerialisation::WriteObject( fileHeader, fout );
		RawSerialisation::WriteString( rNodeEditor->m_ActiveNodeEditorState, fout );

		fout.close();
	}

	void NodeCacheSettings::AppendFile( Buffer& rBuffer, const std::filesystem::path& rFilepath, Ref<NodeEditor>& rNodeEditor )
	{
		SettingsFileHeader fileHeader = *( SettingsFileHeader* ) rBuffer.Data;

		if( strcmp( fileHeader.Magic, ".NCS\0" ) )
		{
			OverrideFile( rFilepath, rNodeEditor );
			return;
		}

		uint8_t* stateData = rBuffer.As<uint8_t>() + sizeof( SettingsFileHeader );

		// Get all currently saved states
		std::vector<std::string> states( fileHeader.SettingsCount );

		for( size_t i = 0; i < fileHeader.SettingsCount; i++ )
		{
			std::string state = *( std::string* ) stateData;

			states[ i ] = state;

			stateData += state.size();
		}

		fileHeader.SettingsCount++;

		std::ofstream fout( rFilepath, std::ios::binary | std::ios::trunc );

		RawSerialisation::WriteObject( fileHeader, fout );

		size_t mapSize = states.size();
		fout.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

		for( const auto& value : states )
		{
			RawSerialisation::WriteString( value, fout );
		}

		RawSerialisation::WriteString( rNodeEditor->m_ActiveNodeEditorState, fout );

		fout.close();
	}
}