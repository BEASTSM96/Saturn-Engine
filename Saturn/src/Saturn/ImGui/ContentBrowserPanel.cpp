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
#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Vulkan/Mesh.h"

#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include "Saturn/Asset/Prefab.h"
#include "Saturn/Audio/Sound2D.h"

#include "Saturn/Premake/Premake.h"
#include "Saturn/GameFramework/SourceManager.h"
#include "Saturn/GameFramework/GamePrefabList.h"
#include "Saturn/GameFramework/EntityScriptManager.h"

#include <imgui_internal.h>

namespace Saturn {
	
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

			if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
			{
				// TODO: Think about this...

				auto path = std::filesystem::relative( entry.path(), s_pAssetsDirectory );

				m_CurrentPath = s_pAssetsDirectory;
				m_CurrentPath /= path;

				OnDirectorySelected( m_CurrentPath, entry.is_directory() );
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
			if( ImGui::MenuItem( "Material" ) )
			{
				auto id = AssetManager::Get().CreateAsset( AssetType::Material );
				auto asset = AssetManager::Get().FindAsset( id );

				asset->SetPath( m_CurrentPath / "Untitled Material.smaterial" );
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
			}

			if( ImGui::MenuItem( "Empty Scene" ) )
			{
				// TODO:
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
					EntityScriptManager::Get().RegisterScript( name );

					Entity* e = new Entity( PrefabAsset->GetScene()->CreateEntity( name ) );
					e->AddComponent<ScriptComponent>().ScriptName = name;

					SClass* sclass = EntityScriptManager::Get().CreateScript( name, e );

					PrefabAsset->SetEntity( *( Entity* ) &e );

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

		/*
		if( ImGui::BeginPopupContextWindow( 0, 1, false ) )
		{
			if( m_ViewMode == CBViewMode::Assets )
			{
				AssetsPopupContextMenu();
			}
			else
			{
				ScriptsPopupContextMenu();
			}

			ImGui::EndPopup();
		}
		*/

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
			static bool s_UseBinFile = false;

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

		// Draw folder item.
		if( rEntry.is_directory() )
		{
			bool Hovered = false;
			bool Clicked = false;
			bool RightClicked = false;

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

				if( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
				{
					RightClicked = true;
				}
			}
		}
		else // Draw file item.
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
						case AssetType::Texture:
							break;

						case AssetType::StaticMesh:
						{
							auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );

							// Find the asset.
							Ref<Asset> asset = AssetManager::Get().FindAsset( path );
							AssetViewer::Add<StaticMeshAssetViewer>( asset->ID );
						} break;

						case AssetType::SkeletalMesh:
							break;

						case AssetType::Material:
						{
							auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );

							// Find the asset.
							Ref<Asset> asset = AssetManager::Get().FindAsset( path );

							// Importing the asset will happen in this function.
							AssetViewer::Add<MaterialAssetViewer>( asset->ID );
						} break;
						case AssetType::MaterialInstance:
							break;

						case AssetType::Prefab:
						{
							auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );

							Ref<Asset> asset = AssetManager::Get().FindAsset( path );

							AssetViewer::Add<PrefabViewer>( asset->ID );
						} break;

						case AssetType::PhysicsMaterial:
						{
							auto path = std::filesystem::relative( rEntry.path(), Project::GetActiveProject()->GetRootDir() );

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
		}

		if( ImGui::BeginPopupContextWindow( 0, 1, true ) )
		{
			// Common Actions
			if( ImGui::MenuItem( "Rename" ) ) 
			{

			}

			// Folder Actions
			if( rEntry.is_directory() )
			{
				if( ImGui::MenuItem( "Show In Explorer" ) )
				{
				}
			}
			else
			{
				if( ImGui::MenuItem( "Delete" ) )
				{
				}
			}

			ImGui::EndPopup();
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

		UpdateFiles( true );
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