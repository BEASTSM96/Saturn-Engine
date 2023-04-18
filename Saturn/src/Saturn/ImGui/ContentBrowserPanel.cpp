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

#include "UITools.h"
#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Serialisation/AssetSerialisers.h"
#include "Saturn/Asset/AssetImporter.h"

#include "Saturn/ImGui/AssetViewer.h"
#include "Saturn/ImGui/PrefabViewer.h"

#include "Saturn/Project/Project.h"
#include "Saturn/Core/App.h"
#include "Saturn/Asset/AssetRegistry.h"
#include "Saturn/Vulkan/Mesh.h"

#include "MaterialAssetViewer.h"

#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include "Saturn/Asset/Prefab.h"

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
	
	void ContentBrowserPanel::Draw()
	{
		ImGui::Begin( "Content Browser" );

		//ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };

		if( ImGui::BeginDragDropTarget( ) )
		{
			auto data = ImGui::AcceptDragDropPayload( "ENTITY_PARENT_SCHPANEL" );

			if( data )
			{
				const Entity* payload = ( const Entity* ) data->Data;

				Ref<Prefab> asset = AssetRegistry::Get().CreateAsset<Prefab>( AssetType::Prefab );
				asset->Create( ( Entity& ) *payload );

				auto& tag = payload->Tag();

				std::filesystem::path path = m_CurrentPath / tag;
				path.replace_extension( ".prefab" );

				asset->SetPath( path );

				PrefabSerialiser ps;
				ps.Serialise( asset );

				AssetRegistrySerialiser ars;
				ars.Serialise();
			}

			ImGui::EndDragDropTarget();
		}

		if( m_ChangeDirectory )
		{
			UpdateFiles( true );

			m_ChangeDirectory = false;
		}

		ImGui::SameLine();

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );

		ImGui::BeginChild( "##CB_TopBar", ImVec2( 0, 30 ) );

		if( ImageButton( m_SwapViewIcon, { 24, 24 } ) )
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

		switch( m_ViewMode )
		{
			case Saturn::CBViewMode::Assets: 
			{
				if( m_CurrentPath != s_pAssetsDirectory )
				{
					if( ImageButton( m_BackIcon, { 24, 24 } ) )
					{
						m_CurrentPath = m_CurrentPath.parent_path();

						UpdateFiles( true );
					}
				}
				else
				{
					ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
					ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
					ImageButton( m_BackIcon, { 24, 24 } );
					ImGui::PopStyleVar( 1 );
					ImGui::PopItemFlag();
				}
			} break;

			case Saturn::CBViewMode::Scripts: 
			{
				if( m_CurrentPath != s_pScriptsDirectory )
				{
					if( ImageButton( m_BackIcon, { 24, 24 } ) )
					{
						m_CurrentPath = m_CurrentPath.parent_path();

						UpdateFiles( true );
					}
				}
				else
				{
					ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
					ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
					ImageButton( m_BackIcon, { 24, 24 } );
					ImGui::PopStyleVar( 1 );
					ImGui::PopItemFlag();
				}
			} break;
		
			default:
				break;
		}

		ImGui::SameLine();

		if( std::filesystem::exists( m_FirstFolder ) )
		{
			if( ImageButton( m_ForwardIcon, { 24, 24 } ) )
			{
				m_CurrentPath /= std::filesystem::relative( m_FirstFolder, s_pMainDirectory );

				UpdateFiles( true );
			}
		}
		else
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
			ImageButton( m_ForwardIcon, { 24, 24 } );
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

			ImGui::Selectable( pFolder.string().c_str(), false, 0, ImVec2( size, 22.0f ) );

			ImGui::SameLine();
		}

		ImGui::PopStyleColor( 3 );

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

		if( ImGui::BeginPopupContextWindow( 0, 1, false ) )	
		{
			if( m_ViewMode == CBViewMode::Assets ) 
			{
				if( ImGui::MenuItem( "Starter Assets" ) )
				{
					auto ActiveProject = Project::GetActiveProject();
					auto AssetPath = ActiveProject->GetAssetPath();

					std::filesystem::copy_file( "content/Templates/Meshes/Cube.fbx", AssetPath / "Meshes" / "Cube.fbx" );
					std::filesystem::copy_file( "content/Templates/Meshes/Plane.fbx", AssetPath / "Meshes" / "Plane.fbx" );
				}

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
						m_ShowMeshImport = true;
						m_ImportMeshPath = path;
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

						UpdateFiles( true );
					}

					auto& names = GamePrefabList::Get().GetNames();

					for ( auto& name : names )
					{
						if( ImGui::MenuItem( name.c_str() ) )
						{
							// In order to create this, we will need to create the class the user wants then we can create the prefab from it.

							// Create the prefab asset
							Ref<Prefab> PrefabAsset = AssetRegistry::Get().CreateAsset<Prefab>( AssetType::Prefab );
							PrefabAsset->Create();

							auto asset = AssetRegistry::Get().FindAsset( PrefabAsset->ID );

							// Create the user class
							// Try register
							EntityScriptManager::Get().RegisterScript( name );

							Entity* e = new Entity( PrefabAsset->GetScene()->CreateEntity( name ) );
							e->AddComponent<ScriptComponent>().ScriptName = name;

							SClass* sclass = EntityScriptManager::Get().CreateScript( name, e );

							PrefabAsset->SetEntity( *(Entity*)&e );

							// Set asset path
							std::filesystem::path path = m_CurrentPath / name;
							path.replace_extension( ".prefab" );

							PrefabAsset->SetPath( path );
							asset->SetPath( path ); // HACK

							// Serialise
							PrefabSerialiser ps;
							ps.Serialise( PrefabAsset );

							AssetRegistrySerialiser ars;
							ars.Serialise();

							UpdateFiles( true );
						}
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

		if( m_ShowMeshImport )
			ImGui::OpenPopup( "Import Mesh##IMPORT_MESH" );

		ImGui::SetNextWindowSize( { 350.0F, 0.0F } );
		if( ImGui::BeginPopupModal( "Import Mesh##IMPORT_MESH", &m_ShowMeshImport, ImGuiWindowFlags_NoMove ) )
		{
			static std::filesystem::path s_GLTFBinPath = "";
			static bool s_UseBinFile = false;

			bool PopupModified = false;

			ImGui::BeginVertical( "##inputv" );

			ImGui::Text( "Path:" );

			ImGui::BeginHorizontal( "##inputH" );

			ImGui::InputText( "##path", (char*) m_ImportMeshPath.string().c_str(), 1024 );

			if( ImGui::Button( "Browse" ) ) 
			{
				m_ImportMeshPath = Application::Get().OpenFile( "Supported asset types (*.fbx *.gltf *.glb)\0*.fbx; *.gltf; *.glb\0" );
			}

			ImGui::EndHorizontal();

			ImGui::EndVertical();

			// If the path a GLTF file then we need to file the bin file.
			if( m_ImportMeshPath.extension() == ".gltf" || m_ImportMeshPath.extension() == ".glb" )
			{
				// We can assume the bin file has the same name as the mesh.
				if( s_GLTFBinPath == "" ) 
				{
					s_GLTFBinPath = m_ImportMeshPath;
					s_GLTFBinPath.replace_extension( ".glb" );
				}

				ImGui::BeginVertical( "##gltfinput" );

				ImGui::Text( "GLTF binary file path:" );

				ImGui::BeginHorizontal( "##gltfinputH" );

				ImGui::InputText( "##binpath", ( char* ) s_GLTFBinPath.string().c_str(), 1024 );

				if( ImGui::Button( "Browse" ) )
				{
					s_GLTFBinPath = Application::Get().OpenFile( "Supported asset types (*.glb *.bin)\0*.glb; *.bin\0" );
				}
				
				ImGui::EndHorizontal();

				ImGui::Checkbox( "Use Binary File", &s_UseBinFile );

				ImGui::EndVertical();
			}
			
			ImGui::BeginHorizontal( "##actionsH" );

			if( ImGui::Button( "Create" ) )
			{
				auto id = AssetRegistry::Get().CreateAsset( AssetType::StaticMesh );
				auto asset = AssetRegistry::Get().FindAsset( id );

				// Copy the mesh source.
				std::filesystem::copy_file( m_ImportMeshPath, m_CurrentPath / m_ImportMeshPath.filename() );

				if( s_UseBinFile )
					std::filesystem::copy_file( s_GLTFBinPath, m_CurrentPath / s_GLTFBinPath.filename() );

				auto assetPath = m_CurrentPath / m_ImportMeshPath.filename();
				assetPath.replace_extension( ".stmesh" );

				asset->SetPath( assetPath );

				// TODO: This is bad.
				// Create the mesh so we can copy over the texture (if any).
				auto mesh = Ref<MeshSource>::Create( m_ImportMeshPath, m_CurrentPath );
				mesh = nullptr;

				// Create the mesh asset.
				auto staticMesh = asset.As<StaticMesh>();
				staticMesh = Ref<StaticMesh>::Create();
				staticMesh->ID = asset->ID;
				staticMesh->Path = asset->Path;
				
				auto& meshPath = assetPath.replace_extension( m_ImportMeshPath.extension() );
				staticMesh->SetFilepath( meshPath.string() );

				// Save the mesh asset
				StaticMeshAssetSerialiser sma;
				sma.Serialise( staticMesh );

				staticMesh->SetPath( assetPath );

				AssetRegistrySerialiser ars;
				ars.Serialise();

				PopupModified = true;

				UpdateFiles( true );
			}

			if( ImGui::Button( "Cancel" ) )
			{
				m_ShowMeshImport = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndHorizontal();

			if( PopupModified )
			{
				m_ShowMeshImport = false;

				ImGui::CloseCurrentPopup();
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

				Project::GetActiveProject()->CreateBuildFile();

				// Update or create the project files.
				Premake* pPremake = new Premake();
				pPremake->Launch( Project::GetActiveProject()->GetRootDir().string() );

				// Next, create the source files.
				// Right now the only script type we support is an entity type.
				SourceManager::Get().CreateEntitySourceFiles( m_CurrentPath, n.c_str() );
				
				AssetRegistrySerialiser ars;
				ars.Serialise();

				PopupModified = true;

				UpdateFiles( true );
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
			ItemClicked = ButtonRd( "##CONTENT_BROWSER_ITEM_BTN", ImRect( TopLeft, BottomRight ), true );

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
							break;
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