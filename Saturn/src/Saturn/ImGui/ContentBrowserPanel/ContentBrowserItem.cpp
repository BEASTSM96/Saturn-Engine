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
#include "ContentBrowserItem.h"

#include "Saturn/ImGui/ImGuiAuxiliary.h"

#include "Saturn/Serialisation/AssetSerialisers.h"
#include "Saturn/Asset/AssetImporter.h"
#include "Saturn/ImGui/AssetViewer.h"
#include "Saturn/ImGui/PrefabViewer.h"
#include "Saturn/ImGui/StaticMeshAssetViewer.h"
#include "Saturn/ImGui/MaterialAssetViewer.h"
#include "Saturn/ImGui/PhysicsMaterialAssetViewer.h"

#include <imgui_internal.h>

namespace Saturn {

	static char s_RenameBuffer[ 1024 ];

	ContentBrowserItem::ContentBrowserItem( const std::filesystem::directory_entry& rEntry )
		: m_Entry( rEntry )
	{
		m_Path = rEntry.path().string();
		m_Filename = rEntry.path().filename().replace_extension().filename();

		m_IsDirectory = rEntry.is_directory();

		m_AssetType = AssetTypeFromExtension( m_Path.filename().extension().string() );
	}

	ContentBrowserItem::~ContentBrowserItem()
	{
	}

	void ContentBrowserItem::Draw( ImVec2 ThumbnailSize, float Padding, Ref<Texture2D> Icon )
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImGuiStyle& style = ImGui::GetStyle();

		const float EdgeOffset = 4.0f;
		const float TextLineHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0f + EdgeOffset * 2.0f;
		const float InfoPanelHeight = std::max( ThumbnailSize.x * 0.5f, TextLineHeight );
		const ImVec2 TopLeft = ImGui::GetCursorScreenPos();
		const ImVec2 ThumbnailBottomRight = ImVec2( TopLeft.x + ThumbnailSize.x, TopLeft.y + ThumbnailSize.y );
		const ImVec2 InfoTopLeft = ImVec2( TopLeft.x, TopLeft.y + ThumbnailSize.y );
		const ImVec2 BottomRight = ImVec2( TopLeft.x + ThumbnailSize.x, TopLeft.y + ThumbnailSize.y + InfoPanelHeight );

		ImGui::PushID( m_Path.c_str() );
		ImGui::BeginGroup();

		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

		// Draw the item.

		if( m_IsDirectory )
		{
			bool Clicked = false;
			bool DoubleClicked = false;

			if( !m_IsRenaming )
				ImGui::ButtonBehavior( ImRect( TopLeft, BottomRight ), ImGui::GetID( m_Path.c_str() ), &m_IsHovered, &Clicked );

			pDrawList->AddRectFilled( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_Button ), 5.0f, ImDrawCornerFlags_All );

			ImGui::ItemSize( ThumbnailSize, style.FramePadding.y );
			ImGui::ItemAdd( ImRect( TopLeft, BottomRight ), ImGui::GetID( m_Path .c_str() ) );

			if( m_IsHovered && !m_IsRenaming )
			{
				// Draw a highlight around the button.
				pDrawList->AddRect( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_ButtonHovered ), 5.0f, ImDrawCornerFlags_All );

				if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
				{
					DoubleClicked = true;

					m_OnDirectorySelected( m_Entry.path().filename() );
				}

				if( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
				{
					Select();
				}
			}

			// Because when we double click we know we will change directory and when we change directory this will no longer be selected.
			if( Clicked && !DoubleClicked )
			{
				m_IsSelected = !m_IsSelected;
			}

			if( m_IsSelected )
			{
				// Draw a highlight around the button.
				pDrawList->AddRect( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_ButtonHovered ), 5.0f, ImDrawCornerFlags_All );
			}
		}
		else
		{
			// Fill background.
			pDrawList->AddRectFilled( TopLeft, ThumbnailBottomRight, ImGui::GetColorU32( ImGuiCol_Button ), 5.0f, ImDrawCornerFlags_Top );

			// Fill Info area
			pDrawList->AddRectFilled( InfoTopLeft, BottomRight, IM_COL32( 47, 47, 47, 255 ), 5.0f, ImDrawCornerFlags_Bot );

			// Draw line between thumbnail and info.
			//pDrawList->AddLine( ThumbnailBottomRight, InfoTopLeft, IM_COL32( 255, 0, 0, 255 ), 1.5f );
			pDrawList->AddLine( ThumbnailBottomRight, InfoTopLeft, AssetTypeToColor( m_AssetType ), 1.5f );

			ImGui::ItemSize( ImRect( TopLeft, BottomRight ).Min, style.FramePadding.y );
			ImGui::ItemAdd( ImRect( TopLeft, BottomRight ), ImGui::GetID( m_Path .c_str() ) );

			bool ItemClicked = false;
			bool Open = false;
			
			if( !m_IsRenaming )
				ItemClicked = Auxiliary::ButtonRd( "##CONTENT_BROWSER_ITEM_BTN", ImRect( TopLeft, BottomRight ), true );

			m_IsHovered = ImGui::IsItemHovered();

			if( m_IsHovered && !m_IsRenaming )
			{
				if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
				{
					Open = true;
				}
				else if( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
				{
					Select();
				}
			}

			if( ItemClicked )
			{
				if( Input::Get().KeyPressed( Key::LeftControl ) || Input::Get().KeyPressed( Key::RightControl ) )
				{
					m_MultiSelected = !m_MultiSelected;
				}

				m_IsSelected = !m_IsSelected;
			}

			if( m_IsSelected )
			{
				// Draw a highlight around the button.
				pDrawList->AddRect( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_ButtonHovered ), 5.0f, ImDrawCornerFlags_All );
			}

			if( ImGui::BeginDragDropSource( ImGuiDragDropFlags_SourceAllowNullID ) )
			{
				// Tooltip
				ImGui::BeginHorizontal( "##dndinfo" );

				Auxiliary::Image( Icon, ImVec2( 24, 24 ) );
				ImGui::Text( m_Filename.string().c_str() );
				
				if( m_MultiSelected )
				{
					ImGui::Text( " + others" );
				}

				ImGui::EndHorizontal();

				auto path = std::filesystem::relative( m_Path, Project::GetActiveProject()->GetRootDir() );
				const wchar_t* c = path.c_str();

				if( Input::Get().KeyPressed( Key::LeftControl ) || Input::Get().KeyPressed( Key::RightControl ) )
				{
					if ( m_IsSelected )
					{
						ImGui::SetDragDropPayload( "CB_ITEM_MOVE", &m_Entry, sizeof( std::filesystem::directory_entry ), ImGuiCond_Once );
					}
				}

				switch( m_AssetType )
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

			if( Open )
			{
				auto path = std::filesystem::relative( m_Path, Project::GetActiveProject()->GetRootDir() );
	
				switch( m_AssetType )
				{
					case AssetType::Texture:
						break;

					case AssetType::StaticMesh:
					{
						// Find the asset.
						Ref<Asset> asset = AssetManager::Get().FindAsset( path );
						AssetViewer::Add<StaticMeshAssetViewer>( asset->ID );
					} break;

					case AssetType::SkeletalMesh:
						break;

					case AssetType::Material:
					{
						// Find the asset.
						Ref<Asset> asset = AssetManager::Get().FindAsset( path );

						// Importing the asset will happen in this function.
						AssetViewer::Add<MaterialAssetViewer>( asset->ID );
					} break;
					case AssetType::MaterialInstance:
						break;

					case AssetType::Prefab:
					{
						Ref<Asset> asset = AssetManager::Get().FindAsset( path );

						AssetViewer::Add<PrefabViewer>( asset->ID );
					} break;

					case AssetType::PhysicsMaterial:
					{
						Ref<Asset> asset = AssetManager::Get().FindAsset( path );

						AssetViewer::Add<PhysicsMaterialAssetViewer>( asset->ID );
					} break;

					case AssetType::Scene:
					case AssetType::Audio:
					case AssetType::Script:
					case AssetType::Unknown:
					case AssetType::COUNT:
					default:
						break;
				}
			}
		}

		ImGui::EndGroup();

		pDrawList->AddImage( Icon->GetDescriptorSet(), TopLeft, ImVec2( TopLeft.x + ThumbnailSize.x, TopLeft.y + ThumbnailSize.y ), { 0, 1 }, { 1, 0 } );

		ImGui::SetCursorScreenPos( ImVec2( TopLeft.x + 2.0f, TopLeft.y + ThumbnailSize.y ) );

		// Filename

		ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos( ImVec2( cursor.x + EdgeOffset + 5.0f, cursor.y + EdgeOffset + 5.0f ) );

		std::string Filename = m_Filename.string();

		if( m_IsDirectory )
		{
			ImGui::BeginVertical( "FILENAME_PANEL", ImVec2( ThumbnailSize.x - EdgeOffset * 2.0f, InfoPanelHeight - EdgeOffset ) );

			ImGui::BeginHorizontal( m_Filename.c_str(), ImVec2( ThumbnailSize.x - 2.0f, 0.0f ) );

			ImGui::PushTextWrapPos( ImGui::GetCursorPosX() + ( ThumbnailSize.x - EdgeOffset * 3.0f ) );

			float textWidth = std::min( ImGui::CalcTextSize( Filename.c_str() ).x, ThumbnailSize.x );

			ImGui::SetNextItemWidth( textWidth );

			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + ( ThumbnailSize.x - ImGui::CalcTextSize( Filename.c_str() ).x ) * 0.5f - EdgeOffset - 5.0f );

			if( m_IsRenaming )
			{
				if( m_StartingRename )
				{
					memset( s_RenameBuffer, 0, 1024 );
					memcpy( s_RenameBuffer, m_Filename.string().c_str(), m_Filename.string().size() );

					ImGui::SetKeyboardFocusHere( 0 );

					m_StartingRename = false;
				}

				if( ImGui::InputText( "##renamefolder", s_RenameBuffer, 1024, ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					m_IsRenaming = false;
					OnRenameCommittedFolder( s_RenameBuffer );

					memset( s_RenameBuffer, 0, 1024 );
				}
			}
			else
			{
				ImGui::Text( Filename.c_str() );
			}

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

			if( m_IsRenaming )
			{
				if( m_StartingRename )
				{
					memset( s_RenameBuffer, 0, 1024 );
					memcpy( s_RenameBuffer, m_Filename.string().c_str(), m_Filename.string().size() );

					ImGui::SetKeyboardFocusHere( 0 );

					m_StartingRename = false;
				}

				if( ImGui::InputText( "##renamefile", s_RenameBuffer, 1024, ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					m_IsRenaming = false;
					OnRenameCommitted( s_RenameBuffer );

					memset( s_RenameBuffer, 0, 1024 );
				}
			}
			else
			{
				ImGui::Text( Filename.c_str() );
			}

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

	void ContentBrowserItem::OnRenameCommitted( const std::string& rName )
	{
		m_Filename = rName;

		std::filesystem::path oldPath = m_Path;
		const std::string extension = oldPath.extension().string();

		std::filesystem::path newPath = fmt::format( "{0}\\{1}{2}", oldPath.parent_path().string(), rName, extension );

		// Rename the file on the filesystem
		std::filesystem::rename( oldPath, newPath );

		// Update our Entry.
		m_Entry = std::filesystem::directory_entry( newPath );
		m_Path = m_Entry.path();

		// Find our asset.
		auto relative = std::filesystem::relative( oldPath, Project::GetActiveProject()->GetRootDir() );
		Ref<Asset> asset = AssetManager::Get().FindAsset( relative );
		if( asset )
		{
			asset->Name = rName;
			asset->SetPath( m_Path );

			AssetManager::Get().Save();
		}
	}

	void ContentBrowserItem::OnRenameCommittedFolder( const std::string& rName )
	{
		m_Filename = rName;

		std::filesystem::path oldPath = m_Path;
		std::filesystem::path newPath = fmt::format( "{0}\\{1}", oldPath.parent_path().string(), rName );

		std::filesystem::rename( oldPath, newPath );

		m_Entry = std::filesystem::directory_entry( newPath );
		m_Path = m_Entry.path();
	}

	void ContentBrowserItem::Select()
	{
		m_IsSelected = true;
		m_MultiSelected = true;
	}

	void ContentBrowserItem::Deselect()
	{
		m_IsSelected = false;
		m_MultiSelected = false;
	}

	void ContentBrowserItem::Rename()
	{
		m_IsRenaming = true;
		m_StartingRename = true;
	}

	void ContentBrowserItem::Delete()
	{
		Ref<Asset> culprit = AssetManager::Get().FindAsset( m_Path );
		AssetManager::Get().RemoveAsset( culprit->ID );

		std::filesystem::remove( m_Path );
	}
}