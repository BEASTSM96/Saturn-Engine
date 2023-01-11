/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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

#include "UITools.h"
#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Serialisation/AssetSerialisers.h"
#include "Saturn/Asset/AssetImporter.h"

#include "Saturn/ImGui/AssetViewer.h"

#include "Saturn/Project/Project.h"
#include "Saturn/Core/App.h"
#include "Saturn/Asset/AssetRegistry.h"
#include "Saturn/Vulkan/Mesh.h"

#include "MaterialAssetViewer.h"

#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include "Saturn/Asset/Prefab.h"

#include "Saturn/Premake/Premake.h"
#include "Saturn/GameFramework/SourceManager.h"

#include <imgui_internal.h>

namespace Saturn {
	
	static std::filesystem::path s_pAssetsDirectory = "Assets";
	static std::filesystem::path s_pScriptsDirectory = "Scripts";

	static std::filesystem::path s_pMainDirectory = "Scripts";

	static bool s_OpenScriptsPopup = false;
	
	ContentBrowserPanel::ContentBrowserPanel()
		: Panel( "Content Browser Panel" ), m_CurrentPath( s_pAssetsDirectory ), m_FirstFolder( s_pAssetsDirectory ), m_ScriptPath( s_pScriptsDirectory )
	{
		m_DirectoryIcon = Ref<Texture2D>::Create( "assets/textures/editor/DirectoryIcon.png", AddressingMode::Repeat );
		m_FileIcon      = Ref<Texture2D>::Create( "assets/textures/editor/FileIcon.png", AddressingMode::Repeat );
		m_ViewMode      = CBViewMode::Assets;
	}
	
	void ContentBrowserPanel::Draw()
	{
		ImGui::Begin( "Content Browser" );

		if( ImGui::BeginDragDropTarget() )
		{
			auto data = ImGui::AcceptDragDropPayload( "SCENE_HIERARCHY_PANEL_CPREFAB" );

			if( data )
			{
				const Entity* payload = ( const Entity* ) data->Data;

				AssetID id = AssetRegistry::Get().CreateAsset( AssetType::Prefab );
				auto asset = AssetRegistry::Get().FindAsset( id );

				auto PrefabAsset = asset.As<Prefab>();
				PrefabAsset->Create( (Entity&)*payload );

				auto& tag = payload->Tag();

				std::filesystem::path path = m_CurrentPath / tag;
				path.replace_extension( ".prefab" );

				asset->SetPath( path );

				PrefabSerialiser ps;
				ps.Serialise( PrefabAsset );
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::BeginChild( "##CB_TopBar_Actions", ImVec2( 0, 30 ) );

		if( ImGui::Button( "Swap View Mode" ) )
		{
			switch( m_ViewMode )
			{
				case Saturn::CBViewMode::Assets:
				{
					m_ViewMode = CBViewMode::Scripts;

					// The Scripts path is always one dir back.
					SetPath( s_pAssetsDirectory.parent_path() );
				} break;
					
					
				case Saturn::CBViewMode::Scripts:
				{
					m_ViewMode = CBViewMode::Assets;

					SetPath( s_pScriptsDirectory.parent_path() );
				} break;
			}
		}

		ImGui::SameLine();

		if( ImGui::Button( "Add" ) )
		{
			ImGui::OpenPopup( "Add_Assets_Popup" );
		}

		if( ImGui::IsItemHovered() ) 
		{
			ImGui::SetTooltip( "Add extra assets..." );
		}

		if( ImGui::BeginPopup( "Add_Assets_Popup" ) ) 
		{
			if ( m_ViewMode == CBViewMode::Assets )
			{
				if( ImGui::Button( "Import assets" ) )
				{
					// TODO:
				}

				if( ImGui::Button( "Starter assets" ) )
				{
					auto ActiveProject = Project::GetActiveProject();
					auto AssetPath = ActiveProject->GetAssetPath();

					std::filesystem::copy_file( "assets/Templates/Meshes/Cube.fbx", AssetPath / "Meshes" / "Cube.fbx" );
					std::filesystem::copy_file( "assets/Templates/Meshes/Plane.fbx", AssetPath / "Meshes" / "Plane.fbx" );
				}
			}

			ImGui::EndPopup();
		}

		ImGui::EndChild();

		ImGui::BeginChild( "##CB_TopBar", ImVec2( 0, 30 ) );

		switch( m_ViewMode )
		{
			case Saturn::CBViewMode::Assets: 
			{
				if( m_CurrentPath != s_pAssetsDirectory )
				{
					if( ImGui::Button( "<-" ) )
					{
						m_CurrentPath = m_CurrentPath.parent_path();
					}
				}
				else
				{
					ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
					ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
					ImGui::Button( "<-" );
					ImGui::PopStyleVar( 1 );
					ImGui::PopItemFlag();
				}
			} break;

			case Saturn::CBViewMode::Scripts: 
			{
				if( m_CurrentPath != s_pScriptsDirectory )
				{
					if( ImGui::Button( "<-" ) )
					{
						m_CurrentPath = m_CurrentPath.parent_path();
					}
				}
				else
				{
					ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
					ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
					ImGui::Button( "<-" );
					ImGui::PopStyleVar( 1 );
					ImGui::PopItemFlag();
				}
			} break;
		
			default:
				break;
		}

		/*
		if( m_CurrentPath != s_pAssetsDirectory )
		{
			if( ImGui::Button( "<-" ) )
			{
				m_CurrentPath = m_CurrentPath.parent_path();
			}
		}
		else
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
			ImGui::Button( "<-" );
			ImGui::PopStyleVar( 1 );
			ImGui::PopItemFlag();
		}
		*/

		ImGui::SameLine();

		if( std::filesystem::exists( m_FirstFolder ) )
		{
			if( ImGui::Button( "->" ) )
			{
				m_CurrentPath /= std::filesystem::relative( m_FirstFolder, s_pMainDirectory );
			}
		}
		else
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
			ImGui::Button( "->" );
			ImGui::PopStyleVar( 1 );
			ImGui::PopItemFlag();
		}
		
		ImGui::SameLine();
		
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.3f, 0.3f, 0.3f, 0.35f ) );

		int i = 0;
		for( auto& pFolder : m_CurrentPath )
		{
			const char* name = m_ViewMode == CBViewMode::Assets ? "Assets" : "Scripts";

			if( i == 0 && pFolder != name )
				continue;

			i++;

			if( pFolder != name )
			{
				ImGui::Text( "/" );
			}
			
			ImGui::SameLine();

			float size = strlen( pFolder.string().c_str() ) + ImGui::CalcTextSize( pFolder.string().c_str() ).x;

			if( ImGui::Selectable( pFolder.string().c_str(), false, 0, ImVec2( size, 22.0f ) ) )
			{				
			}
			ImGui::SameLine();
		}

		ImGui::PopStyleColor( 2 );

		ImGui::EndChild();

		ImGui::Separator();
		
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

		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
			RenderEntry( rEntry, { thumbnailSizeX, thumbnailSizeY }, padding, true );
		
		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
			RenderEntry( rEntry, { thumbnailSizeX, thumbnailSizeY }, padding, false );

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

		if( ImGui::BeginPopupContextWindow( 0, 1, false ) )	
		{
			if( m_ViewMode == CBViewMode::Assets ) 
			{
				if( ImGui::MenuItem( "Import" ) )
				{
					auto result = Application::Get().OpenFile( "Supported asset types (*.fbx *.gltf *.glb *.png *.tga *.jpeg *.jpg)\0*.fbx; *.gltf; *.glb; *.png; *.tga; *.jpeg; *jpg\0" );

					std::filesystem::path path = result;

					if( path.extension() == ".png" || path.extension() == ".tga" || path.extension() == ".jpeg" || path.extension() == ".jpg" )
					{
						auto id = AssetRegistry::Get().CreateAsset( AssetType::Texture );

						auto asset = AssetRegistry::Get().FindAsset( id );

						std::filesystem::copy_file( path, m_CurrentPath / path.filename() );

						asset->SetPath( m_CurrentPath / path.filename() );

						AssetRegistrySerialiser ars;
						ars.Serialise();
					}

					// Meshes
					if( path.extension() == ".fbx" || path.extension() == ".gltf" )
					{
						auto id = AssetRegistry::Get().CreateAsset( AssetType::StaticMesh );

						auto asset = AssetRegistry::Get().FindAsset( id );

						std::filesystem::copy_file( path, m_CurrentPath / path.filename() );

						asset->SetPath( m_CurrentPath / path.filename() );

						if( path.extension() == ".gltf" )
						{
							auto filename = path.filename().string();

							size_t pos = filename.find_last_of( "/\\" );

							filename = filename.substr( pos + 1 );

							pos = filename.find_last_of( "." );

							filename = filename.substr( 0, pos );

							auto& binaryFile = path.parent_path() / filename += ".bin";
							auto& binaryFileTo = m_CurrentPath / filename += ".bin";

							if( !std::filesystem::exists( binaryFile ) )
								binaryFile = path.parent_path() / filename += ".glb";

							if( std::filesystem::exists( binaryFile ) )
								std::filesystem::copy_file( binaryFile, binaryFileTo );
						}

						// Create the mesh so we can copy over the texture (if any).
						auto mesh = Ref<MeshSource>::Create( path, m_CurrentPath );
						mesh = nullptr;

						AssetRegistrySerialiser ars;
						ars.Serialise();
					}
				}

				if( ImGui::BeginMenu( "Create" ) )
				{
					if( ImGui::MenuItem( "Material" ) )
					{
						auto id = AssetRegistry::Get().CreateAsset( AssetType::Material );
						auto asset = AssetRegistry::Get().FindAsset( id );

						asset->SetPath( m_CurrentPath / "Untitled Material.smaterial" );

						auto materialAsset = asset.As<MaterialAsset>();

						MaterialAssetSerialiser mas;
						mas.Serialise( materialAsset );

						AssetRegistrySerialiser urs;
						urs.Serialise();
					}

					ImGui::EndMenu();
				}
			}
			else
			{
				if( ImGui::BeginMenu( "Create" ) )
				{
					if( ImGui::MenuItem( "Script" ) )
					{
						s_OpenScriptsPopup = true;
					}

					ImGui::EndMenu();
				}
			}

			ImGui::EndPopup();
		}

		if( s_OpenScriptsPopup )
			ImGui::OpenPopup( "Create A Script##Create_Script" );

		ImGui::SetNextWindowSize( { 350.0F, 0.0F } );
		if( ImGui::BeginPopupModal( "Create A Script##Create_Script", &s_OpenScriptsPopup, ImGuiWindowFlags_NoMove ) )
		{
			static std::string n;

			bool PopupModified = false;

			ImGui::BeginVertical( "##inputv" );

			ImGui::Text( "Name:" );

			ImGui::InputText( "##n", ( char* ) n.c_str(), 1024 );

			ImGui::EndVertical();

			if( ImGui::Button( "Create" ) )
			{
				if( !Project::GetActiveProject()->HasPremakeFile() )
				{
					Project::GetActiveProject()->CreatePremakeFile();
				}

				// Update or create the project files.
				Premake* pPremake = new Premake();
				pPremake->Launch( Project::GetActiveProject()->GetRootDir().string() );

				// Next, create the source files.
				// Right now the only script type we support is an entity type.
				SourceManager::Get().CreateEntitySourceFiles( m_CurrentPath, n.c_str() );
				
				AssetRegistrySerialiser ars;
				ars.Serialise();

				PopupModified = true;
			}

			if( PopupModified )
			{
				s_OpenScriptsPopup = false;

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

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
	}

	void ContentBrowserPanel::SwapViewMode( CBViewMode newMode )
	{
		m_ViewMode = newMode;
	}

	void ContentBrowserPanel::RenderEntry( const std::filesystem::directory_entry& rEntry, ImVec2 ThumbnailSize, float Padding, bool excludeFiles /*= true */ )
	{
		if( !rEntry.is_directory() && excludeFiles || rEntry.is_directory() && !excludeFiles )
			return;

		auto* pDrawList = ImGui::GetWindowDrawList();

		auto RelativePath = std::filesystem::relative( rEntry.path(), s_pAssetsDirectory );
		auto path = RelativePath.string();

		if( RelativePath.extension() == ".sreg" )
			return;

		std::string filename = rEntry.path().filename().string();

		Ref<Texture2D> Icon = excludeFiles ? m_DirectoryIcon : m_FileIcon;
		
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
			ItemClicked = ButtonRd( "##CONTENT_BROWSER_ITEM_BTN", ImRect( TopLeft, BottomRight ), true );

			if( !excludeFiles )
			{
				auto assetType = AssetTypeFromExtension( rEntry.path().filename().extension().string() );

				if( ImGui::BeginDragDropSource( ImGuiDragDropFlags_SourceAllowNullID ) )
				{
					const wchar_t* c = rEntry.path().c_str();

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
							break;
						case Saturn::AssetType::SkeletalMesh:
							break;
						case Saturn::AssetType::Material:
						{
							// Find the asset.
							Ref<Asset> asset = AssetRegistry::Get().FindAsset( rEntry.path().string() );
							
							// Importing the asset will happen in this function.
							MaterialAssetViewer::Get().AddMaterialAsset( asset );
						} break;
						case Saturn::AssetType::MaterialInstance:
							break;
						case Saturn::AssetType::Audio:
						case Saturn::AssetType::Scene:
						case Saturn::AssetType::Prefab:
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
		// TODO:
	}

}