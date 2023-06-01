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
#include "EditorLayer.h"

#include <Saturn/Project/Project.h>

#include <Saturn/ImGui/ImGuiAuxiliary.h>
#include <Saturn/Vulkan/SceneRenderer.h>
#include <Saturn/ImGui/TitleBar.h>
#include <Saturn/ImGui/MaterialAssetViewer.h>
#include <Saturn/ImGui/PrefabViewer.h>
#include <Saturn/ImGui/Panel/Panel.h>
#include <Saturn/ImGui/Panel/PanelManager.h>

#include <Saturn/Serialisation/SceneSerialiser.h>
#include <Saturn/Serialisation/ProjectSerialiser.h>
#include <Saturn/Serialisation/UserSettingsSerialiser.h>
#include <Saturn/Serialisation/AssetRegistrySerialiser.h>
#include <Saturn/Serialisation/AssetSerialisers.h>

#include <Saturn/JoltPhysics/JoltPhysicsFoundation.h>

#include <Saturn/Vulkan/MaterialInstance.h>

#include <Saturn/Core/EnvironmentVariables.h>

#include <ImGuizmo/ImGuizmo.h>

#include <Saturn/Core/Math.h>

#include <Saturn/Core/StringUtills.h>

#include <Saturn/Core/UserSettings.h>

#include <Saturn/Asset/AssetRegistry.h>
#include <Saturn/Asset/Prefab.h>

#include <Saturn/GameFramework/GameDLL.h>
#include <Saturn/GameFramework/GameManager.h>
#include <Saturn/GameFramework/EntityScriptManager.h>

#include <Saturn/Core/Renderer/RenderThread.h>

#include <Saturn/Audio/Sound2D.h>

#include <Saturn/Premake/Premake.h>

#include <typeindex>

#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	bool s_HasPremakePath = false;
	bool OpenAssetRegistryDebug = false;
	bool OpenLoadedAssetDebug = false;
	bool OpenAttributions = false;

	static inline bool operator==( const ImVec2& lhs, const ImVec2& rhs ) { return lhs.x == rhs.x && lhs.y == rhs.y; }
	static inline bool operator!=( const ImVec2& lhs, const ImVec2& rhs ) { return !( lhs == rhs ); }

	EditorLayer::EditorLayer() 
		: m_EditorCamera( 45.0f, 1280.0f, 720.0f, 0.1f, 1000.0f ), m_EditorScene( Ref<Scene>::Create() )
	{
		Scene::SetActiveScene( m_EditorScene.Pointer() );

		AssetRegistry* ar = new AssetRegistry();

		m_RuntimeScene = nullptr;
		
		// Create Panel Manager.
		m_PanelManager = Ref<PanelManager>::Create();
		
		m_PanelManager->AddPanel( new SceneHierarchyPanel() );
		m_PanelManager->AddPanel( new ContentBrowserPanel() );

		SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel *) m_PanelManager->GetPanel( "Scene Hierarchy Panel" );

		m_TitleBar = new TitleBar();

		m_TitleBar->AddMenuBarFunction( [&]() -> void
		{
			if( ImGui::BeginMenu( "File" ) )
			{
				if( ImGui::MenuItem( "Save Scene", "Ctrl+S" ) )          SaveFile();
				if( ImGui::MenuItem( "Save Scene As", "Ctrl+Shift+S" ) ) SaveFileAs();
				if( ImGui::MenuItem( "Open Scene", "Ctrl+O" ) )          OpenFile();
				
				if( ImGui::MenuItem( "Save Project" ) )                  SaveProject();
				if( ImGui::MenuItem( "Exit", "Alt+F4" ) )                Application::Get().Close();

				ImGui::EndMenu();
			}
		} );
		
		m_TitleBar->AddMenuBarFunction( []() -> void
		{
			if( ImGui::BeginMenu( "Saturn" ) )
			{
				if( ImGui::MenuItem( "Attributions" ) )
				{
					OpenAttributions = true;
				}

				ImGui::EndMenu();
			}

			if( ImGui::BeginMenu( "Project" ) )
			{
				if( ImGui::MenuItem( "Recreate project files" ) )
				{
					Project::GetActiveProject()->CreatePremakeFile();

					Premake* pPremake = new Premake();
					pPremake->Launch( Project::GetActiveProject()->GetRootDir().string() );
				}

				if( ImGui::MenuItem( "Prepare Project for Distribution" ) )
				{
					Project::GetActiveProject()->PrepForDist();
				}

				ImGui::EndMenu();
			}
		} );
		
		m_TitleBar->AddMenuBarFunction( [&]() -> void
		{
			if( ImGui::BeginMenu( "Settings" ) )
			{
				if( ImGui::MenuItem( "User settings", "" ) ) m_ShowUserSettings = !m_ShowUserSettings;
				if( ImGui::MenuItem( "Asset Registry Debug", "" ) ) OpenAssetRegistryDebug = !OpenAssetRegistryDebug;
				if( ImGui::MenuItem( "Loaded asset debug", "" ) ) OpenLoadedAssetDebug = !OpenLoadedAssetDebug;

				ImGui::EndMenu();
			}
		} );

		Window::Get().SetTitlebarHitTest( [ & ]( int x, int y ) -> bool
		{
			auto TitleBarHeight = m_TitleBar->Height();

			RECT windowRect;
			GetClientRect( glfwGetWin32Window( (GLFWwindow*)Window::Get().NativeWindow() ), &windowRect );

			// Drag the menu bar to move the window
			if( !Window::Get().Maximized() && !ImGui::IsAnyItemHovered() && ( y < ( windowRect.top + TitleBarHeight ) ) )
				return true;
			else
				return false;
		} );

		pHierarchyPanel->SetContext( m_EditorScene );
		pHierarchyPanel->SetSelectionChangedCallback( SAT_BIND_EVENT_FN( EditorLayer::SelectionChanged ) );

		m_EditorCamera.SetActive( true );
		
		m_CheckerboardTexture = Ref< Texture2D >::Create( "content/textures/editor/checkerboard.tga", AddressingMode::Repeat );

		m_StartRuntimeTexture = Ref< Texture2D >::Create( "content/textures/editor/Play.png", AddressingMode::Repeat );
		m_EndRuntimeTexture   = Ref< Texture2D >::Create( "content/textures/editor/Stop.png", AddressingMode::Repeat );

		m_TranslationTexture  = Ref< Texture2D >::Create( "content/textures/editor/Move.png", AddressingMode::Repeat );
		m_RotationTexture     = Ref< Texture2D >::Create( "content/textures/editor/Rotate.png", AddressingMode::Repeat );
		m_ScaleTexture        = Ref< Texture2D >::Create( "content/textures/editor/Scale.png", AddressingMode::Repeat );
		m_SyncTexture         = Ref< Texture2D >::Create( "content/textures/editor/Sync.png", AddressingMode::Repeat );

		// Init Physics
		JoltPhysicsFoundation::Get().Init();

		ContentBrowserPanel* pContentBrowserPanel = ( ContentBrowserPanel* ) m_PanelManager->GetPanel( "Content Browser Panel" );

		auto& rUserSettings = GetUserSettings();

		pContentBrowserPanel->SetPath( rUserSettings.StartupProject );

		ProjectSerialiser ps;
		ps.Deserialise( rUserSettings.FullStartupProjPath.string() );

		if( !Project::GetActiveProject() )
			SAT_CORE_ASSERT( false, "No project was given." );

		Project::GetActiveProject()->LoadAssetRegistry();
		Project::GetActiveProject()->CheckMissingAssetRefs();

		// Lazy load.
		// TODO: We should not lazy load something this important.
		EntityScriptManager::Get();

		GameDLL* pGameDLL = new GameDLL();
		pGameDLL->Load();

		GameManager* pGameManager = new GameManager();

		OpenFile( Project::GetActiveProject()->GetConfig().StartupScenePath );

		s_HasPremakePath = Auxiliary::HasEnvironmentVariable( "SATURN_PREMAKE_PATH" );

		/*
		Ref<Asset> asset = AssetRegistry::Get().FindAsset( "Assets\\Sound\\Music_MainThemePiano.s2d" );
		Ref<Sound2D> music = AssetRegistry::Get().GetAssetAs<Sound2D>( asset->ID );
		music->Loop();
		music->TryPlay();
		*/
	}

	EditorLayer::~EditorLayer()
	{
		Window::Get().SetTitlebarHitTest( nullptr );
		
		delete m_TitleBar;

		m_EditorScene = nullptr;
		
		m_CheckerboardTexture = nullptr;
		
		m_TitleBar = nullptr;
	
		m_PanelManager = nullptr;
		JoltPhysicsFoundation::Get().Terminate();
	}

	void EditorLayer::OnUpdate( Timestep time )
	{
		SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) m_PanelManager->GetPanel( "Scene Hierarchy Panel" );
		
		if( m_RequestRuntime )
		{
			if( !m_RuntimeScene )
			{
				m_RuntimeScene = Ref<Scene>::Create();

				m_EditorScene->CopyScene( m_RuntimeScene );

				EntityScriptManager::Get().SetCurrentScene( m_RuntimeScene );
				EntityScriptManager::Get().TransferEntities( m_EditorScene );

				m_RuntimeScene->OnRuntimeStart();

				pHierarchyPanel->SetContext( m_RuntimeScene );

				m_RuntimeScene->m_RuntimeRunning = true;

				Scene::SetActiveScene( m_RuntimeScene.Pointer() );
				Application::Get().PrimarySceneRenderer().SetCurrentScene( m_RuntimeScene.Pointer() );
			}
		}
		else
		{
			if( m_RuntimeScene && m_RuntimeScene->m_RuntimeRunning )
			{
				m_RuntimeScene->OnRuntimeEnd();
				EntityScriptManager::Get().DestroyEntityInScene( m_RuntimeScene );

				m_RuntimeScene = nullptr;

				pHierarchyPanel->SetContext( m_EditorScene );

				Scene::SetActiveScene( m_EditorScene.Pointer() );
				EntityScriptManager::Get().SetCurrentScene( m_EditorScene );
				Application::Get().PrimarySceneRenderer().SetCurrentScene( m_EditorScene.Pointer() );
			}
		}

		if( m_RuntimeScene ) 
		{
			m_RuntimeScene->OnUpdate( time );
			m_RuntimeScene->OnRenderRuntime( time, Application::Get().PrimarySceneRenderer() );
		}
		else 
		{
			m_EditorCamera.SetActive( m_AllowCameraEvents );
			m_EditorCamera.OnUpdate( time );

			m_EditorScene->OnUpdate( time );
			m_EditorScene->OnRenderEditor( m_EditorCamera, time, Application::Get().PrimarySceneRenderer() );
		}

		if( Input::Get().MouseButtonPressed( Mouse::Right ) && !m_StartedRightClickInViewport && m_ViewportFocused && m_MouseOverViewport )
			m_StartedRightClickInViewport = true;

		if( !Input::Get().MouseButtonPressed( Mouse::Right ) )
			m_StartedRightClickInViewport = false;

		// Render scenes in other asset viewers
		AssetViewer::Update( time );
	}

	void EditorLayer::OnImGuiRender()
	{
		// Draw dockspace.
		ImGuiIO& io = ImGui::GetIO();
		ImGuiViewport* pViewport = ImGui::GetWindowViewport();
		auto Height = ImGui::GetFrameHeight();

		ImGui::DockSpaceOverViewport( pViewport );
		
		if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) || ( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) && !m_StartedRightClickInViewport ) )
		{
			if( !m_RuntimeScene )
			{
				ImGui::FocusWindow( GImGui->HoveredWindow );
				Input::Get().SetCursorMode( CursorMode::Normal );
			}
		}

		io.ConfigWindowsResizeFromEdges = io.BackendFlags & ImGuiBackendFlags_HasMouseCursors;

		m_TitleBar->Draw();
		AssetViewer::Draw();

		m_PanelManager->DrawAllPanels();
		Application::Get().PrimarySceneRenderer().ImGuiRender();

		if( m_ShowUserSettings )
			UI_Titlebar_UserSettings();

		if( OpenAssetRegistryDebug ) 
		{
			if( ImGui::Begin( "AssetRegistry", &OpenAssetRegistryDebug ) )
			{
				static ImGuiTextFilter Filter;

				ImGui::Text( "Search" );
				ImGui::SameLine();
				Filter.Draw( "##search" );

				for ( auto&& [id, asset] : AssetRegistry::Get().GetAssetMap() )
				{
					if( !Filter.PassFilter( asset->GetName().c_str() ) )
						continue;

					ImGui::Selectable( asset->GetName().c_str(), false );
					ImGui::SameLine();
					ImGui::Selectable( std::to_string( id ).c_str(), false );
					ImGui::SameLine();
					ImGui::Selectable( AssetTypeToString( asset->GetAssetType() ).c_str(), false );
				}

				ImGui::End();
			}
		}

		if( OpenLoadedAssetDebug ) 
		{
			if( ImGui::Begin( "Loaded Assets", &OpenLoadedAssetDebug ) )
			{
				static ImGuiTextFilter Filter;

				ImGui::Text( "Search" );
				ImGui::SameLine();
				Filter.Draw( "##search" );

				for( auto&& [id, asset] : AssetRegistry::Get().GetLoadedAssetsMap() )
				{
					if( !Filter.PassFilter( asset->GetName().c_str() ) )
						continue;

					ImGui::Selectable( asset->GetName().c_str(), false );
					ImGui::SameLine();
					ImGui::Selectable( std::to_string( id ).c_str(), false );
					ImGui::SameLine();
					ImGui::Selectable( AssetTypeToString( asset->GetAssetType() ).c_str(), false );
				}

				ImGui::End();
			}
		}

		if( OpenAttributions )
		{
			if( ImGui::Begin( "Attributions", &OpenAttributions ) )
			{
				ImGui::Text("All icons in the engine are provided by icons8 via https://icons8.com/\nUsing the Tanah Basah set.");

				ImGui::End();
			}
		}

		ImGui::Begin( "Renderer" );

		ImGui::Text( "Frame Time: %.2f ms", Application::Get().Time().Milliseconds() );

		for( const auto& devices : VulkanContext::Get().GetPhysicalDeviceProperties() )
		{
			ImGui::Text( "Device Name: %s", devices.DeviceProps.deviceName );
			ImGui::Text( "API Version: %i", devices.DeviceProps.apiVersion );
			ImGui::Text( "Vendor ID: %i", devices.DeviceProps.vendorID );
			ImGui::Text( "Vulkan Version: 1.2.128" );
		}
		
		ImGui::End();
	
		ImGui::Begin( "Materials" );

		SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel *) m_PanelManager->GetPanel( "Scene Hierarchy Panel" );

		if( auto& rSelection = pHierarchyPanel->GetSelectionContext() )
		{
			if( rSelection.HasComponent<StaticMeshComponent>() )
			{
				if( auto& mesh = rSelection.GetComponent<StaticMeshComponent>().Mesh )
				{
					ImGui::TextDisabled( "%llx", rSelection.GetComponent<IdComponent>().ID );

					ImGui::Separator();

					for( auto& rMaterial : mesh->GetMaterialAssets() )
					{
						if( ImGui::CollapsingHeader( rMaterial->GetName().c_str() ) ) 
						{
							ImGui::PushID( static_cast<int>( rMaterial->GetAssetID() ) );

							ImGui::Text( "Mesh name: %s", mesh->FilePath().c_str() );
							ImGui::Text( "Asset ID: %llu", (uint64_t)rMaterial->GetAssetID() );
							
							ImGui::Separator();

							UUID id = rMaterial->GetAssetID();
							Auxiliary::DrawAssetDragDropTarget<MaterialAsset>( "Change asset", rMaterial->GetName().c_str(), id, 
								[rMaterial](Ref<MaterialAsset> asset) mutable
								{
									rMaterial->SetMaterial( asset->GetMaterial() );
								} );

							ImGui::Separator();

							auto drawItemValue = [&]( const char* name, const char* property )
							{
								ImGui::Text( name );

								ImGui::Separator();

								float v = rMaterial->Get< float >( property );

								ImGui::PushID( name );

								ImGui::DragFloat( "##drgflt", &v, 0.01f, 0.0f, 10000.0f );

								ImGui::PopID();

								if( v != rMaterial->Get<float>( property ) )
									rMaterial->Set( property, v );
							};

							auto displayItemMap = [&]( const char* property ) 
							{
								Ref< Texture2D > v = rMaterial->GetResource( property );

								if( v )
								{
									if( v && v->GetDescriptorSet() )
										ImGui::Image( v->GetDescriptorSet(), ImVec2( 100, 100 ) );
									else
										ImGui::Image( m_CheckerboardTexture->GetDescriptorSet(), ImVec2( 100, 100 ) );
								}
							};

							ImGui::Text( "Albedo" );

							ImGui::Separator();

							displayItemMap( "u_AlbedoTexture" );

							ImGui::SameLine();

							if( ImGui::Button( "...##opentexture", ImVec2( 50, 20 ) ) )
							{
								Application::Get().SubmitOnMainThread( [&]() 
									{
										std::string file = Application::Get().OpenFile( "Texture File (*.png *.tga)\0*.tga; *.png\0" );

										if( !file.empty() )
										{
											rMaterial->SetResource( "u_AlbedoTexture", Ref<Texture2D>::Create( file, AddressingMode::Repeat ) );
										}
									} );
							}

							glm::vec3 color = rMaterial->Get<glm::vec3>( "u_Materials.AlbedoColor" );

							bool changed = ImGui::ColorEdit3( "##Albedo Color", glm::value_ptr( color ), ImGuiColorEditFlags_NoInputs );

							if( changed )
								rMaterial->Set<glm::vec3>( "u_Materials.AlbedoColor", color );

							drawItemValue( "Emissive", "u_Materials.Emissive" );

							ImGui::Text( "Normal" );

							ImGui::Separator();

							bool UseNormalMap = rMaterial->Get< float >( "u_Materials.UseNormalMap" );

							if( UseNormalMap )
								displayItemMap( "u_NormalTexture" );

							if( ImGui::Checkbox( "Use Normal Map", &UseNormalMap ) )
								rMaterial->Set( "u_Materials.UseNormalMap", UseNormalMap ? 1.0f : 0.0f );
							
							// Roughness Value
							drawItemValue( "Roughness", "u_Materials.Roughness" );

							// Roughness map
							displayItemMap( "u_RoughnessTexture" );

							// Metalness value
							drawItemValue( "Metalness", "u_Materials.Metalness" );
							
							// Metalness map
							displayItemMap( "u_MetallicTexture" );

							ImGui::PopID();
						}
					}
				}
			}
		}

		if( !s_HasPremakePath )
		{
			if( ImGui::BeginPopupModal( "Missing Environment Variable", NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
			{
				ImGui::Text( "The environment variable SATURN_PREMAKE_PATH is not set." );
				ImGui::Text( "This is required in order to build projects." );

				ImGui::Separator();

				static std::string path = "";

				if( path.empty() )
					path = "";

				ImGui::InputText( "##path", ( char* ) path.c_str(), 1024, ImGuiInputTextFlags_ReadOnly );
				ImGui::SameLine();
				if( ImGui::Button( "..." ) ) 
				{
					if( RenderThread::Get().IsRenderThread() )
					{
						Application::Get().SubmitOnMainThread( [=]()
							{
								path = Application::Get().OpenFile( ".exe\0*.exe;\0" );
							} );
					}
				}

				if( !path.empty() )
				{
					if( ImGui::Button( "Close" ) )
					{
						ImGui::CloseCurrentPopup();
						
						Auxiliary::SetEnvironmentVariable( "SATURN_PREMAKE_PATH", path.c_str() );

						s_HasPremakePath = true;
					}
				}

				ImGui::EndPopup();
			}

			ImGui::OpenPopup( "Missing Environment Variable" );
		}

		ImGui::End();

		// Viewport
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;

		ImGui::Begin( "Viewport", 0, flags );

		if( m_ViewportSize != ImGui::GetContentRegionAvail() )
		{
			m_ViewportSize = ImGui::GetContentRegionAvail();

			Application::Get().PrimarySceneRenderer().SetViewportSize( ( uint32_t ) m_ViewportSize.x, ( uint32_t ) m_ViewportSize.y );
			m_EditorCamera.SetViewportSize( ( uint32_t ) m_ViewportSize.x, ( uint32_t ) m_ViewportSize.y );
		}

		ImGui::PushID( "VIEWPORT_IMAGE" );

		// In the editor we only should flip the image UV, we don't have to flip anything else.
		Auxiliary::Image( Application::Get().PrimarySceneRenderer().CompositeImage(), m_ViewportSize, { 0, 1 }, { 1, 0 } );

		if( ImGui::BeginDragDropTarget() )
		{
			if( auto payload = ImGui::AcceptDragDropPayload( "CONTENT_BROWSER_ITEM_SCENE" ) )
			{
				const wchar_t* path = ( const wchar_t* ) payload->Data;
				OpenFile( path );
			}

			if( auto payload = ImGui::AcceptDragDropPayload( "CONTENT_BROWSER_ITEM_PREFAB" ) )
			{
				const wchar_t* path = ( const wchar_t* ) payload->Data;

				std::filesystem::path p = path;

				Ref<Asset> asset = AssetRegistry::Get().FindAsset( p );
				// Make sure to load the prefab.
				Ref<Prefab> prefabAsset = AssetRegistry::Get().GetAssetAs<Prefab>( asset->GetAssetID() );

				m_EditorScene->CreatePrefab( prefabAsset );
			}

			if( auto payload = ImGui::AcceptDragDropPayload( "CONTENT_BROWSER_ITEM_MODEL" ) )
			{
				const wchar_t* path = ( const wchar_t* ) payload->Data;
				
				std::filesystem::path p = path;

				// We have now that path to the *.stmesh but we need to path to the fbx/gltf.

				Ref<Asset> asset = AssetRegistry::Get().FindAsset( p );
				Ref<StaticMesh> meshAsset = AssetRegistry::Get().GetAssetAs<Prefab>( asset->GetAssetID() );
				
				auto entity = m_EditorScene->CreateEntity( asset->Name );
				entity.AddComponent<StaticMeshComponent>().Mesh = meshAsset;
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::PopID();

		ImVec2 minBound = ImGui::GetWindowPos();
		ImVec2 maxBound = { minBound.x + m_ViewportSize.x, minBound.y + m_ViewportSize.y };

		// Viewport Gizmo toolbar
		ImGui::PushID( "VP_GIZMO" );
		
		const float windowHeight = 32.0f;
		const float windowWidth = 200.0f;

		ImGui::SetNextWindowPos ( ImVec2( minBound.x + 5.0f, minBound.y + 5.0f ) );
		ImGui::SetNextWindowSize( ImVec2( windowWidth, windowHeight ) );
		ImGui::Begin( "##viewport_tools", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking );

		ImGui::BeginVertical  ( "##v_gizmoV", { windowWidth, ImGui::GetContentRegionAvail().y } );
		ImGui::BeginHorizontal( "##v_gizmoH", { windowWidth, ImGui::GetContentRegionAvail().y } );

		ImGui::PushStyleColor( ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f } );
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 5.0f * 2.0f, 0 ) );

		const Ref<Texture2D>& texture = m_RequestRuntime == false ? m_StartRuntimeTexture : m_EndRuntimeTexture;
		if( Auxiliary::ImageButton( texture, { 24.0f, 24.0f } ) ) m_RequestRuntime ^= 1;
		if( Auxiliary::ImageButton( m_TranslationTexture, { 24.0f, 24.0f } ) ) m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		if( Auxiliary::ImageButton( m_RotationTexture, { 24.0f, 24.0f } ) ) m_GizmoOperation = ImGuizmo::OPERATION::ROTATE;
		if( Auxiliary::ImageButton( m_ScaleTexture, { 24.0f, 24.0f } ) ) m_GizmoOperation = ImGuizmo::OPERATION::SCALE;

		// Hot-Reload game
		if( Auxiliary::ImageButton( m_SyncTexture, { 24.0f, 24.0f } ) ) HotReloadGame();

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		ImGui::Spring();
		ImGui::EndHorizontal();
		ImGui::Spring();
		ImGui::EndVertical();

		ImGui::End();

		ImGui::PopID();

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_MouseOverViewport = ImGui::IsWindowHovered();

		m_AllowCameraEvents = ImGui::IsMouseHoveringRect( minBound, maxBound ) && m_ViewportFocused || m_StartedRightClickInViewport;

		Entity selectedEntity = pHierarchyPanel->GetSelectionContext();

		if( selectedEntity && m_GizmoOperation != -1 )
		{
			if( !selectedEntity.HasComponent<SkylightComponent>() )
			{
				ImGuizmo::SetOrthographic( false );
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect( minBound.x, minBound.y, m_ViewportSize.x, m_ViewportSize.y );

				auto& tc = selectedEntity.GetComponent<TransformComponent>();

				glm::mat4 transform = m_RuntimeScene ? m_RuntimeScene->GetTransformRelativeToParent( selectedEntity ) : m_EditorScene->GetTransformRelativeToParent( selectedEntity );

				const glm::mat4 Projection = m_EditorCamera.ProjectionMatrix();
				const glm::mat4 View = m_EditorCamera.ViewMatrix();

				ImGuizmo::Manipulate( glm::value_ptr( View ), glm::value_ptr( Projection ), ( ImGuizmo::OPERATION ) m_GizmoOperation, ImGuizmo::LOCAL, glm::value_ptr( transform ) );

				if( ImGuizmo::IsUsing() )
				{
					glm::vec3 translation;
					glm::vec3 rotation;
					glm::vec3 scale;

					Math::DecomposeTransform( transform, translation, rotation, scale );

					glm::vec3 DeltaRotation = rotation - tc.Rotation;

					tc.Position = translation;
					tc.Rotation += DeltaRotation;
					tc.Scale = scale;
				}
			}
		}

		ImGui::PopStyleVar();
		ImGui::End();
	}

	void EditorLayer::OnEvent( Event& rEvent )
	{
		EventDispatcher dispatcher( rEvent );
		dispatcher.Dispatch<KeyPressedEvent>( SAT_BIND_EVENT_FN( EditorLayer::OnKeyPressed ) );

		if( m_MouseOverViewport )
			m_EditorCamera.OnEvent( rEvent );

		AssetViewer::ProcessEvent( rEvent );
	}

	void EditorLayer::SaveFileAs()
	{
		if( RenderThread::Get().IsRenderThread() )
		{
			Application::Get().SubmitOnMainThread( [=]()
				{
					auto res = Application::Get().SaveFile( "Saturn Scene file (*.scene, *.sc)\0*.scene; *.sc\0" );

					SceneSerialiser serialiser( m_EditorScene );
					serialiser.Serialise( res );
				} );
		}
		else
		{
			auto res = Application::Get().SaveFile( "Saturn Scene file (*.scene, *.sc)\0*.scene; *.sc\0" );

			SceneSerialiser serialiser( m_EditorScene );
			serialiser.Serialise( res );
		}
	}

	void EditorLayer::SaveFile()
	{
		if( std::filesystem::exists( m_EditorScene->Filepath() ) )
		{
			SceneSerialiser ss( m_EditorScene );
			ss.Serialise( m_EditorScene->Filepath() );
		}
		else
		{
			SaveFileAs();
		}
	}

	void EditorLayer::OpenFile( const std::filesystem::path& rFilepath )
	{
		if( rFilepath.empty() )
			return;

		SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) m_PanelManager->GetPanel( "Scene Hierarchy Panel" );

		Ref<Scene> newScene = Ref<Scene>::Create();
		EntityScriptManager::Get().SetCurrentScene( newScene );

		pHierarchyPanel->SetSelected( {} );

		auto fullPath = Project::GetActiveProject()->FilepathAbs( rFilepath );

		SceneSerialiser serialiser( newScene );
		serialiser.Deserialise( fullPath.string() );

		m_EditorScene = newScene;

		// We maybe don't need to transfer the entities but just to be sure we will do it.
		EntityScriptManager::Get().SetCurrentScene( m_EditorScene );
		EntityScriptManager::Get().TransferEntities( newScene );

		newScene = nullptr;

		pHierarchyPanel->SetContext( m_EditorScene );

		Application::Get().PrimarySceneRenderer().SetCurrentScene( m_EditorScene.Pointer() );
	}

	void EditorLayer::OpenFile()
	{
		if( RenderThread::Get().IsRenderThread() )
		{
			Application::Get().SubmitOnMainThread( [=]() 
				{
					auto res = Application::Get().OpenFile( "Saturn Scene file (*.scene, *.sc)\0*.scene; *.sc\0" );
					OpenFile( res );
				} );
		}
		else
		{
			auto res = Application::Get().OpenFile( "Saturn Scene file (*.scene, *.sc)\0*.scene; *.sc\0" );
			OpenFile( res );
		}
	}

	void EditorLayer::SaveProject()
	{
		ProjectSerialiser ps( Project::GetActiveProject() );
		ps.Serialise( Project::GetActiveProject()->m_Config.Path );
	}

	void EditorLayer::SelectionChanged( Entity e )
	{
	}

	void EditorLayer::ViewportSizeCallback( uint32_t Width, uint32_t Height )
	{

	}

	bool EditorLayer::OnKeyPressed( KeyPressedEvent& rEvent )
	{
		switch( rEvent.KeyCode() )
		{
			case Key::Delete:
			{
				SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) m_PanelManager->GetPanel( "Scene Hierarchy Panel" );

				if( pHierarchyPanel )
				{
					if( auto& rEntity = pHierarchyPanel->GetSelectionContext() )
					{
						m_EditorScene->DeleteEntity( rEntity );
						pHierarchyPanel->SetSelected( {} );
					}
				}
			} break;

			case Key::Q:
				m_GizmoOperation = -1;
				break;
			case Key::W:
				m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Key::E:
				m_GizmoOperation = ImGuizmo::OPERATION::ROTATE;
				break;
			case Key::R:
				m_GizmoOperation = ImGuizmo::OPERATION::SCALE;
				break;
		}

		if( Input::Get().KeyPressed( Key::LeftControl ) )
		{
			switch( rEvent.KeyCode() )
			{
				case Key::D:
				{
					SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) m_PanelManager->GetPanel( "Scene Hierarchy Panel" );
					
					if( pHierarchyPanel ) 
					{
						if( auto& rEntity = pHierarchyPanel->GetSelectionContext() )
						{
							m_EditorScene->DuplicateEntity( rEntity );
						}
					}

				} break;

				case Key::F:
				{
					SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) m_PanelManager->GetPanel( "Scene Hierarchy Panel" );

					if( auto& selectedEntity = pHierarchyPanel->GetSelectionContext() ) 
					{
						m_EditorCamera.Focus( selectedEntity.GetComponent<TransformComponent>().Position );
					}
				} break;

				case Key::O:
				{
					OpenFile();
				} break;

				case Key::S:
				{
					SaveFile();
				} break;
			}

			if( Input::Get().KeyPressed( Key::LeftShift ) ) 
			{
				switch( rEvent.KeyCode() )
				{
					case Key::S:
					{
						SaveFileAs();
					} break;
				}
			}
		}

		return true;
	}

	static bool s_OpenAssetFinderPopup = false;
	static AssetID s_AssetFinderID = 0;

	void EditorLayer::UI_Titlebar_UserSettings()
	{
		auto& userSettings = GetUserSettings();

		auto& startupProject = userSettings.StartupProject;
		auto& startupScene = Project::GetActiveProject()->m_Config.StartupScenePath;

		ImGuiIO& rIO = ImGui::GetIO();

		ImGui::SetNextWindowPos( ImVec2( rIO.DisplaySize.x * 0.5f - 150.0f, rIO.DisplaySize.y * 0.5f - 150.0f ), ImGuiCond_Once );

		ImGui::Begin( "User settings", &m_ShowUserSettings );

		ImGui::SetCursorPosX( ImGui::GetWindowContentRegionWidth() * 0.5f - ImGui::CalcTextSize( "User settings" ).x * 0.5f );
		ImGui::Text( "User settings" );

		ImGui::Separator();

		ImGui::Text( "Startup project:" );
		ImGui::SameLine();
		startupProject.empty() ?  ImGui::Text( "None" ) : ImGui::Text( startupProject.c_str() );
		ImGui::SameLine();
		if( ImGui::Button( "...##openprj" ) )
		{
			if( RenderThread::Get().IsRenderThread() )
			{
				Application::Get().SubmitOnMainThread( [&]()
					{
						startupProject = Application::Get().OpenFile( "Saturn project file (*.scene) (*.sc)\0*.scene\0" );
					} );
			}
		}

		ImGui::Text( "Startup Scene:" );
		
		if( s_OpenAssetFinderPopup )
			ImGui::OpenPopup( "AssetFinderPopup" );
		
		ImGui::SameLine();
		startupScene.empty() ? ImGui::Text( "None" ) : ImGui::Text( startupScene.c_str() );
		ImGui::SameLine();
		
		if( ImGui::Button( "...##scene" ) )
			s_OpenAssetFinderPopup = true;

		ImGui::SetNextWindowSize( { 250.0f, 0.0f } );
		if( ImGui::BeginPopup( "AssetFinderPopup", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize ) )
		{
			bool PopupModified = false;

			if( ImGui::BeginListBox( "##ASSETLIST", ImVec2( -FLT_MIN, 0.0f ) ) )
			{
				for( const auto& [assetID, rAsset] : AssetRegistry::Get().GetAssetMap() )
				{
					bool Selected = ( s_AssetFinderID == assetID );

					if( rAsset->GetAssetType() == AssetType::Scene || rAsset->GetAssetType() == AssetType::Unknown )
					{
						if( ImGui::Selectable( rAsset->GetName().c_str() ) )
						{
							Project::GetActiveProject()->m_Config.StartupScenePath = std::filesystem::relative( rAsset->GetPath(), Project::GetActiveProject()->GetRootDir() ).string();
							
							PopupModified = true;
						}
					}

					if( Selected )
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndListBox();
			}

			if( PopupModified )
			{
				s_OpenAssetFinderPopup = false;

				ProjectSerialiser ps;
				ps.Serialise( Project::GetActiveProject()->GetRootDir().string() );

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		
		ImGui::End();
	}

	void EditorLayer::HotReloadGame()
	{
		SAT_CORE_ASSERT(false, "EditorLayer::HotReloadGame not implemented.");
	}

}