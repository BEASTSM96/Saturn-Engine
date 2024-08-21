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

#include "ContentBrowserBase.h"

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

	struct SClassMetadata;

	class ContentBrowserPanel : public ContentBrowserBase
	{
	public:
		ContentBrowserPanel();
		ContentBrowserPanel( const std::string& rName );

		virtual ~ContentBrowserPanel();

		virtual void Draw() override;
		static const char* GetStaticName()
		{
			return "Content Browser Panel";
		}

		virtual void ResetPath( const std::filesystem::path& rPath ) override;

	private:
		virtual void UpdateFiles( bool clear = false ) override;
		virtual void OnItemSelected( ContentBrowserItem* pItem, bool clicked ) override;
		virtual void DrawItems( std::vector<Ref<ContentBrowserItem>>& rList, ImVec2 size, float padding ) override;

		void BuildSearchList();

		void DrawFolderTree( const std::filesystem::path& rPath );

		void DrawAssetsFolderTree();
		void DrawScriptsFolderTree();

		void DrawRootFolder( CBViewMode type, bool open = false );

		void AssetsPopupContextMenu();
		void ScriptsPopupContextMenu();

		void OnFilewatchEvent( const std::string& rPath, const filewatch::Event Event );

		Ref<ContentBrowserItem> GetActiveHoveredItem();

		void DrawClassHierarchy( const std::string& rKeyName, const SClassMetadata& rData );
		void DrawCreateClass( const std::string& rKeyName, const SClassMetadata& rData );

		void ClearSearchQuery();
		void DrawImportSoundPopup();
		void DrawImportMeshPopup();

	private:
		std::filesystem::path m_ScriptPath;

		ImGuiTextFilter m_TextFilter;

		bool m_ShowFolderPopupMenu = false;

		struct AssetInfo
		{
			AssetType Type;
			std::filesystem::path Path;
		};

		bool m_RenderCreateWindow = false;

		SClassMetadata m_SelectedMetadata = {};

		filewatch::FileWatch<std::string>* m_Watcher = nullptr;

	private:
		bool m_ShowAssetImportPopup = false;
		AssetType m_AssetImportType = AssetType::Unknown;
		std::filesystem::path m_ImportAssetPath;
		
		// Popup data
		std::string m_ClassInstanceName;
		std::string m_NewClassName;

		bool m_OpenScriptsPopup = false;
		bool m_OpenClassInstancePopup = false;
	};
}
