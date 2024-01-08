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

#include "Saturn/ImGui/Panel/Panel.h"

#include "Saturn/Vulkan/Texture.h"

#include "Saturn/Asset/Asset.h"

#include "ContentBrowserItem.h"

#include "Saturn/GameFramework/SClass.h"

#include <imgui.h>
#include <filesystem>
#include <functional>
#include <map>

#include <filewatch/filewatch.h>

namespace Saturn {
	
	enum class CBViewMode 
	{
		Assets,
		Scripts
	};

	struct SClassMetadata;

	class ContentBrowserPanel : public Panel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel();

		virtual void Draw() override;

		void ResetPath( const std::filesystem::path& rPath );

	private:	
		void OnItemSelected( ContentBrowserItem* pItem, bool clicked );

		void UpdateFiles( bool clear = false );

		void DrawFolderTree( const std::filesystem::path& rPath );

		void DrawAssetsFolderTree();
		void DrawScriptsFolderTree();

		void DrawRootFolder( CBViewMode type, bool open = false );

		void AssetsPopupContextMenu();
		void ScriptsPopupContextMenu();

		void OnFilewatchEvent( const std::string& rPath, const filewatch::Event Event );
		void FindAndRenameItem( const std::filesystem::path& rPath );
		Ref<ContentBrowserItem> FindItem( const std::filesystem::path& rPath );

		Ref<ContentBrowserItem> GetActiveHoveredItem();

		int32_t GetFilenameCount( const std::string& rName );

		void DrawClassHierarchy( const std::string& rKeyName, const SClassMetadata& rData );
		void DrawCreateClass( const std::string& rKeyName, const SClassMetadata& rData );

		void ClearSelected();

	private: // Editor Content
		void EdDrawRootFolder( CBViewMode type, bool open = false );
		void EdDrawAssetsFolderTree();
		void EdSetPath();

		void DrawTopBar();

	private:
		std::filesystem::path m_CurrentPath;
		std::filesystem::path m_FirstFolder;
		std::filesystem::path m_ScriptPath;

		std::filesystem::path m_EditorContent;
		std::filesystem::path m_EditorScripts;

		std::string m_SearchText = "";
		ImGuiTextFilter m_TextFilter;
		bool m_Searching = false;

		Ref< Texture2D > m_DirectoryIcon;
		Ref< Texture2D > m_FileIcon;
		Ref< Texture2D > m_BackIcon;
		Ref< Texture2D > m_ForwardIcon;

		std::vector<Ref<ContentBrowserItem>> m_SelectedItems;

		bool m_ShowFolderPopupMenu = false;

		struct AssetInfo
		{
			AssetType Type;
			std::filesystem::path Path;
		};

		CBViewMode m_ViewMode;

		// Files and folder, sorted by folders then files.
		std::vector<Ref<ContentBrowserItem>> m_Files;
		std::vector<Ref<ContentBrowserItem>> m_ValidSearchFiles;

		bool m_FilesNeedSorting = false;
		bool m_RenderCreateWindow = false;
		bool m_ChangeDirectory = false;

		SClassMetadata m_SelectedMetadata = {};

		filewatch::FileWatch<std::string>* m_Watcher = nullptr;

	private:
		// Popup data

		bool m_ShowMeshImport = false;
		bool m_ShowSoundImport = false;
		
		std::filesystem::path m_ImportMeshPath;
		std::filesystem::path m_ImportSoundPath;
		
		std::string m_ClassInstanceName;
		std::string m_NewClassName;
	};
}
