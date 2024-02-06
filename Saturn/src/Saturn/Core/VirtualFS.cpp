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

#include "Saturn/ImGui/ImGuiAuxiliary.h"
#include <imgui.h>

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

	void VirtualFS::Init()
	{
	}

	void VirtualFS::Terminate()
	{
		m_RootDirectory.Clear();
	}

	void VirtualFS::MountBase( const std::string& rID, const std::filesystem::path& rRealPath )
	{
		m_RootDirectory.AddDirectory( rID );
		m_MountBases.emplace( rID, rRealPath );
	}

	bool VirtualFS::Mount( const std::string& rMountBase, const std::filesystem::path& rVirtualPath )
	{
		// Check if the mount base exists.
		const auto Itr = m_MountBases.find( rMountBase );

		if( Itr == m_MountBases.end() )
			return false;

		auto& MountBasePath = Itr->second;

		// Get the mount base folder.
		VDirectory& rMountBaseDir = m_RootDirectory.Directories[ rMountBase ];

		VDirectory* pCurrentDir = &rMountBaseDir;

		for( auto it = rVirtualPath.begin(); it != rVirtualPath.end(); ++it )
		{
			std::filesystem::path temporaryPath = *it;
			std::string segmentStr = temporaryPath.string();

			bool isDir = !temporaryPath.has_extension();

			if( isDir )
			{
				auto dirItr = pCurrentDir->Directories.find( segmentStr );

				if( dirItr == pCurrentDir->Directories.end() )
				{
					// Directory does not exist.
					pCurrentDir->Directories.emplace( segmentStr, VDirectory( segmentStr ) );
				}

				pCurrentDir = &pCurrentDir->Directories[ segmentStr ];
			}
			else
			{
				auto fileItr = pCurrentDir->Files.find( segmentStr );

				if( fileItr == pCurrentDir->Files.end() )
				{
					// Directory does not exist.
					pCurrentDir->Files.emplace( segmentStr, VFile( segmentStr ) );
				}
			}
		}

		// Mount successful.
		return true;
	}

	size_t VirtualFS::GetMountBases()
	{
		return m_MountBases.size();
	}

	size_t VirtualFS::GetMounts()
	{
		size_t mounts = 0;

		for( auto&& [name, rDir] : m_RootDirectory.Directories )
			mounts += GetMountsForDir( rDir );

		return mounts;
	}

	size_t VirtualFS::GetMountsForDir( VDirectory& rDirectory )
	{
		size_t mounts = 0;

		for( auto&& [name, rDir] : rDirectory.Directories )
		{
			mounts += rDir.Files.size();
			mounts += rDirectory.Directories.size();

			mounts += GetMountsForDir( rDir );
		}

		return mounts;
	}

	void VirtualFS::DrawDirectory( VDirectory& rDirectory )
	{		
		if( ImGui::TreeNode( rDirectory.GetName().c_str() ) )
		{
			for( auto&& [name, rDirectory] : rDirectory.Directories )
			{
				DrawDirectory( rDirectory );
			}

			for( auto&& [name, rFile] : rDirectory.Files )
			{
				ImGui::Selectable( name.c_str() );
			}

			ImGui::TreePop();
		}
	}

	void VirtualFS::ImGuiRender()
	{
		if( Auxiliary::TreeNode( "Mount Bases", false ) ) 
		{
			for( auto&& [name, rPath] : m_MountBases )
			{
				if( ImGui::TreeNode( name.c_str() ) )
				{
					ImGui::Text( "ID: %s", name.c_str() );
					ImGui::Text( "Path: %s", rPath.string().c_str() );

					ImGui::TreePop();
				}
			}

			Auxiliary::EndTreeNode();
		}

		if( Auxiliary::TreeNode( "Mounts", false ) )
		{
			for( auto&& [name, rDirectory] : m_RootDirectory.Directories )
			{
				DrawDirectory( rDirectory );
			}

			Auxiliary::EndTreeNode();
		}
	}
}