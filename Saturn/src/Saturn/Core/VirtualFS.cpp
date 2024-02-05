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
#include "VirtualFS.h"

namespace Saturn {

	VirtualFS::VirtualFS()
		: m_RootDirectory( "/" )
	{
		Init();
	}

	VirtualFS::~VirtualFS()
	{
		Terminate();
	}

	void VirtualFS::MountBase( const std::string& rID, const std::filesystem::path& rRealPath )
	{
		std::wstring rIDW = Auxiliary::ConvertString( rID );

		m_RootDirectory.AddDirectory( rIDW );
		m_MountBases.emplace( rID, rRealPath );
	}

	bool VirtualFS::Mount( const std::string& rMountBase, const std::filesystem::path& rVirtualPath )
	{
		// Check if the mount base exists.
		const auto Itr = m_MountBases.find( rMountBase );

		if( Itr == m_MountBases.end() )
			return false;

		auto& MountBasePath = Itr->second;
		std::wstring MouseBasePathW = Auxiliary::ConvertString( rMountBase );

		// Get the mount base folder.
		VDirectory& rMountBaseDir = m_RootDirectory.Directories[ MouseBasePathW ];

		VDirectory CurrentDir = rMountBaseDir;

		for( auto it = rVirtualPath.begin(); it != rVirtualPath.end(); ++it )
		{
			std::wstring segmentStr = *it;
			std::filesystem::path temporaryPath = *it;

			bool isDir = !temporaryPath.has_extension();

			if( isDir )
			{
				auto dirItr = CurrentDir.Directories.find( segmentStr );

				if( dirItr == CurrentDir.Directories.end() )
				{
					// Directory does not exist.
					CurrentDir.Directories.emplace( segmentStr, VDirectory( segmentStr ) );
				}
			}
			else
			{
				auto fileItr = CurrentDir.Files.find( segmentStr );

				if( fileItr == CurrentDir.Files.end() )
				{
					// Directory does not exist.
					CurrentDir.Files.emplace( segmentStr, VFile( segmentStr ) );
				}
			}

			//CurrentDir = CurrentDir.Directories[ SegmentStr ];
		}

		// Mount successful.
		return true;
	}

	void VirtualFS::Init()
	{
	}

	void VirtualFS::Terminate()
	{
		m_RootDirectory.Clear();
	}

}