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
#include "ContentBrowserPanel.h"

#include "Saturn/ImGui/ImGuiAuxiliary.h"
#include "Saturn/ImGui/AssetImportPopups.h"

#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Asset/PhysicsMaterialAsset.h"
#include "Saturn/Asset/AssetImporter.h"

#include "Saturn/Serialisation/AssetSerialisers.h"
#include "Saturn/Serialisation/SceneSerialiser.h"

#include "Saturn/Core/App.h"
#include "Saturn/Core/Process.h"

#include "Saturn/Project/Project.h"
#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Vulkan/Mesh.h"

#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include "Saturn/Asset/Prefab.h"
#include "Saturn/Audio/Sound2D.h"

#include "Saturn/Premake/Premake.h"

#include "Saturn/GameFramework/Core/SourceManager.h"
#include "Saturn/GameFramework/Core/ClassMetadataHandler.h"
#include "Saturn/GameFramework/Core/GameModule.h"

#include "ContentBrowserThumbnailGenerator.h"

#include <imgui_internal.h>
#include <ranges>

namespace Saturn {
	
	static inline ImVec2 operator+( const ImVec2& lhs, const ImVec2& rhs ) { return ImVec2( lhs.x + rhs.x, lhs.y + rhs.y ); }

	static bool s_OpenScriptsPopup = false;
	static bool s_OpenClassInstancePopup = false;
	
	ContentBrowserPanel::ContentBrowserPanel()
		: ContentBrowserBase()
	{
		m_ViewMode = CBViewMode::Assets;
		ContentBrowserThumbnailGenerator::Init();
	}

	ContentBrowserPanel::ContentBrowserPanel( const std::string& rName )
		: ContentBrowserBase()
	{
	}

	ContentBrowserPanel::~ContentBrowserPanel()
	{
		ContentBrowserThumbnailGenerator::Terminate();
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
					Ref<Asset> target = AssetManager::Get().FindAsset( assetPath );
					target->SetPath( dstPath );

					AssetManager::Get().Save();

					// TODO: Change this when we support moving multiple items.
					m_SelectedItems[ 0 ]->Deselect();

					UpdateFiles( true );
				}
			}

			if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
			{
				// TODO: Think about this...

				auto path = std::filesystem::relative( entry.path(), m_CurrentViewModeDirectory );

				m_CurrentPath = m_RootPath;
				m_CurrentPath /= path;

				m_ChangeDirectory = true;
			}
		}
	}
	
	void ContentBrowserPanel::DrawAssetsFolderTree()
	{
		DrawFolderTree( m_CurrentViewModeDirectory );
	}

	void ContentBrowserPanel::DrawScriptsFolderTree()
	{
		DrawFolderTree( m_ScriptPath );
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
					ClearSelected();

					// Switch and set path to the game content.
					m_ViewMode = CBViewMode::Assets;
					ResetPath( Project::GetActiveProject()->GetRootDir() );
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
					ClearSelected();
				
					// Switch and set path to the game content.
					m_ViewMode = CBViewMode::Scripts;
					ResetPath( Project::GetActiveProject()->GetRootDir() );
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

			if( ImGui::MenuItem( "New Material" ) )
			{
				auto id = AssetManager::Get().CreateAsset( AssetType::Material );
				auto asset = AssetManager::Get().FindAsset( id );
				auto newPath = m_CurrentPath / "Untitled Material.smaterial";
				int32_t count = GetFilenameCount( "Untitled Material.smaterial" );

				if( count >= 1 )
				{
					newPath = std::format( "{0}\\{1} ({2}).smaterial", m_CurrentPath.string(), "Untitled Material", count );
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

			if( ImGui::MenuItem( "New Physics Material" ) )
			{
				auto id = AssetManager::Get().CreateAsset( AssetType::PhysicsMaterial );
				auto asset = AssetManager::Get().FindAsset( id );
				auto newPath = m_CurrentPath / "Untitled Physics Material.smaterial";

				int32_t count = GetFilenameCount( "Untitled Physics Material.smaterial" );

				if( count >= 1 )
				{
					newPath = std::format( "{0}\\{1} ({2}).sphymaterial", m_CurrentPath.string(), "Untitled Physics Material", count );
				}

				asset->SetPath( newPath );

				auto materialAsset = asset.As<PhysicsMaterialAsset>();

				PhysicsMaterialAssetSerialiser mas;
				mas.Serialise( materialAsset );

				AssetRegistrySerialiser ars;
				ars.Serialise( AssetManager::Get().GetAssetRegistry() );

				UpdateFiles( true );
				FindAndRenameItem( asset->Path );
			}

			if( ImGui::MenuItem( "New Scene" ) )
			{
				auto id = AssetManager::Get().CreateAsset( AssetType::Scene );
				auto asset = AssetManager::Get().FindAsset( id );
				auto newPath = m_CurrentPath / "Empty Scene.scene";
				int32_t count = GetFilenameCount( "Empty Scene.scene" );

				if( count >= 1 )
				{
					newPath = std::format( "{0}\\{1} ({2}).scene", m_CurrentPath.string(), "Empty Scene", count );
				}

				asset->SetPath( newPath );

				Ref<Scene> newScene = Ref<Scene>::Create();
				Scene* CurrentScene = GActiveScene;
				Scene::SetActiveScene( newScene.Get() );

				SceneSerialiser ss( newScene );
				ss.Serialise();

				Scene::SetActiveScene( CurrentScene );

				AssetManager::Get().Save();

				UpdateFiles( true );
				FindAndRenameItem( asset->Path );
			}

			if( ImGui::MenuItem( "New Entity Instance" ) ) 
			{
				s_OpenClassInstancePopup = true;
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
			case filewatch::Event::removed:
			{
				ClearSearchQuery();
				UpdateFiles( true );
			} break;

			case filewatch::Event::modified:
			{
			} break;

			case filewatch::Event::renamed_new:
			case filewatch::Event::renamed_old:
			{
				ClearSearchQuery();
				UpdateFiles( true );
			} break;

			default:
				break;
		}
	}

	Ref<ContentBrowserItem> ContentBrowserPanel::GetActiveHoveredItem()
	{
		const auto Itr = std::find_if( m_Files.begin(), m_Files.end(), []( auto& rItem ) { return rItem->IsHovered(); } );

		if( Itr != m_Files.end() )
			return *Itr;

		return nullptr;
	}

	void ContentBrowserPanel::BuildSearchList()
	{
		if( m_ValidSearchFiles.size() )
			m_ValidSearchFiles.clear();

		for( const auto& entry : std::filesystem::recursive_directory_iterator( m_CurrentViewModeDirectory ) )
		{
			if( m_TextFilter.PassFilter( entry.path().filename().string().c_str() ) )
			{
				Ref<ContentBrowserItem> item = Ref<ContentBrowserItem>::Create( entry );
				item->SetSelectedFn( SAT_BIND_EVENT_FN( ContentBrowserPanel::OnItemSelected ) );

				if( std::find( m_ValidSearchFiles.begin(), m_ValidSearchFiles.end(), item ) != m_ValidSearchFiles.end() )
				{
					if( !std::filesystem::exists( entry ) )
						m_ValidSearchFiles.erase( std::remove( m_ValidSearchFiles.begin(), m_ValidSearchFiles.end(), item ), m_ValidSearchFiles.end() );

					continue;
				}

				if( !entry.is_directory() )
				{
					if( AssetTypeFromExtension( entry.path().extension().string() ) == AssetType::Unknown )
						continue;
				}

				m_ValidSearchFiles.push_back( item );
			}
		}
	}

	void ContentBrowserPanel::Draw()
	{
		ImGui::Begin( "Content Browser" );

		if( m_ChangeDirectory )
		{
			ClearSearchQuery();
			UpdateFiles( true );

			m_ChangeDirectory = false;
		}

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::BeginChild( "Top Bar", ImVec2( 0, 30 ), false, flags );

		DrawTopBar();

		ImGui::EndChild();

		ImGui::BeginChild( "Folder Tree", ImVec2( 200, 0 ), false );

		if( Auxiliary::TreeNode( Project::GetActiveProject()->GetConfig().Name.c_str() ) )
		{
			DrawRootFolder( CBViewMode::Assets );
			DrawRootFolder( CBViewMode::Scripts );

			Auxiliary::EndTreeNode();
		}

		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild( "Folder Contents", ImVec2( 0, 0 ), false );

		// Search
		ImGui::BeginHorizontal( "##cbfinder" );

		if( m_TextFilter.DrawWithHint( "##contentfinder", "Search for content", 436.0f ) )
		{
			m_Searching = m_TextFilter.IsActive();
			BuildSearchList();
		}

		ImGui::EndHorizontal();

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.3f, 0.3f, 0.3f, 0.35f ) );

		constexpr float padding = 16.0f;
		constexpr int thumbnailSizeX = 180;
		constexpr int thumbnailSizeY = 180;
		constexpr int cellSize = thumbnailSizeX + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x - 20.0f + ImGui::GetStyle().ScrollbarSize;

		int columnCount = ( int ) ( panelWidth / cellSize );
		if( columnCount < 1 ) columnCount = 1;

		ImGui::Columns( columnCount, 0, false );

		if( m_Searching )
		{
			DrawItems( m_ValidSearchFiles, { thumbnailSizeX, thumbnailSizeY }, padding );

			// No longer searching, means that user has clicked on a file/folder.
			if( !m_Searching )
				ClearSearchQuery();
		}
		else
		{
			DrawItems( m_Files, { thumbnailSizeX, thumbnailSizeY }, padding );
		}
		
		if( !m_Searching && m_ValidSearchFiles.size() )
			m_ValidSearchFiles.clear();

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

		if( ImGui::BeginPopupContextWindow( "CB_ItemAction", ImGuiPopupFlags_MouseButtonRight ) )
		{		
			// Theses actions are only going to be used when one item is selected.
			if( m_SelectedItems.size() )
			{
				// Common Actions
				if( ImGui::MenuItem( "Rename" ) )
				{
					m_SelectedItems[ 0 ]->Rename();
				}

				// Folder Actions
				if( m_SelectedItems[ 0 ]->IsDirectory() )
				{
					if( ImGui::MenuItem( "Show In Explorer" ) )
					{
						// TODO TEMP: Create a process class.

						std::wstring CommandLine = L"";
						std::filesystem::path AssetPath = m_SelectedItems[ 0 ]->Path();
						CommandLine = std::format( L"explorer.exe \"{0}\"", AssetPath.wstring() );

						Process explorer( CommandLine );

						// Wait a bit for the process to start.
						// Usually the explorer process takes a bit to start for some reason?
						using namespace std::chrono_literals;
						std::this_thread::sleep_for( 1.5s );
					}
				}
				else
				{
					if( ImGui::MenuItem( "Delete" ) )
					{
						for( auto& rItem : m_SelectedItems )
						{
							rItem->Delete();
						}
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
		if( Auxiliary::DrawImportSoundPopup( &m_ShowSoundImport, m_CurrentPath ) )
		{
			// Popup was modified.
			// And the popup does not save the asset registry so save it here.
			AssetRegistrySerialiser ars;
			ars.Serialise( AssetManager::Get().GetAssetRegistry() );

			UpdateFiles( true );
		}

		ImGui::SetNextWindowSize( { 350.0F, 0.0F } );
		if( Auxiliary::DrawImportMeshPopup( &m_ShowMeshImport, m_CurrentPath ) )
		{
			// Popup was modified.
			// And the popup does not save the asset registry so save it here.
			AssetRegistrySerialiser ars;
			ars.Serialise( AssetManager::Get().GetAssetRegistry() );

			UpdateFiles( true );
		}

		if( s_OpenScriptsPopup )
			ImGui::OpenPopup( "Create A New Class##Create_Script" );

		ImGui::SetNextWindowSize( { 350.0F, 0.0F } );
		if( ImGui::BeginPopupModal( "Create A New Class##Create_Script", &s_OpenScriptsPopup, ImGuiWindowFlags_NoMove ) )
		{
			bool PopupModified = false;

			ImGui::BeginVertical( "##inputv" );
			ImGui::BeginHorizontal( "##inputh" );

			ImGui::Text( "Name:" );
			char buffer[ 256 ];
			memset( buffer, 0, 256 );
			memcpy( buffer, m_NewClassName.data(), m_NewClassName.length() );

			if( ImGui::InputText( "##n", buffer, 256 ) ) 
			{
				m_NewClassName = std::string( buffer );
			}
			
			ImGui::EndHorizontal();

			ImGuiIO& rIO = ImGui::GetIO();
			auto boldFont = rIO.Fonts->Fonts[ 1 ];

			ImGui::PushFont( boldFont );
			ImGui::Text( "Choose a parent class" );
			ImGui::PopFont();

			if( ImGui::BeginListBox( "##classes", ImVec2( -FLT_MIN, 0.0f ) ) )
			{
				// Root Tree
				DrawClassHierarchy( "SClass", ClassMetadataHandler::Get().GetSClassMetadata() );

				ImGui::EndListBox();
			}

			ImGui::EndVertical();

			auto drawDisabledBtn = [&]( const char* n ) -> bool
			{
				ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
				bool value = ImGui::Button( n );
				ImGui::PopStyleVar( 1 );
				ImGui::PopItemFlag();

				return value;
			};

			bool Pressed = false;
			if( m_NewClassName.empty() || m_SelectedMetadata.Name.empty() )
				Pressed = drawDisabledBtn( "Create" );
			else
				Pressed = ImGui::Button( "Create" );

			if( Pressed )
			{
				if( !Project::GetActiveProject()->HasPremakeFile() )
				{
					Project::GetActiveProject()->CreatePremakeFile();
				}

				Project::GetActiveProject()->CreateBuildFile();

				// Update or create the project files.
				Premake::Launch( Project::GetActiveProject()->GetRootDir().wstring() );

				// Next, create the source files.
				// Right now the only script type we support is an entity type.
				SourceManager::Get().CreateEntitySourceFiles( m_CurrentPath, m_NewClassName.c_str() );

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

		if( s_OpenClassInstancePopup )
			ImGui::OpenPopup( "Create A New Class Instance (Prefab)##Create_ClassIns" );

		ImGui::SetNextWindowSize( { 350.0F, 0.0F } );
		if( ImGui::BeginPopupModal( "Create A New Class Instance (Prefab)##Create_ClassIns", &s_OpenClassInstancePopup, ImGuiWindowFlags_NoMove ) )
		{
			bool PopupModified = false;

			ImGui::BeginHorizontal( "##inputH" );

			ImGui::Text( "Name:" );
			
			// I wish that we did not have to do this. But for some reason ImGui does not work well when I use a string.
			char buffer[ 256 ];
			memset( buffer, 0, 256 );
			memcpy( buffer, m_ClassInstanceName.c_str(), m_ClassInstanceName.length() );

			if( ImGui::InputText( "##instanceName", buffer, 256 ) )
			{
				m_ClassInstanceName = std::string( buffer );
			}

			ImGui::EndHorizontal();

			ImGui::Text( "Choose Parent class" );

			if( ImGui::BeginListBox( "##CLASSES_INST", ImVec2( -FLT_MIN, 0.0f ) ) ) 
			{
				// Root Tree
				DrawClassHierarchy( "SClass", ClassMetadataHandler::Get().GetSClassMetadata() );

				ImGui::EndListBox();
			}

			auto drawDisabledBtn = [&]( const char* n ) -> bool
			{
				ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
				bool value = ImGui::Button( n );
				ImGui::PopStyleVar( 1 );
				ImGui::PopItemFlag();

				return value;
			};

			if( ImGui::Button( "Create" ) )
			{
				// First, create the asset.
				Ref<Asset> prefabAsset = AssetManager::Get().FindAsset( AssetManager::Get().CreateAsset( AssetType::Prefab ) );
				Ref<Prefab> prefab = prefabAsset.As<Prefab>();
				prefab = Ref<Prefab>::Create();

				// Setup the asset
				std::filesystem::path path = m_CurrentPath / m_ClassInstanceName;
				path.replace_extension( ".prefab" );

				prefab->SetPath( path );
				prefab->Name = m_ClassInstanceName;
				prefab->ID = prefabAsset->ID;
				prefab->Type = prefabAsset->Type;
				prefab->Flags = prefabAsset->Flags;

				// Create the source entity.
				Ref<Entity> sourceEntity = nullptr;
				
				if( ClassMetadataHandler::Get().IsEngineMetadata( m_SelectedMetadata ) ) 
				{
					// TODO: Create the class somehow?
				}
				else
				{
					sourceEntity = GameModule::Get().CreateEntity( m_SelectedMetadata.Name );
				}

				sourceEntity->AddComponent<ScriptComponent>().ScriptName = m_SelectedMetadata.Name;

				prefab->Create( sourceEntity );

				// Delete the temporary source entity from the current scene.
				GActiveScene->DeleteEntity( sourceEntity );

				// Save the prefab.
				PrefabSerialiser ps;
				ps.Serialise( prefab );

				AssetManager::Get().Save();

				PopupModified = true;
			}

			if( PopupModified )
			{
				s_OpenClassInstancePopup = false;
				m_ClassInstanceName = "";

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::EndChild();

		ImGui::PopStyleColor();

		ImGui::End();
	}

	void ContentBrowserPanel::DrawClassHierarchy( const std::string& rKeyName, const SClassMetadata& rData )
	{
		ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		m_SelectedMetadata.Name == rKeyName ? Flags |= ImGuiTreeNodeFlags_Selected : 0;

		bool opened = ImGui::TreeNodeEx( rKeyName.c_str(), Flags );

		if( ImGui::IsItemClicked() )
		{
			m_SelectedMetadata = rData;
		}

		if( opened ) 
		{
			ClassMetadataHandler::Get().EachTreeNode(
				[&]( auto& rMetadata )
				{
					if( rMetadata.ParentClassName == rKeyName )
					{
						DrawClassHierarchy( rMetadata.Name, rMetadata );
					}
				} );

			Auxiliary::EndTreeNode();
		}
	}

	void ContentBrowserPanel::DrawCreateClass( const std::string& rKeyName, const SClassMetadata& rData )
	{
		if( ImGui::MenuItem( rKeyName.c_str() ) )
		{
			// In order to create this, we will need to create the class the user wants then we can create the prefab from it.

			// Create the prefab asset
			Ref<Prefab> PrefabAsset = AssetManager::Get().CreateAsset<Prefab>( AssetType::Prefab, AssetRegistryType::Game );
			PrefabAsset->Create();

			auto asset = AssetManager::Get().FindAsset( PrefabAsset->ID );

			// Create the class in the prefab scene.
			// Swap context
			Scene* OldActiveScene = Scene::GetActiveScene();
			Scene::SetActiveScene( PrefabAsset->GetScene().Get() );

			Ref<Entity> entity = GameModule::Get().CreateEntity( rData.Name );
			PrefabAsset->SetEntity( entity );
			
			// Swap back to the current scene.
			Scene::SetActiveScene( OldActiveScene );

			// Set asset path
			std::filesystem::path path = m_CurrentPath / rData.Name;
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

	void ContentBrowserPanel::ClearSearchQuery()
	{
		m_TextFilter.Clear();
		m_ValidSearchFiles.clear();
	}

	void ContentBrowserPanel::ResetPath( const std::filesystem::path& rProjectRootPath )
	{
		ClearSearchQuery();

		m_ScriptPath = rProjectRootPath / "Source";

		switch( m_ViewMode )
		{
			case Saturn::CBViewMode::Assets: 
			{
				m_CurrentViewModeDirectory = rProjectRootPath / "Assets";
			} break;

			case Saturn::CBViewMode::Scripts: 
			{
				m_CurrentViewModeDirectory = rProjectRootPath / "Source";
			} break;
		}

		m_RootPath = m_CurrentViewModeDirectory;
		m_CurrentPath = m_CurrentViewModeDirectory;
		m_FirstFolder = m_CurrentViewModeDirectory;

		delete m_Watcher;
		m_Watcher = new filewatch::FileWatch<std::string>( m_RootPath.string(),
			[this]( const std::string& path, const filewatch::Event event )
			{
				OnFilewatchEvent( path, event );
			} );

		UpdateFiles( true );
	}

	void ContentBrowserPanel::UpdateFiles( bool clear /*= false */ )
	{
		if( clear )
			m_Files.clear();

		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
		{
			Ref<ContentBrowserItem> item = Ref<ContentBrowserItem>::Create( rEntry );
			item->SetSelectedFn( SAT_BIND_EVENT_FN( ContentBrowserPanel::OnItemSelected ) );

			// Item will never exist if we have cleared the list.
			if( !clear )
			{
				if( std::find( m_Files.begin(), m_Files.end(), item ) != m_Files.end() )
				{
					if( !std::filesystem::exists( rEntry ) )
						m_Files.erase( std::remove( m_Files.begin(), m_Files.end(), item ), m_Files.end() );

					continue;
				}
			}

			if( !rEntry.is_directory() )
			{
				if( AssetTypeFromExtension( rEntry.path().extension().string() ) == AssetType::Unknown )
					continue;
			}

			m_Files.push_back( item );

			m_FilesNeedSorting = true;
		}

		SortFiles();
	}

	void ContentBrowserPanel::OnItemSelected( ContentBrowserItem* pItem, bool clicked )
	{
		if( pItem->IsDirectory() && clicked && !pItem->MultiSelected() ) 
		{
			m_CurrentPath /= pItem->Path();

			m_ChangeDirectory = true;

			ClearSelected();

			m_Searching = false;
		}
		else
		{
			if( pItem->MultiSelected() )
			{
				m_SelectedItems.push_back( pItem );
			}
			else
			{
				ClearSelected();

				m_SelectedItems.push_back( pItem );
			}
		}
	}

	void ContentBrowserPanel::DrawItems( std::vector<Ref<ContentBrowserItem>>& rList, ImVec2 size, float padding )
	{
		for( auto& item : rList )
		{
			item->Draw( { size.x, size.y }, padding );

			// This happens if we rename a file as we then have to create the file cache again.
			if( !item )
				break;

			if( !item->IsSelected() )
			{
				// Is the item in the selection list if so and we are no longer selected then we need to remove it.
				if( std::find( m_SelectedItems.begin(), m_SelectedItems.end(), item ) != m_SelectedItems.end() )
				{
					item->Deselect();

					m_SelectedItems.erase( std::remove( m_SelectedItems.begin(), m_SelectedItems.end(), item ), m_SelectedItems.end() );
				}
			}
		}
	}

}