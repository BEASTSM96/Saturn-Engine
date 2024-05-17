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
#include "ContentBrowserItem.h"

#include "Saturn/ImGui/ImGuiAuxiliary.h"

#include "Saturn/Serialisation/AssetSerialisers.h"
#include "Saturn/Asset/AssetImporter.h"
#include "Saturn/Asset/Asset.h"

#include "Saturn/ImGui/AssetViewer.h"
#include "Saturn/ImGui/PrefabViewer.h"
#include "Saturn/ImGui/StaticMeshAssetViewer.h"
#include "Saturn/ImGui/MaterialAssetViewer.h"
#include "Saturn/ImGui/PhysicsMaterialAssetViewer.h"
#include "Saturn/ImGui/TextureViewer.h"

#include "ContentBrowserThumbnailGenerator.h"

#include <imgui_internal.h>

namespace Saturn {

	static char s_RenameBuffer[ 1024 ];

	ContentBrowserItem::ContentBrowserItem( const std::filesystem::directory_entry& rEntry )
		: m_Entry( rEntry )
	{
		m_Path = rEntry.path().string();
		m_Filename = rEntry.path().filename().replace_extension().filename();

		m_IsDirectory = rEntry.is_directory();

		if( !m_IsDirectory )
		{
			auto path = std::filesystem::relative( m_Path, Project::GetActiveProject()->GetRootDir() );
			auto asset = AssetManager::Get().FindAsset( path );
			
			if( asset )
			{
				m_Asset = asset;
			}
		}

		// Do not generate the icon in the constructor wait until render.
		m_Icon = ContentBrowserThumbnailGenerator::GetDefault( m_IsDirectory ? CB_DIRECTORY_ICON : CB_FILE_ICON );
	}

	ContentBrowserItem::~ContentBrowserItem()
	{
		m_Asset.Reset();
		m_Icon = nullptr;
	}

	void ContentBrowserItem::Draw( ImVec2 ThumbnailSize, float Padding )
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

			pDrawList->AddRectFilled( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_Button ), 5.0f, ImDrawFlags_RoundCornersAll );

			ImGui::ItemSize( ThumbnailSize, style.FramePadding.y );
			ImGui::ItemAdd( ImRect( TopLeft, BottomRight ), ImGui::GetID( m_Path.c_str() ) );

			if( m_IsHovered && !m_IsRenaming )
			{
				// Draw a highlight around the button.
				pDrawList->AddRect( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_ButtonHovered ), 5.0f, ImDrawFlags_RoundCornersAll );

				if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
				{
					DoubleClicked = true;

					m_OnSelected( this, true );
				}

				if( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
				{
					Select();

					m_OnSelected( this, false );
				}
			}

			// Selected but not opened!
			// Double clicked = open (change CB folder)
			// Single clicked = selected or deselected
			if( Clicked && !DoubleClicked )
			{
				m_IsSelected = !m_IsSelected;

				// Selected but not opened!
				m_OnSelected( this, false );
			}

			if( m_IsSelected )
			{
				// Draw a highlight around the button.
				pDrawList->AddRect( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_ButtonHovered ), 5.0f, ImDrawFlags_RoundCornersAll );
			}
		}
		else
		{
			// Generate new thumbnail OR return existing one in cache.
			// Returns default icon while generating.
			m_Icon = ContentBrowserThumbnailGenerator::GetFor( m_Asset );

			// Fill background.
			pDrawList->AddRectFilled( TopLeft, ThumbnailBottomRight, ImGui::GetColorU32( ImGuiCol_Button ), 5.0f, ImDrawFlags_RoundCornersTop );

			// Fill Info area
			pDrawList->AddRectFilled( InfoTopLeft, BottomRight, IM_COL32( 47, 47, 47, 255 ), 5.0f, ImDrawFlags_RoundCornersBottom );

			// Draw line between thumbnail and info.
			//pDrawList->AddLine( ThumbnailBottomRight, InfoTopLeft, IM_COL32( 255, 0, 0, 255 ), 1.5f );
			pDrawList->AddLine( ThumbnailBottomRight, InfoTopLeft, AssetTypeToColor( m_Asset->Type ), 1.5f );

			ImGui::ItemSize( ImRect( TopLeft, BottomRight ).Min, style.FramePadding.y );
			ImGui::ItemAdd( ImRect( TopLeft, BottomRight ), ImGui::GetID( m_Path.c_str() ) );

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

					m_OnSelected( this, false );
				}
			}

			if( ItemClicked )
			{
				if( Input::Get().KeyPressed( RubyKey::Ctrl ) || Input::Get().KeyPressed( RubyKey::RightCtrl ) )
				{
					m_MultiSelected = !m_MultiSelected;
				}

				m_IsSelected = !m_IsSelected;
				m_OnSelected( this, m_IsSelected );
			}

			if( m_IsSelected )
			{
				// Draw a highlight around the button.
				pDrawList->AddRect( TopLeft, BottomRight, ImGui::GetColorU32( ImGuiCol_ButtonHovered ), 5.0f, ImDrawFlags_RoundCornersAll );
			}

			HandleDragDrop();

			if( Open )
			{
				auto path = std::filesystem::relative( m_Path, Project::GetActiveProject()->GetRootDir() );
	
				switch( m_Asset->Type )
				{
					case AssetType::Texture:
					{
						AssetViewer::Add<TextureViewer>( m_Asset->ID );
					} break;

					case AssetType::StaticMesh:
					{
						AssetViewer::Add<StaticMeshAssetViewer>( m_Asset->ID );
					} break;

					case AssetType::SkeletalMesh:
						break;

					case AssetType::Material:
					{
						// Importing the asset will happen in this function.
						AssetViewer::Add<MaterialAssetViewer>( m_Asset->ID );
					} break;
					case AssetType::MaterialInstance:
						break;

					case AssetType::Prefab:
					{
						AssetViewer::Add<PrefabViewer>( m_Asset->ID );
					} break;

					case AssetType::PhysicsMaterial:
					{
						AssetViewer::Add<PhysicsMaterialAssetViewer>( m_Asset->ID );
					} break;

					case AssetType::Audio:
					{
						//AssetViewer::Add<SoundAssetViewer>( m_Asset->ID );
					} break;

					case AssetType::Scene:
					case AssetType::Script:
					case AssetType::Unknown:
					case AssetType::COUNT:
					default:
						break;
				}
			}
		}

		ImGui::EndGroup();

		pDrawList->AddImage( 
			m_Icon->GetDescriptorSet(),
			TopLeft, 
			ImVec2( TopLeft.x + ThumbnailSize.x, TopLeft.y + ThumbnailSize.y ), 
			{ 0, 1 }, { 1, 0 } );

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

		std::filesystem::path newPath = std::format( "{0}\\{1}{2}", oldPath.parent_path().string(), rName, extension );

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
		std::filesystem::path newPath = std::format( "{0}\\{1}", oldPath.parent_path().string(), rName );

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
		// TODO: Check for asset links.

		if( m_IsDirectory )
		{
			AssetManager::Get().Each( [&]( Ref<Asset> asset ) 
				{
					if( asset->Path.string().contains( m_Path.string() ) ) 
					{
						AssetManager::Get().RemoveAsset( asset->ID );
					}
				} );
		}
		else
		{
			auto relativePath = std::filesystem::relative( m_Path, Project::GetActiveProject()->GetRootDir() );
			Ref<Asset> asset = AssetManager::Get().FindAsset( relativePath );

			if( asset )
				AssetManager::Get().RemoveAsset( asset->ID );
		}

		std::filesystem::remove( m_Path );
	}

	void ContentBrowserItem::HandleDragDrop()
	{
		auto Icon = ContentBrowserThumbnailGenerator::GetDefault( m_IsDirectory ? CB_DIRECTORY_ICON : CB_FILE_ICON );

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

			// If we can't drag we can assume this file belongs to a read only asset registry.
			// So set the payload to CB_ITEM_COPY_RO and return out.
			if( !m_CanEverDrag )
			{
				auto path = std::filesystem::relative( m_Path, Application::Get().GetRootContentDir().parent_path().parent_path() );
				const wchar_t* c = path.c_str();

				// Copied from read only asset registry.
				ImGui::SetDragDropPayload( "CB_ITEM_COPY_RO", c, ( wcslen( c ) + 1 ) * sizeof( wchar_t ), ImGuiCond_Once );

				ImGui::EndDragDropSource();
				return;
			}

			auto path = std::filesystem::relative( m_Path, Project::GetActiveProject()->GetRootDir() );
			const wchar_t* c = path.c_str();

			if( Input::Get().KeyPressed( RubyKey::Ctrl ) || Input::Get().KeyPressed( RubyKey::RightCtrl ) )
			{
				if( m_IsSelected )
				{
					ImGui::SetDragDropPayload( "CB_ITEM_MOVE", &m_Entry, sizeof( std::filesystem::directory_entry ), ImGuiCond_Once );
				}
			}

			switch( m_Asset->Type )
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
					ImGui::SetDragDropPayload( "asset_payload", c, ( wcslen( c ) + 1 ) * sizeof( wchar_t ), ImGuiCond_Once );
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
	}

}