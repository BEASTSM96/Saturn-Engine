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
#include "VDirectory.h"

#include "Saturn/Serialisation/RawSerialisation.h"

#include "VFile.h"

namespace Saturn {

	VDirectory::VDirectory( const std::wstring& rName )
	{
		m_Name = Auxiliary::ConvertWString( rName );
	}

	VDirectory::VDirectory( const std::string& rName ) 
		: m_Name( rName )
	{
	}

	VDirectory::VDirectory( const std::string& rName, VDirectory* parentDirectory )
		: m_Name( rName ), m_ParentDirectory( parentDirectory )
	{

	}

	void VDirectory::AddFile( const std::string& rName )
	{
		Files.emplace( rName, VFile( rName ) );
	}

	void VDirectory::RemoveFile( const std::string& rName )
	{
		Files.erase( rName );
	}

	void VDirectory::AddDirectory( const std::string& rName )
	{
		Directories.emplace( rName, VDirectory( rName ) );
	}

	void VDirectory::RemoveDirectory( const std::string& rName )
	{
		Directories.erase( rName );
	}

	void VDirectory::Clear()
	{
		Files.clear();

		for( auto&& [k, dir] : Directories )
		{
			dir.Clear();
		}

		Directories.clear();
	}

	void VDirectory::Serialise( const VDirectory& rObject, std::ofstream& rStream )
	{
		RawSerialisation::WriteString( rObject.m_Name, rStream );

		RawSerialisation::WriteUnorderedMap( rObject.Files, rStream );
		RawSerialisation::WriteUnorderedMap( rObject.Directories, rStream );
	}

	void VDirectory::Deserialise( VDirectory& rObject, std::ifstream& rStream )
	{
		rObject.m_Name = RawSerialisation::ReadString( rStream );

		RawSerialisation::ReadUnorderedMap( rObject.Files, rStream );
		RawSerialisation::ReadUnorderedMap( rObject.Directories, rStream );
	}
}