/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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

#include "Panel/Panel.h"

#include "Saturn/Vulkan/Texture.h"

#include "Saturn/Asset/Asset.h"

#include <imgui.h>
#include <filesystem>
#include <functional>

#include <filewatch/filewatch.h>

namespace Saturn {
	
	enum class CBViewMode 
	{
		Assets,
		Scripts
	};

	class ContentBrowserPanel : public Panel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		virtual void Draw() override;

		void SetPath( const std::filesystem::path& rPath );

	private:

		void RenderEntry( const std::filesystem::directory_entry& rEntry, ImVec2 ThumbnailSize, float Padding, bool excludeFiles = true );
				
		void OnDirectorySelected( std::filesystem::path& rPath, bool IsFile = false );

		void UpdateFiles( bool clear = false );

		void DrawFolderTree( const std::filesystem::path& rPath );

		void DrawAssetsFolderTree();
		void DrawScriptsFolderTree();

		void DrawRootFolder( CBViewMode type, bool open = false );

	private:
		std::filesystem::path m_CurrentPath;
		std::filesystem::path m_FirstFolder;
		std::filesystem::path m_ScriptPath;

		Ref< Texture2D > m_DirectoryIcon;
		Ref< Texture2D > m_FileIcon;
		Ref< Texture2D > m_SwapViewIcon;
		Ref< Texture2D > m_BackIcon;
		Ref< Texture2D > m_ForwardIcon;

		bool m_ShowMeshImport = false;
		bool m_ShowSoundImport = false;
		std::filesystem::path m_ImportMeshPath;
		std::filesystem::path m_ImportSoundPath;

		struct AssetInfo
		{
			AssetType Type;
			std::filesystem::path Path;
		};

		CBViewMode m_ViewMode;

		// Files and folder, sorted by folders then files.
		std::vector<std::filesystem::directory_entry> m_Files;

		bool m_FilesNeedSorting = false;
		bool m_RenderCreateWindow = false;
		bool m_ChangeDirectory = false;

		filewatch::FileWatch<std::string>* m_Watcher = nullptr;
	};
}
