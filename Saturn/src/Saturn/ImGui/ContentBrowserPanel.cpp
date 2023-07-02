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

#include "sppch.h"
#include "ContentBrowserPanel.h"

#include "ImGuiAuxiliary.h"
#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Asset/PhysicsMaterialAsset.h"
#include "Saturn/Serialisation/AssetSerialisers.h"
#include "Saturn/Asset/AssetImporter.h"

#include "AssetViewer.h"
#include "PrefabViewer.h"
#include "StaticMeshAssetViewer.h"
#include "MaterialAssetViewer.h"
#include "PhysicsMaterialAssetViewer.h"

#include "Saturn/Project/Project.h"
#include "Saturn/Core/App.h"
#include "Saturn/Asset/AssetRegistry.h"
#include "Saturn/Vulkan/Mesh.h"

#include "Saturn/Serialisation/GameAssetRegistrySerialiser.h"

#include "Saturn/Asset/Prefab.h"
#include "Saturn/Audio/Sound2D.h"

#include "Saturn/Premake/Premake.h"
#include "Saturn/GameFramework/SourceManager.h"
#include "Saturn/GameFramework/GamePrefabList.h"
#include "Saturn/GameFramework/EntityScriptManager.h"

#include <imgui_internal.h>

namespace Saturn {
	
	static std::filesystem::path s_pAssetsDirectory = "Assets";
	static std::filesystem::path s_pScriptsDirectory = "Scripts";

	static std::filesystem::path s_pMainDirectory = "Scripts";

	static bool s_OpenScriptsPopup = false;
	
	ContentBrowserPanel::ContentBrowserPanel()
		: Panel( "Content Browser Panel" ), m_CurrentPath( s_pAssetsDirectory ), m_FirstFolder( s_pAssetsDirectory ), m_ScriptPath( s_pScriptsDirectory )
	{
		m_DirectoryIcon = Ref<Texture2D>::Create( "content/textures/editor/DirectoryIcon.png", AddressingMode::Repeat );
		m_FileIcon      = Ref<Texture2D>::Create( "content/textures/editor/FileIcon.png",      AddressingMode::Repeat );
		m_SwapViewIcon  = Ref<Texture2D>::Create( "content/textures/editor/Swap.png",          AddressingMode::Repeat );
		m_BackIcon      = Ref<Texture2D>::Create( "content/textures/editor/Left.png",          AddressingMode::Repeat );
		m_ForwardIcon   = Ref<Texture2D>::Create( "content/textures/editor/Right.png",         AddressingMode::Repeat );

		m_ViewMode      = CBViewMode::Assets;
	}

	void ContentBrowserPanel::DrawFolderTree( const std::filesystem::path& rPath )
	{
		for( const auto& entry : std::filesystem::directory_iterator( rPath ) )
		{
			if( !entry.is_directory() )
				continue;

			const std::filesystem::path& entryPath = entry.path();
			const std::string entryName = entryPath.filename().string();

			if( ImGui::TreeNode( entryName.c_str() ) )
			{
				DrawFolderTree( entryPath );

				ImGui::TreePop();
			}
		}
	}
	
	void ContentBrowserPanel::Draw()
	{
		ImGui::Begin( "Content Browser" );

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::BeginChild( "Top Bar", ImVec2( 0, 30 ), false, flags );
		
		if( Auxiliary::ImageButton( m_SwapViewIcon, { 24, 24 } ) ) 
		{
		}

		ImGui::SameLine();

		// Back button.
		if( m_CurrentPath != s_pAssetsDirectory )
		{
			if( Auxiliary::ImageButton( m_BackIcon, { 24, 24 } ) )
			{
				m_CurrentPath = m_CurrentPath.parent_path();

				UpdateFiles( true );
			}
		}
		else
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
			Auxiliary::ImageButton( m_BackIcon, { 24, 24 } );
			ImGui::PopStyleVar( 1 );
			ImGui::PopItemFlag();
		}

		ImGui::SameLine();

		// Forward button.
		if( std::filesystem::exists( m_FirstFolder ) )
		{
			if( Auxiliary::ImageButton( m_ForwardIcon, { 24, 24 } ) )
			{
				m_CurrentPath /= std::filesystem::relative( m_FirstFolder, s_pMainDirectory );

				UpdateFiles( true );
			}
		}
		else
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
			Auxiliary::ImageButton( m_ForwardIcon, { 24, 24 } );
			ImGui::PopStyleVar( 1 );
			ImGui::PopItemFlag();
		}

		ImGui::EndChild();

		if( m_ChangeDirectory )
		{
			UpdateFiles( true );

			m_ChangeDirectory = false;
		}

		ImGui::BeginChild( "Folder Tree", ImVec2( 200, 0 ), false );

		if( Auxiliary::TreeNode( "Game" ) )
		{
			DrawFolderTree( s_pMainDirectory );
			
			Auxiliary::EndTreeNode();
		}

		if( Auxiliary::TreeNode( "Editor", false ) )
		{
			Auxiliary::EndTreeNode();
		}

		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild( "Folder Contents", ImVec2( 0, 0 ), false );

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.3f, 0.3f, 0.3f, 0.35f ) );

		static float padding = 16.0f;
		static float thumbnailSizeX = 180;
		static float thumbnailSizeY = 180;
		float cellSize = thumbnailSizeX + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x - 20.0f + ImGui::GetStyle().ScrollbarSize;

		int columnCount = ( int ) ( panelWidth / cellSize );
		if( columnCount < 1 ) columnCount = 1;

		ImGui::Columns( columnCount, 0, false );

		for( auto& rEntry : m_Files )
			RenderEntry( rEntry, { thumbnailSizeX, thumbnailSizeY }, padding );

		// Get the first folder in the current directory.
		m_FirstFolder = "";
		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
		{
			if( rEntry.is_directory() )
			{
				m_FirstFolder = rEntry.path();
				break;
			}
		}

		ImGui::Columns( 1 );

		ImGui::PopStyleColor( 2 );

		ImGui::EndChild();

		ImGui::PopStyleColor();

		ImGui::End();
	}

	void ContentBrowserPanel::SetPath( const std::filesystem::path& rPath )
	{
		switch( m_ViewMode )
		{
			case Saturn::CBViewMode::Assets: 
			{
				s_pAssetsDirectory = rPath / "Assets";

				m_CurrentPath = rPath / "Assets";
				m_FirstFolder = rPath / "Assets";

				s_pMainDirectory = s_pAssetsDirectory;
			} break;

			case Saturn::CBViewMode::Scripts: 
			{
				s_pScriptsDirectory = rPath / "Scripts";

				m_CurrentPath = rPath / "Scripts";
				m_FirstFolder = rPath / "Scripts";

				s_pMainDirectory = s_pScriptsDirectory;
			} break;
		}

		UpdateFiles( true );
	}

	void ContentBrowserPanel::SwapViewMode( CBViewMode newMode )
	{
		m_ViewMode = newMode;
	}

	void ContentBrowserPanel::RenderEntry( const std::filesystem::directory_entry& rEntry, ImVec2 ThumbnailSize, float Padding, bool excludeFiles /*= true */ )
	{
		//if( !rEntry.is_directory() && excludeFiles || rEntry.is_directory() && !excludeFiles )
		//	return;

		auto* pDrawList = ImGui::GetWindowDrawList();

		//auto RelativePath = std::filesystem::relative( rEntry.path(), s_pAssetsDirectory );
		auto path = rEntry.path().string();

		if( rEntry.path().extension() == ".sreg" )
			return;

		std::string filename = rEntry.path().filename().string();

		bool isFile = !rEntry.is_directory();

		Ref<Texture2D> Icon = isFile ? m_FileIcon : m_DirectoryIcon;

		// Draw background.
		const float EdgeOffset = 4.0f;
		const float TextLineHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0f + EdgeOffset * 2.0f;
		const float InfoPanelHeight = std::max( ThumbnailSize.x * 0.5f, TextLineHeight );
		const ImVec2 TopLeft = ImGui::GetCursorScreenPos();
		const ImVec2 ThumbnailBottomRight = ImVec2( TopLeft.x + ThumbnailSize.x, TopLeft.y + ThumbnailSize.y );
		const ImVec2 InfoTopLeft = ImVec2( TopLeft.x, TopLeft.y + ThumbnailSize.y );
		const ImVec2 BottomRight = ImVec2( TopLeft.x + ThumbnailSize.x, TopLeft.y + ThumbnailSize.y + InfoPanelHeight );
		
		ImGui::PushID( path.c_str() );
		ImGui::BeginGroup();

		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

		if( rEntry.is_directory() )
		{
			bool Hovered = false;
			bool Clicked = false;

			ImGui::ButtonBehavior( ImRect( TopLeft, BottomRight ), ImGui::GetID( path.c_str() ), &Hovered, &Clicked );

			pDrawList->AddRectFilled( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_Button ), 5.0f, ImDrawCornerFlags_All );

			ImGuiStyle& style = ImGui::GetStyle();

			ImGui::ItemSize( ThumbnailSize, style.FramePadding.y );
			ImGui::ItemAdd( ImRect( TopLeft, BottomRight ), ImGui::GetID( path.c_str() ) );

			if( Hovered )
			{
				// Draw a highlight on the button.
				pDrawList->AddRect( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_ButtonHovered ), 5.0f, ImDrawCornerFlags_All );

				if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
				{
					m_CurrentPath /= rEntry.path().filename();
					OnDirectorySelected( m_CurrentPath, rEntry.is_directory() );
				}
			}
		}
		else
		{
			ImGuiStyle& style = ImGui::GetStyle();

			// Fill background.
			pDrawList->AddRectFilled( TopLeft, ThumbnailBottomRight, ImGui::GetColorU32( ImGuiCol_Button ), 5.0f, ImDrawCornerFlags_Top );

			// Fill Info area
			pDrawList->AddRectFilled( InfoTopLeft, BottomRight, IM_COL32( 47, 47, 47, 255 ), 5.0f, ImDrawCornerFlags_Bot );

			// Draw line between thumbnail and info.
			pDrawList->AddLine( ThumbnailBottomRight, InfoTopLeft, IM_COL32( 255, 0, 0, 255 ), 1.5f );

			ImGui::ItemSize( ImRect( TopLeft, BottomRight ).Min, style.FramePadding.y );
			ImGui::ItemAdd( ImRect( TopLeft, BottomRight ), ImGui::GetID( path.c_str() ) );

			bool ItemClicked = false;
			ItemClicked = Auxiliary::ButtonRd( "##CONTENT_BROWSER_ITEM_BTN", ImRect( TopLeft, BottomRight ), true );

			//if( !excludeFiles )
			{
				auto assetType = AssetTypeFromExtension( rEntry.path().filename().extension().string() );

				if( ImGui::BeginDragDropSource( ImGuiDragDropFlags_SourceAllowNullID ) )
				{
					auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );
					const wchar_t* c = path.c_str();

					switch( assetType )
					{
						case Saturn::AssetType::Texture:
							break;
						case Saturn::AssetType::StaticMesh:
						{
							ImGui::SetDragDropPayload( "CONTENT_BROWSER_ITEM_MODEL", c, ( wcslen( c ) + 1 ) * sizeof( wchar_t ), ImGuiCond_Once );
						}	break;
						case Saturn::AssetType::SkeletalMesh:
						case Saturn::AssetType::Material:
						{
							ImGui::SetDragDropPayload( "asset_playload", c, ( wcslen( c ) + 1 ) * sizeof( wchar_t ), ImGuiCond_Once );
						}	break;
						case Saturn::AssetType::MaterialInstance:
						case Saturn::AssetType::Audio:
							break;
						case Saturn::AssetType::Scene:
						{
							ImGui::SetDragDropPayload( "CONTENT_BROWSER_ITEM_SCENE", c, ( wcslen( c ) + 1 ) * sizeof( wchar_t ), ImGuiCond_Once );
						} break;

						case Saturn::AssetType::Prefab: 
						{	
							ImGui::SetDragDropPayload( "CONTENT_BROWSER_ITEM_PREFAB", c, ( wcslen( c ) + 1 ) * sizeof( wchar_t ), ImGuiCond_Once );
						} break;

						case Saturn::AssetType::Script: 
						{	
							ImGui::SetDragDropPayload( "CONTENT_BROWSER_ITEM_SCRIPT", c, ( wcslen( c ) + 1 ) * sizeof( wchar_t ), ImGuiCond_Once );
						} break;

						case Saturn::AssetType::Unknown:
						case Saturn::AssetType::COUNT:
						default:
							break;
					}

					ImGui::EndDragDropSource();
				}

				if( ItemClicked )
				{
					ItemClicked = false;

					switch( assetType )
					{
						case Saturn::AssetType::Texture:
							break;

						case Saturn::AssetType::StaticMesh:
						{
							auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );

							// Find the asset.
							Ref<Asset> asset = AssetRegistry::Get().FindAsset( path );
							AssetViewer::Add<StaticMeshAssetViewer>( asset->ID );
						} break;

						case Saturn::AssetType::SkeletalMesh:
							break;

						case Saturn::AssetType::Material:
						{
							auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );

							// Find the asset.
							Ref<Asset> asset = AssetRegistry::Get().FindAsset( path );
							
							// Importing the asset will happen in this function.
							AssetViewer::Add<MaterialAssetViewer>( asset->ID );
						} break;
						case Saturn::AssetType::MaterialInstance:
							break;

						case Saturn::AssetType::Prefab: 
						{
							auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );

							Ref<Asset> asset = AssetRegistry::Get().FindAsset( path );

							AssetViewer::Add<PrefabViewer>( asset->ID );
						} break; 

						case Saturn::AssetType::PhysicsMaterial:
						{
							auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );

							Ref<Asset> asset = AssetRegistry::Get().FindAsset( path );

							AssetViewer::Add<PhysicsMaterialAssetViewer>( asset->ID );
						} break;

						case Saturn::AssetType::Scene:
						case Saturn::AssetType::Audio:
						case Saturn::AssetType::Script:
						case Saturn::AssetType::Unknown:
						case Saturn::AssetType::COUNT:
						default:
							break;
					}
				}
			}
		}

		ImGui::EndGroup();

		// Draw icon.
		pDrawList->AddImage( Icon->GetDescriptorSet(), TopLeft, ImVec2( TopLeft.x + ThumbnailSize.x, TopLeft.y + ThumbnailSize.y ), { 0, 1 }, { 1, 0 } );

		ImGui::SetCursorScreenPos( ImVec2( TopLeft.x + 2.0f, TopLeft.y + ThumbnailSize.y ) );

		// Filename 

		ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos( ImVec2( cursor.x + EdgeOffset + 5.0f, cursor.y + EdgeOffset + 5.0f ) );

		if( rEntry.is_directory() ) 
		{
			ImGui::BeginVertical( "FILENAME_PANEL", ImVec2( ThumbnailSize.x - EdgeOffset * 2.0f, InfoPanelHeight - EdgeOffset ) );

			ImGui::BeginHorizontal( filename.c_str(), ImVec2( ThumbnailSize.x - 2.0f, 0.0f ) );
			
			ImGui::PushTextWrapPos( ImGui::GetCursorPosX() + ( ThumbnailSize.x - EdgeOffset * 3.0f ) );

			float textWidth = std::min( ImGui::CalcTextSize( filename.c_str() ).x, ThumbnailSize.x );

			ImGui::SetNextItemWidth( textWidth );

			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + ( ThumbnailSize.x - ImGui::CalcTextSize( filename.c_str() ).x ) * 0.5f - EdgeOffset - 5.0f );

			ImGui::Text( filename.c_str() );

			ImGui::PopTextWrapPos();

			ImGui::Spring();
			ImGui::EndHorizontal();
			ImGui::Spring();
			ImGui::EndVertical();
		}
		else
		{
			ImGui::BeginVertical( "FILENAME_PANEL", ImVec2( ThumbnailSize.x - EdgeOffset * 3.0f, InfoPanelHeight - EdgeOffset ) );

			ImGui::BeginHorizontal( "FILENAME_PANEL_HOR", ImVec2( 0.0f, 0.0f ) );

			ImGui::SuspendLayout();

			ImGui::PushTextWrapPos( ImGui::GetCursorPosX() + ( ThumbnailSize.x - EdgeOffset - 5.0f * 3.0f ) );

			ImGui::Text( filename.c_str() );

			ImGui::PopTextWrapPos();
			ImGui::ResumeLayout();

			ImGui::Spring();

			ImGui::EndHorizontal();

			ImGui::Spring();
			ImGui::EndVertical();
		}

		ImGui::PopStyleVar();

		ImGui::NextColumn();
		ImGui::PopID();
	}

	void ContentBrowserPanel::OnDirectorySelected( std::filesystem::path& rPath, bool IsFile /*= false */ )
	{
		m_ChangeDirectory = true;
	}

	void ContentBrowserPanel::UpdateFiles( bool clear /*= false */ )
	{
		if( clear )
			m_Files.clear();

		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
		{
			if( std::find( m_Files.begin(), m_Files.end(), rEntry ) != m_Files.end() )
			{
				if( !std::filesystem::exists( rEntry ) )
					m_Files.erase( std::remove( m_Files.begin(), m_Files.end(), rEntry ), m_Files.end() );

				continue;
			}

			if( !rEntry.is_directory() ) 
			{
				if( AssetTypeFromExtension( rEntry.path().extension().string() ) == AssetType::Unknown )
					continue;
			}

			m_Files.push_back( rEntry );
			m_FilesNeedSorting = true;
		}

		if( m_FilesNeedSorting )
		{
			auto Fn = []( const auto& a, const auto& b ) -> bool
			{
				if( a.is_directory() && !b.is_directory() )
					return true; // a is a directory sort first.
				else if( !a.is_directory() && b.is_directory() )
					return false;
				else
					return a.path().filename() < b.path().filename();
			};

			std::sort( m_Files.begin(), m_Files.end(), Fn );

			m_FilesNeedSorting = false;
		}
	}

}