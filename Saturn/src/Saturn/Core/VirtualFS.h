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

#include "VFile.h"

#include "Memory/Buffer.h"

#include <filesystem>
#include <string>
#include <map>

namespace Saturn {

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

		void UnmountBase( const std::string& rID );
		void Unmount( const std::string& rMountBase, const std::filesystem::path& rVirtualPath );

		VFile FindFile( const std::string& rMountBase, const std::filesystem::path& rVirtualPath );
		VDirectory FindDirectory( const std::string& rMountBase, const std::filesystem::path& rVirtualPath );

		size_t GetMountBases();
		size_t GetMounts();

	public:
		void ImGuiRender();

	private:
		void Init();
		void Terminate();

		void BuildPath( VDirectory& rDir, const std::string& rMountBase );

		void DrawDirectory( VDirectory& rDirectory );
		size_t GetMountsForDir( VDirectory& rDirectory );

		VFile SearchRecursiveFile( VDirectory& rLastDir, std::filesystem::path::iterator Iterator, const std::filesystem::path::iterator& rEnd, const std::string& rTargetName = "" );

		VDirectory SearchRecursiveDir( VDirectory& rLastDir, std::filesystem::path::iterator Iterator, const std::filesystem::path::iterator& rEnd );

	private:
		VDirectory m_RootDirectory;

		std::map<std::string, std::filesystem::path> m_MountBases;
		
		// Mount Base -> Path -> Dir
		std::map<std::string, std::map<std::filesystem::path, VDirectory>> m_PathToDir;
	};
}