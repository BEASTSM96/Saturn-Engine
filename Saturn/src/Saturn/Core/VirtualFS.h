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

#pragma once

#include "StringAuxiliary.h"

#include "Memory/Buffer.h"

#include <filesystem>
#include <string>
#include <map>

namespace Saturn {

	struct VFile
	{
		std::string Name;

		Buffer FileContents;
	};

	class VDirectory
	{
	public:
		VDirectory() = default;

		VDirectory( const std::string& rName ) : m_Name( rName ) {}

		VDirectory( const std::wstring& rName )
		{
			m_Name = Auxiliary::ConvertWString( rName );
		}
	public:
		void AddFile( const std::string& rName ) 
		{
			Files.emplace( rName, VFile( rName ) );
		}

		void RemoveFile( const std::string& rName )
		{
			Files.erase( rName );
		}

		void AddDirectory( const std::string& rName )
		{
			Directories.emplace( rName, VDirectory( rName ) );
		}

		void RemoveDirectory( const std::string& rName )
		{
			Directories.erase( rName );
		}

		template<typename Func>
		void EachFile( Func Function ) 
		{
			for( const auto& rFile : Files ) 
			{
				Function( rFile );
			}
		}

		template<typename Func>
		void EachDirectory( Func Function )
		{
			for( const auto& rDir : Directories )
			{
				Function( rDir );
			}
		}

		void Clear() 
		{
			Files.clear();
			Directories.clear();
		}

		const std::string& GetName() { return m_Name; }

	public:
		// Name -> File
		std::unordered_map< std::string, VFile > Files;

		// Name -> VDirectory
		std::unordered_map< std::string, VDirectory > Directories;

	private:
		std::string m_Name;
	};

	class VirtualFS
	{
	public:
		static inline VirtualFS& Get() { return *SingletonStorage::GetOrCreateSingleton<VirtualFS>(); }
	public:
		VirtualFS();
		~VirtualFS();

		// Mounts a real path to a virtual one.
		// For example:
		// ID = FPS, RealPath = D:\Projects\FPS\ would result in:
		// /FPS/
		// 
		// Virtual path is then added to the root directory.
		// Real path is then added to the mount bases list.
		void MountBase( const std::string& rID, const std::filesystem::path& rRealPath );
		
		// Mount a virtual path to a mount base
		// For example:
		// MountBase = FPS
		// VirtualPath = Assets/Meshes/Gun.fbx
		bool Mount( const std::string& rMountBase, const std::filesystem::path& rVirtualPath );

		VFile& FindFile( const std::string& rMountBase, const std::filesystem::path& rVirtualPath );
		VDirectory& FindDirectory( const std::string& rMountBase, const std::filesystem::path& rVirtualPath );

		size_t GetMountBases();
		size_t GetMounts();

	public:
		void ImGuiRender();

	private:
		void Init();
		void Terminate();

		void DrawDirectory( VDirectory& rDirectory );
		size_t GetMountsForDir( VDirectory& rDirectory );

		template<typename Ty>
		Ty* Search( const std::string& rMountBase, const std::filesystem::path& rVirtualPath );

		VDirectory* SearchRecursive( VDirectory& rLastDir, std::filesystem::path::iterator Iterator, const std::filesystem::path::iterator& rEnd );

	private:
		VDirectory m_RootDirectory;

		std::map<std::string, std::filesystem::path> m_MountBases;
		
		// Files and Directories in the ROOT DIR ONLY!
		//std::map<std::wstring, VFile> m_Files;
	};
}