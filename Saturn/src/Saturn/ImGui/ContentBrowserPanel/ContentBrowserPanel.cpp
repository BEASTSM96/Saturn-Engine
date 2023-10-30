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

#include "Saturn/ImGui/ImGuiAuxiliary.h"
#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Asset/PhysicsMaterialAsset.h"
#include "Saturn/Serialisation/AssetSerialisers.h"
#include "Saturn/Asset/AssetImporter.h"

#include "Saturn/Serialisation/SceneSerialiser.h"

#include "Saturn/Project/Project.h"
#include "Saturn/Core/App.h"
#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Vulkan/Mesh.h"

#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include "Saturn/Asset/Prefab.h"
#include "Saturn/Audio/Sound2D.h"

#include "Saturn/Premake/Premake.h"
#include "Saturn/GameFramework/Core/SourceManager.h"
#include "Saturn/GameFramework/Core/GamePrefabList.h"

#include <imgui_internal.h>

namespace Saturn {
	
	static inline ImVec2 operator+( const ImVec2& lhs, const ImVec2& rhs ) { return ImVec2( lhs.x + rhs.x, lhs.y + rhs.y ); }

	// The absolute Assets and Scripts path.
	static std::filesystem::path s_pAssetsDirectory = "Assets";
	static std::filesystem::path s_pScriptsDirectory = "Scripts";

	/* The root dir, i.e. C:\\MyProjects\\Project1\\Assets */
	static std::filesystem::path s_RootDirectory = "Scripts";

	static bool s_OpenScriptsPopup = false;
	
	ContentBrowserPanel::ContentBrowserPanel()
		: Panel( "Content Browser Panel" ), m_CurrentPath( s_pAssetsDirectory ), m_FirstFolder( s_pAssetsDirectory ), m_ScriptPath( s_pScriptsDirectory )
	{
		m_DirectoryIcon = Ref<Texture2D>::Create( "content/textures/editor/DirectoryIcon.png", AddressingMode::Repeat );
		m_FileIcon      = Ref<Texture2D>::Create( "content/textures/editor/FileIcon.png",      AddressingMode::Repeat );
		m_BackIcon      = Ref<Texture2D>::Create( "content/textures/editor/Left.png",          AddressingMode::Repeat );
		m_ForwardIcon   = Ref<Texture2D>::Create( "content/textures/editor/Right.png",         AddressingMode::Repeat );

		m_ViewMode      = CBViewMode::Assets;
		m_EditorContent = Application::Get().GetRootContentDir();
		m_EditorScripts = m_EditorContent.parent_path();
		m_EditorScripts /= "src";
	}

	ContentBrowserPanel::~ContentBrowserPanel()
	{
		m_DirectoryIcon = nullptr;
		m_FileIcon = nullptr;
		m_BackIcon = nullptr;
		m_ForwardIcon = nullptr;
	}

	void ContentBrowserPanel::DrawFolderTree( const std::filesystem::path& rPath )
	{
		for( const auto& entry : std::filesystem::directory_iterator( rPath ) )
		{
			if( !entry.is_directory() )
				continue;

			const std::filesystem::path& entryPath = entry.path();
			const std::string entryName = entryPath.filename().string();

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

			// I don't know, what happens when we open a folder that is two subfolders down the folder tree will display the assets folder.
			if( m_CurrentPath == entryPath )
				flags |= ImGuiTreeNodeFlags_DefaultOpen;

			if( ImGui::TreeNodeEx( entryName.c_str(), flags ) )
			{
				DrawFolderTree( entryPath );

				ImGui::TreePop();
			}

			if( ImGui::BeginDragDropTarget() )
			{
				auto data = ImGui::AcceptDragDropPayload( "CB_ITEM_MOVE", ImGuiDragDropFlags_None );

				if( data )
				{
					std::filesystem::directory_entry& entry = *( std::filesystem::directory_entry* ) data->Data;

					std::filesystem::path srcPath = entry.path();
					std::filesystem::path dstPath = entryPath / srcPath.filename();

					std::filesystem::path assetPath = std::filesystem::relative( srcPath, Project::GetActiveProject()->GetRootDir() );
					
					std::filesystem::copy_file( entry, dstPath );
					std::filesystem::remove( entry );

					// Find and update the asset that is linked to this path.
					Ref<Asset> culprit = AssetManager::Get().FindAsset( assetPath );
					culprit->SetPath( dstPath );

					AssetManager::Get().Save();

					// TODO: Change this when we support moving multiple items.
					m_SelectedItems[ 0 ]->Deselect();

					UpdateFiles( true );
				}
			}

			if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
			{
				// TODO: Think about this...

				auto path = std::filesystem::relative( entry.path(), s_pAssetsDirectory );

				m_CurrentPath = s_pAssetsDirectory;
				m_CurrentPath /= path;

				m_ChangeDirectory = true;
			}
		}
	}
	
	void ContentBrowserPanel::DrawAssetsFolderTree()
	{
		DrawFolderTree( s_pAssetsDirectory );
	}

	void ContentBrowserPanel::DrawScriptsFolderTree()
	{
		DrawFolderTree( s_pScriptsDirectory );
	}

	void ContentBrowserPanel::DrawRootFolder( CBViewMode type, bool open/* = false*/ )
	{
		switch( type )
		{
			case CBViewMode::Assets: 
			{
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
				flags |= ImGuiTreeNodeFlags_DefaultOpen;

				ImGui::PushID( "PrjAssets" );

				bool opened = ImGui::TreeNodeEx( "Assets##PrjAssets", flags );

				if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
				{
					// Switch and set path to the game content.
					m_ViewMode = CBViewMode::Assets;
					SetPath( Project::GetActiveProject()->GetRootDir() );
				}

				if( opened )
				{
					DrawAssetsFolderTree();

					ImGui::TreePop();
				}

				ImGui::PopID();
			} break;

			case CBViewMode::Scripts: 
			{
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

				flags |= ImGuiTreeNodeFlags_DefaultOpen;

				ImGui::PushID( "PrjScripts" );

				bool opened = ImGui::TreeNodeEx( "Scripts##PrjScripts", flags );

				if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
				{
					// Switch and set path to the game content.
					m_ViewMode = CBViewMode::Scripts;
					SetPath( Project::GetActiveProject()->GetRootDir() );
				}

				if( opened )
				{
					DrawScriptsFolderTree();

					ImGui::TreePop();
				}

				ImGui::PopID();
			} break;

			default:
				break;
		}
	}

	void ContentBrowserPanel::AssetsPopupContextMenu()
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
			auto result = Application::Get().OpenFile( "Supported asset types (*.fbx *.gltf *.glb *.png *.tga *.jpeg *.jpg *wav)\0*.fbx; *.gltf; *.glb; *.png; *.tga; *.jpeg; *jpg; *.wav\0" );

			std::filesystem::path path = result;

			if( path.extension() == ".png" || path.extension() == ".tga" || path.extension() == ".jpeg" || path.extension() == ".jpg" )
			{
				auto id = AssetManager::Get().CreateAsset( AssetType::Texture );

				auto asset = AssetManager::Get().FindAsset( id );

				std::filesystem::copy_file( path, m_CurrentPath / path.filename() );

				asset->SetPath( m_CurrentPath / path.filename() );

				AssetRegistrySerialiser ars;
				ars.Serialise( AssetManager::Get().GetAssetRegistry() );
			}

			// Meshes
			if( path.extension() == ".fbx" || path.extension() == ".gltf" )
			{
				m_ShowMeshImport = true;
				m_ImportMeshPath = path;
			}

			// Audio
			if( path.extension() == ".wav" || path.extension() == ".mp3" )
			{
				m_ShowSoundImport = true;
				m_ImportSoundPath = path;
			}
		}

		if( ImGui::BeginMenu( "Create" ) )
		{
			if( ImGui::MenuItem( "New Folder" ) )
			{
				std::filesystem::create_directories( m_CurrentPath / "New Folder" );

				UpdateFiles( true );

				FindAndRenameItem( m_CurrentPath / "New Folder" );
			}

			if( ImGui::MenuItem( "Material" ) )
			{
				auto id = AssetManager::Get().CreateAsset( AssetType::Material );
				auto asset = AssetManager::Get().FindAsset( id );
				auto newPath = m_CurrentPath / "Untitled Material.smaterial";
				uint32_t count = GetFilenameCount( "Untitled Material.smaterial" );

				if( count >= 1 )
				{
					newPath = fmt::format( "{0}\\{1} ({2}).smaterial", m_CurrentPath.string(), "Untitled Material", count );
				}

				asset->SetPath( newPath );
				Ref<MaterialAsset> material = asset;
				material = Ref<MaterialAsset>::Create( nullptr );

				// TODO: (Asset) Fix this.
				struct
				{
					UUID ID;
					AssetType Type;
					uint32_t Flags;
					std::filesystem::path Path;
					std::string Name;
				} OldAssetData = {};

				OldAssetData.ID = asset->ID;
				OldAssetData.Type = asset->Type;
				OldAssetData.Flags = asset->Flags;
				OldAssetData.Path = asset->Path;
				OldAssetData.Name = asset->Name;

				asset = material;
				asset->ID = OldAssetData.ID;
				asset->Type = OldAssetData.Type;
				asset->Flags = OldAssetData.Flags;
				asset->Path = OldAssetData.Path;
				asset->Name = OldAssetData.Name;

				MaterialAssetSerialiser mas;
				mas.Serialise( asset );

				AssetRegistrySerialiser ars;
				ars.Serialise( AssetManager::Get().GetAssetRegistry() );

				UpdateFiles( true );
				
				FindAndRenameItem( asset->Path );
			}

			if( ImGui::MenuItem( "Physics Material" ) )
			{
				auto id = AssetManager::Get().CreateAsset( AssetType::PhysicsMaterial );
				auto asset = AssetManager::Get().FindAsset( id );

				asset->SetPath( m_CurrentPath / "Untitled Physics Material.sphymaterial" );

				auto materialAsset = asset.As<PhysicsMaterialAsset>();

				PhysicsMaterialAssetSerialiser mas;
				mas.Serialise( materialAsset );

				AssetRegistrySerialiser ars;
				ars.Serialise( AssetManager::Get().GetAssetRegistry() );

				UpdateFiles( true );
				FindAndRenameItem( asset->Path );
			}

			if( ImGui::MenuItem( "Empty Scene" ) )
			{
				auto id = AssetManager::Get().CreateAsset( AssetType::Scene );
				auto asset = AssetManager::Get().FindAsset( id );

				asset->SetPath( m_CurrentPath / "Empty Scene.scene" );

				Ref<Scene> newScene = Ref<Scene>::Create();

				SceneSerialiser ss( newScene );
				ss.Serialise( newScene->Filepath() );

				AssetManager::Get().Save();

				UpdateFiles( true );
				FindAndRenameItem( asset->Path );
			}

			auto& names = GamePrefabList::Get().GetNames();

			for( auto& name : names )
			{
				if( ImGui::MenuItem( name.c_str() ) )
				{
					// In order to create this, we will need to create the class the user wants then we can create the prefab from it.

					// Create the prefab asset
					Ref<Prefab> PrefabAsset = AssetManager::Get().CreateAsset<Prefab>( AssetType::Prefab, AssetRegistryType::Game );
					PrefabAsset->Create();

					auto asset = AssetManager::Get().FindAsset( PrefabAsset->ID );

					// Create the user class
					// Try register
					/*
					EntityScriptManager::Get().RegisterScript( name );

					Entity* e = new Entity( PrefabAsset->GetScene()->CreateEntity( name ) );
					e->AddComponent<ScriptComponent>().ScriptName = name;

					SClass* sclass = EntityScriptManager::Get().CreateScript( name, nullptr );

					PrefabAsset->SetEntity( *( Entity* ) &e );
					*/

					// Set asset path
					std::filesystem::path path = m_CurrentPath / name;
					path.replace_extension( ".prefab" );

					PrefabAsset->SetPath( path );
					asset->SetPath( path ); // HACK

					// Serialise
					PrefabSerialiser ps;
					ps.Serialise( PrefabAsset );

					AssetRegistrySerialiser ars;
					ars.Serialise( AssetManager::Get().GetAssetRegistry() );

					UpdateFiles( true );
				}
			}

			ImGui::EndMenu();
		}
	}

	void ContentBrowserPanel::ScriptsPopupContextMenu()
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

	void ContentBrowserPanel::OnFilewatchEvent( const std::string& rPath, const filewatch::Event Event )
	{
		switch( Event )
		{
			case filewatch::Event::added: 
			{
			} break;

			case filewatch::Event::removed:
			{
			} break;

			case filewatch::Event::modified:
			{
			} break;

			case filewatch::Event::renamed_new:
			{ 
				UpdateFiles( true );
			} break;

			case filewatch::Event::renamed_old:
			{
				UpdateFiles( true );
			} break;

			default:
				break;
		}
	}

	void ContentBrowserPanel::FindAndRenameItem( const std::filesystem::path& rPath )
	{
		for( auto&& rrItem : m_Files )
		{
			if( rrItem->Path() == rPath )
			{
				rrItem->Rename();
				break;
			}
		}
	}

	Ref<ContentBrowserItem> ContentBrowserPanel::FindItem( const std::filesystem::path& rPath )
	{
		for( auto&& rrItem : m_Files )
		{
			if( rrItem->Path() == rPath )
			{
				return rrItem;
			}
		}

		return nullptr;
	}

	Ref<ContentBrowserItem> ContentBrowserPanel::GetActiveHoveredItem()
	{
		for( auto&& rrItem : m_Files )
		{
			if( rrItem->IsHovered() )
			{
				return rrItem;
			}
		}

		return nullptr;
	}

	uint32_t ContentBrowserPanel::GetFilenameCount( const std::string& rName )
	{
		uint32_t count = 0;

		for( const auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
		{
			if( !rEntry.is_regular_file() )
				continue;

			std::string filename = rEntry.path().filename().string();

			if( filename.find( rName ) != std::string::npos )
				count++;
		}

		return count;
	}

	void ContentBrowserPanel::EdDrawRootFolder( CBViewMode type, bool open /*= false */ )
	{
		switch( type )
		{
			case Saturn::CBViewMode::Assets: 
			{
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
				if( open )
					flags |= ImGuiTreeNodeFlags_DefaultOpen;

				ImGui::PushID( "EDITOR_ASSETS" );

				bool opened = ImGui::TreeNodeEx( "Assets##EDITOR_ASSETS", flags );

				if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
				{
					// Switch and set path to the game content.
					m_ViewMode = CBViewMode::Assets;
					EdSetPath();
				}

				if( opened )
				{
					EdDrawAssetsFolderTree();

					ImGui::TreePop();
				}

				ImGui::PopID();
			} break;
			
			
			case Saturn::CBViewMode::Scripts: 
			{
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
				if( open )
					flags |= ImGuiTreeNodeFlags_DefaultOpen;

				ImGui::PushID( "EDITOR_SCRIPTS" );

				bool opened = ImGui::TreeNodeEx( "Scripts##EDITOR_SCRIPTS", flags );

				if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
				{
					// Switch and set path to the game content.
					m_ViewMode = CBViewMode::Scripts;
					EdSetPath();
				}

				if( opened )
				{
					DrawFolderTree( m_EditorScripts );

					ImGui::TreePop();
				}

				ImGui::PopID();
			} break;

			default:
				break;
		}
	}

	void ContentBrowserPanel::EdDrawAssetsFolderTree()
	{
		DrawFolderTree( m_EditorContent );
	}

	void ContentBrowserPanel::EdSetPath()
	{
		// No need to set these are they are not used by the editor.
		//s_pAssetsDirectory = m_EditorContent;
		//s_pScriptsDirectory = m_EditorScripts;

		switch( m_ViewMode )
		{
			case Saturn::CBViewMode::Assets:
			{
				s_RootDirectory = m_EditorContent;
				m_CurrentPath = m_EditorContent;
				m_FirstFolder = m_EditorContent;
			} break;

			case Saturn::CBViewMode::Scripts:
			{
				s_RootDirectory = m_EditorScripts;
				m_CurrentPath = m_EditorScripts;
				m_FirstFolder = m_EditorScripts;
			} break;
		}

		UpdateFiles( true );
	}

	void ContentBrowserPanel::Draw()
	{
		//bool open_demo = true;
		//ImGui::ShowDemoWindow( &open_demo );

		ImGui::Begin( "Content Browser" );

		if( m_ChangeDirectory )
		{
			UpdateFiles( true );

			m_ChangeDirectory = false;
		}

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::BeginChild( "Top Bar", ImVec2( 0, 30 ), false, flags );

		// Back button.
		if( m_CurrentPath != s_pAssetsDirectory )
		{
			if( Auxiliary::ImageButton( m_BackIcon, { 24, 24 } ) )
			{
				m_CurrentPath = m_CurrentPath.parent_path();

				for( auto&& rrItem : m_SelectedItems )
				{
					rrItem->Deselect();
				}

				m_SelectedItems.clear();

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
				m_CurrentPath /= std::filesystem::relative( m_FirstFolder, s_RootDirectory );

				for( auto&& rrItem : m_SelectedItems )
				{
					rrItem->Deselect();
				}

				m_SelectedItems.clear();

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

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.3f, 0.3f, 0.3f, 0.35f ) );

		ImGui::SameLine();

		// I don't think this is a good way because this is the absolute path.
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

			std::string filename = pFolder.string();

			float size = strlen( filename.c_str() ) + ImGui::CalcTextSize( filename.c_str() ).x;

			ImGui::Selectable( filename.c_str(), false, 0, ImVec2( size, 22.0f ) );

			ImGui::SameLine();
		}

		ImGui::PopStyleColor( 2 );

		ImGui::EndChild();

		ImGui::BeginChild( "Folder Tree", ImVec2( 200, 0 ), false );

		if( Auxiliary::TreeNode( Project::GetActiveProject()->GetName().c_str() ) )
		{
			DrawRootFolder( CBViewMode::Assets );
			DrawRootFolder( CBViewMode::Scripts );

			Auxiliary::EndTreeNode();
		}

		if( Auxiliary::TreeNode( "Editor", false ) )
		{
			EdDrawRootFolder( CBViewMode::Assets, true );
			EdDrawRootFolder( CBViewMode::Scripts );

			Auxiliary::EndTreeNode();
		}

		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild( "Folder Contents", ImVec2( 0, 0 ), false );

		// Search
		ImGui::SetNextItemWidth( 436.0f );
		if( m_TextFilter.Draw( "##search", "Search For Content" ) )
		{
			m_Searching = m_TextFilter.IsActive();
		}

		ImVec2 contentSize = ImGui::GetContentRegionAvail();

		/*
		if( ImGui::IsMouseHoveringRect( ImGui::GetWindowPos(), ImGui::GetWindowPos() + contentSize ) )
		{
			if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
			{
				// TODO: Think about this.
				if( m_SelectedItems.size() )
				{
					bool IsAnyItemHoverved = GetActiveHoveredItem();
					bool Clear = false;

					for( auto&& rrItem : m_SelectedItems )
					{
						if( !rrItem->IsSelected() || rrItem->IsRenaming() )
							continue;

						// If the item is multi selected the we need to check if we are still any item is hovered.
						if( rrItem->MultiSelected() && IsAnyItemHoverved )
							break;

						// If we are not multi selected we can just check if we are still hovered.
						if( rrItem->IsHovered() )
							break;

						rrItem->Deselect();

						Clear = true;
					}

					if( Clear )
						m_SelectedItems.clear();
				}
			}
		}
		*/

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

		if( !m_Searching )
		{
			for( auto& item : m_Files )
			{
				Ref<Texture2D> Icon = item->IsDirectory() ? m_DirectoryIcon : m_FileIcon;

				item->Draw( { thumbnailSizeX, thumbnailSizeY }, padding, Icon );

				// This happens if we rename a file as we then have to create the file cache again.
				if( !item )
					break;

				// Is the item in the selection list if so and we are no longer selected then we need to remove it.
				if( std::find( m_SelectedItems.begin(), m_SelectedItems.end(), item ) != m_SelectedItems.end() )
				{
					if( !item->IsSelected() )
					{
						item->Deselect();

						m_SelectedItems.erase( std::remove( m_SelectedItems.begin(), m_SelectedItems.end(), item ), m_SelectedItems.end() );
					}
				}
				else
				{
					if( item->IsSelected() )
					{
						m_SelectedItems.push_back( item );

						// We are selected but if we are not multi selected, we need to deselect the other one.
						if( !item->MultiSelected() && item != m_SelectedItems[ 0 ] )
						{
							m_SelectedItems[ 0 ]->Deselect();

							m_SelectedItems.erase( std::remove( m_SelectedItems.begin(), m_SelectedItems.end(), item ), m_SelectedItems.end() );
						}
					}
				}
			}
		}
		else
		{
			for( const auto& entry : std::filesystem::recursive_directory_iterator( s_pAssetsDirectory ) )
			{
				if( m_TextFilter.PassFilter( entry.path().filename().string().c_str() ) )
				{
				}
			}
		}

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

		if( ImGui::BeginPopupContextWindow( 0, 1, true ) )
		{
			// Theses actions are only going to be used when one item is selected.
			if( m_SelectedItems.size() )
			{
				// Common Actions
				if( ImGui::MenuItem( "Rename" ) )
				{
					m_SelectedItems[ 0 ]->Rename();

					// Once we have filewatch setup we will no longer need to do this.
					//UpdateFiles( true );
				}

				// Folder Actions
				if( m_SelectedItems[ 0 ]->IsDirectory() )
				{
					if( ImGui::MenuItem( "Show In Explorer" ) )
					{
						// TODO TEMP: Create a process class.

						std::filesystem::path AssetPath = m_SelectedItems[ 0 ]->Path();
						std::string AssetPathString = AssetPath.string();

						STARTUPINFOA StartupInfo = {};
						StartupInfo.cb = sizeof( StartupInfo );
						StartupInfo.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
						StartupInfo.dwFlags = STARTF_USESTDHANDLES;

						PROCESS_INFORMATION ProcessInfo;

						std::string CommandLine = "explorer.exe '";
						CommandLine += AssetPathString;
						CommandLine += "'";

						bool res = CreateProcessA( nullptr, CommandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo );

						if( !res )
							SAT_CORE_ERROR( "Failed to start explorer process!" );

						CloseHandle( ProcessInfo.hProcess );
						CloseHandle( ProcessInfo.hThread );
					}
				}
				else
				{
					if( ImGui::MenuItem( "Delete" ) )
					{
						m_SelectedItems[ 0 ]->Delete();

						// Once we have filewatch setup we will no longer need to do this.
						UpdateFiles( true );
					}
				}
			}
			else
			{
				if( m_ViewMode == CBViewMode::Assets )
				{
					AssetsPopupContextMenu();
				}
				else
				{
					ScriptsPopupContextMenu();
				}
			}

			ImGui::EndPopup();
		}

		if( m_ShowMeshImport )
			ImGui::OpenPopup( "Import Mesh##IMPORT_MESH" );

		if( m_ShowSoundImport )
			ImGui::OpenPopup( "Import Sound##IMPORT_SOUND" );

		ImGui::SetNextWindowSize( { 350.0F, 0.0F } );
		if( ImGui::BeginPopupModal( "Import Sound##IMPORT_SOUND", &m_ShowSoundImport, ImGuiWindowFlags_NoMove ) )
		{
			bool PopupModified = false;

			ImGui::BeginVertical( "##inputv" );

			ImGui::Text( "Path:" );

			ImGui::BeginHorizontal( "##inputH" );

			ImGui::InputText( "##path", ( char* ) m_ImportSoundPath.string().c_str(), 1024 );

			if( ImGui::Button( "Browse" ) )
			{
				m_ImportSoundPath = Application::Get().OpenFile( "Supported asset types (*.wav *.mp3)\0*.wav; *.mp3\0" );
			}

			ImGui::EndHorizontal();
			ImGui::EndVertical();

			ImGui::BeginHorizontal( "##actionsH" );

			if( ImGui::Button( "Create" ) )
			{
				// TODO: Right now we only support sound 2Ds.
				auto id = AssetManager::Get().CreateAsset( AssetType::Audio );
				auto asset = AssetManager::Get().FindAsset( id );

				// Copy the audio source.
				std::filesystem::copy_file( m_ImportSoundPath, m_CurrentPath / m_ImportSoundPath.filename() );

				auto assetPath = m_CurrentPath / m_ImportSoundPath.filename();
				assetPath.replace_extension( ".s2d" );

				assetPath = std::filesystem::relative( assetPath, Project::GetActiveProject()->GetRootDir() );

				asset->Path = assetPath;

				// Create the asset.
				auto sound = asset.As<Sound2D>();
				sound = Ref<Sound2D>::Create();
				sound->ID = asset->ID;
				sound->Path = assetPath;
				sound->Type = AssetType::Audio;

				sound->SetRawPath( m_CurrentPath / m_ImportSoundPath.filename() );

				// Save the asset
				Sound2DAssetSerialiser s2d;
				s2d.Serialise( sound );

				AssetRegistrySerialiser ars;
				ars.Serialise( AssetManager::Get().GetAssetRegistry() );

				sound->SetPath( assetPath );

				PopupModified = true;

				UpdateFiles( true );
			}

			if( ImGui::Button( "Cancel" ) )
			{
				m_ShowSoundImport = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndHorizontal();

			if( PopupModified )
			{
				m_ShowSoundImport = false;

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::SetNextWindowSize( { 350.0F, 0.0F } );
		if( ImGui::BeginPopupModal( "Import Mesh##IMPORT_MESH", &m_ShowMeshImport, ImGuiWindowFlags_NoMove ) )
		{
			static std::filesystem::path s_GLTFBinPath = "";
			static bool s_UseBinFile = true;

			bool PopupModified = false;

			ImGui::BeginVertical( "##inputv" );

			ImGui::Text( "Path:" );

			ImGui::BeginHorizontal( "##inputH" );

			ImGui::InputText( "##path", ( char* ) m_ImportMeshPath.string().c_str(), 1024 );

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
					s_GLTFBinPath.replace_extension( ".bin" );
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
				auto id = AssetManager::Get().CreateAsset( AssetType::StaticMesh );
				auto asset = AssetManager::Get().FindAsset( id );

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
				ars.Serialise( AssetManager::Get().GetAssetRegistry() );

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
				ars.Serialise( AssetManager::Get().GetAssetRegistry() );

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

		ImGui::EndChild();

		ImGui::PopStyleColor();

		ImGui::End();
	}

	void ContentBrowserPanel::SetPath( const std::filesystem::path& rPath )
	{
		s_pAssetsDirectory = rPath / "Assets";
		s_pScriptsDirectory = rPath / "Scripts";

		switch( m_ViewMode )
		{
			case Saturn::CBViewMode::Assets: 
			{
				s_RootDirectory = s_pAssetsDirectory;
				m_CurrentPath = s_pAssetsDirectory;
				m_FirstFolder = s_pAssetsDirectory;
			} break;

			case Saturn::CBViewMode::Scripts: 
			{
				s_RootDirectory = s_pScriptsDirectory;
				m_CurrentPath = s_pScriptsDirectory;
				m_FirstFolder = s_pScriptsDirectory;
			} break;
		}

		m_Watcher = new filewatch::FileWatch<std::string>( s_pAssetsDirectory.string(),
			[this]( const std::string& path, const filewatch::Event event )
			{
				OnFilewatchEvent( path, event );
			} );

		UpdateFiles( true );
	}

	void ContentBrowserPanel::OnDirectorySelected( const std::filesystem::path& rPath )
	{
		m_CurrentPath /= rPath;

		m_ChangeDirectory = true;

		for( auto&& rrItem : m_SelectedItems )
		{
			rrItem->Deselect();
		}

		m_SelectedItems.clear();
	}

	void ContentBrowserPanel::UpdateFiles( bool clear /*= false */ )
	{
		if( clear )
			m_Files.clear();

		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
		{
			Ref<ContentBrowserItem> item = Ref<ContentBrowserItem>::Create( rEntry );

			item->SetDirectorySelectedFn( SAT_BIND_EVENT_FN( OnDirectorySelected ) );

			if( std::find( m_Files.begin(), m_Files.end(), item ) != m_Files.end() )
			{
				if( !std::filesystem::exists( rEntry ) )
					m_Files.erase( std::remove( m_Files.begin(), m_Files.end(), item ), m_Files.end() );

				continue;
			}

			if( !rEntry.is_directory() ) 
			{
				if( AssetTypeFromExtension( rEntry.path().extension().string() ) == AssetType::Unknown )
					continue;
			}

			m_Files.push_back( item );

			m_FilesNeedSorting = true;
		}

		if( m_FilesNeedSorting )
		{
			auto Fn = []( Ref<ContentBrowserItem>& a, Ref<ContentBrowserItem>& b) -> bool
			{
				if( a->IsDirectory() && !b->IsDirectory() )
					return true; // a is a directory sort first.
				else if( !a->IsDirectory() && b->IsDirectory() )
					return false;
				else
					return a->Filename() < b->Filename();
			};

			std::sort( m_Files.begin(), m_Files.end(), Fn );
			m_FilesNeedSorting = false;
		}
	}

}