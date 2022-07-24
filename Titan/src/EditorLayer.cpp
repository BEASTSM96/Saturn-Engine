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
#include "EditorLayer.h"

#include <Saturn/ImGui/ViewportBar.h>
#include <Saturn/Vulkan/SceneRenderer.h>
#include <Saturn/ImGui/TitleBar.h>

#include <Saturn/ImGui/Panel/Panel.h>
#include <Saturn/ImGui/Panel/PanelManager.h>

#include <Saturn/Serialisation/SceneSerialiser.h>
#include <Saturn/PhysX/PhysXFnd.h>

#include <Saturn/Vulkan/MaterialInstance.h>

#include <Saturn/Core/EnvironmentVariables.h>

#include <glm/gtc/type_ptr.hpp>


namespace Saturn {

	EditorLayer::EditorLayer() 
		: m_EditorCamera( glm::radians( 45.0f ), 1280.0f / 720.0f, 0.1f, 1000.0f )
	{
		m_EditorScene = Ref<Scene>::Create();
		m_RuntimeScene = nullptr;
		
		// Create Panel Manager.
		PanelManager::Get();
		
		PanelManager::Get().AddPanel( new SceneHierarchyPanel() );
		PanelManager::Get().AddPanel( new ContentBrowserPanel() );
		PanelManager::Get().AddPanel( new ViewportBar() );
		
		SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel *)PanelManager::Get().GetPanel( "Scene Hierarchy Panel" );

		m_Viewport = new Viewport();
		m_TitleBar = new TitleBar();

		m_TitleBar->AddMenuBarFunction( [&]() 
		{
			if( ImGui::BeginMenu( "File" ) )
			{
				if( ImGui::MenuItem( "Exit", "Alt+F4" ) ) Application::Get().Close();
				if( ImGui::MenuItem( "Save", "Ctrl+S" ) ) SaveFile();
				if( ImGui::MenuItem( "Open", "Ctrl+O" ) ) OpenFile();

				ImGui::EndMenu();
			}
		} );
		
		m_TitleBar->AddMenuBarFunction( []()
		{
			if( ImGui::BeginMenu( "Saturn" ) )
			{
				if( ImGui::MenuItem( "Environment Variables" ) )
				{
					if( ImGui::BeginPopupModal( "##Saturn", NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
					{
						ImGui::EndPopup();
					}

					ImGui::OpenPopup( "##Saturn" );
				}

				ImGui::EndMenu();
			}
		} );

		m_Viewport->AddViewportSizeFunction( [&]( uint32_t w, uint32_t h ) -> void
		{
			SceneRenderer::Get().SetViewportSize( w, h );

			m_EditorCamera.SetViewportSize( w, h );
		} );

		pHierarchyPanel->SetContext( m_EditorScene );
		pHierarchyPanel->SetSelectionChangedCallback( SAT_BIND_EVENT_FN( EditorLayer::SelectionChanged ) );

		m_EditorCamera.AllowEvents( true );
		m_EditorCamera.SetActive( true );
		
		m_CheckerboardTexture = Ref< Texture2D >::Create( "assets/textures/editor/checkerboard.tga", AddressingMode::Repeat );

		PhysXFnd::Get();
	}

	EditorLayer::~EditorLayer()
	{
		delete m_Viewport;
		delete m_TitleBar;

		m_EditorScene = nullptr;
		
		m_CheckerboardTexture = nullptr;
		
		m_Viewport = nullptr;
		m_TitleBar = nullptr;

		PanelManager::Get().Terminate();
	}

	void EditorLayer::OnUpdate( Timestep time )
	{
		if( m_Viewport->m_SendCameraEvents )
			m_EditorCamera.OnUpdate( time );

		SceneRenderer::Get().SetEditorCamera( m_EditorCamera );

		ViewportBar* pViewportBar = ( ViewportBar* ) PanelManager::Get().GetPanel( "Viewport Bar" );
		SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) PanelManager::Get().GetPanel( "Scene Hierarchy Panel" );
		
		if( pViewportBar->RequestedStartRuntime() )
		{
			if( !m_RuntimeScene )
			{
				m_RuntimeScene = Ref<Scene>::Create();

				m_EditorScene->CopyScene( m_RuntimeScene );

				m_RuntimeScene->OnRuntimeStart();

				pHierarchyPanel->SetContext( m_RuntimeScene );

				m_RuntimeScene->m_RuntimeRunning = true;
			}
		}
		else
		{
			if( m_RuntimeScene && m_RuntimeScene->m_RuntimeRunning )
			{
				m_RuntimeScene->OnRuntimeEnd();

				m_RuntimeScene = nullptr;

				pHierarchyPanel->SetContext( m_EditorScene );
			}
		}

		if( m_RuntimeScene ) 
		{
			m_RuntimeScene->OnUpdate( Application::Get().Time() );
			m_RuntimeScene->OnRenderEditor( m_EditorCamera, Application::Get().Time() );
		}
		else 
		{
			m_EditorScene->OnUpdate( Application::Get().Time() );
			m_EditorScene->OnRenderEditor( m_EditorCamera, Application::Get().Time() );
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		// Draw dockspace.
		ImGuiViewport* pViewport = ImGui::GetWindowViewport();
		ImGui::DockSpaceOverViewport( pViewport );
		
		m_TitleBar->Draw();

		PanelManager::Get().DrawAllPanels();

		// Draw widgets.
		m_Viewport->Draw();

		SceneRenderer::Get().ImGuiRender();

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

		SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel *)PanelManager::Get().GetPanel( "Scene Hierarchy Panel" );

		if( auto& rSelection = pHierarchyPanel->GetSelectionContext() )
		{
			if( rSelection.HasComponent<MeshComponent>() )
			{
				if( auto& mesh = rSelection.GetComponent<MeshComponent>().Mesh )
				{
					ImGui::TextDisabled( "%llx", rSelection.GetComponent<IdComponent>().ID );

					ImGui::Separator();

					for ( auto& rMaterial : mesh->GetMaterials() )
					{
						if( ImGui::CollapsingHeader( rMaterial->GetName().c_str() ) ) 
						{
							ImGui::PushID( rMaterial->GetName().c_str() );

							ImGui::Text( "Mesh name: %s", mesh->FilePath().c_str() );

							ImGui::Separator();

							ImGui::Text( "Shader: %s", rMaterial->GetShader()->GetName().c_str() );

							ImGui::Separator();

							ImGui::Text( "Albedo" );

							ImGui::Separator();

							Ref< Texture2D > texture = rMaterial->GetResource( "u_AlbedoTexture" );
								
							if( texture && texture->GetDescriptorSet() )
								ImGui::Image( texture->GetDescriptorSet(), ImVec2( 100, 100 ) );
							else
								ImGui::Image( m_CheckerboardTexture->GetDescriptorSet(), ImVec2( 100, 100 ) );

							ImGui::SameLine();

							if( ImGui::Button( "...##opentexture", ImVec2( 50, 20 ) ) )
							{
								std::string file = Application::Get().OpenFile( "Texture File (*.png *.tga)\0*.tga; *.png\0" );
								
								if( !file.empty() )
								{
									rMaterial->SetResource( "u_AlbedoTexture", Ref<Texture2D>::Create( file, AddressingMode::Repeat ) );
								}
							}

							glm::vec3& color = rMaterial->Get<glm::vec3>( "u_Materials.AlbedoColor" );

							ImGui::ColorEdit3( "##Albedo Color", glm::value_ptr( color ), ImGuiColorEditFlags_NoInputs );

							ImGui::Text( "Normal" );

							ImGui::Separator();

							bool UseNormalMap = rMaterial->Get< float >( "u_Materials.UseNormalMap" );

							if( UseNormalMap )
							{
								Ref< Texture2D > texture = rMaterial->GetResource( "u_NormalTexture" );
								
								if( texture && texture->GetDescriptorSet() )
									ImGui::Image( texture->GetDescriptorSet(), ImVec2( 100, 100 ) );
								else
									ImGui::Image( m_CheckerboardTexture->GetDescriptorSet(), ImVec2( 100, 100 ) );
							}

							if( ImGui::Checkbox( "Use Normal Map", &UseNormalMap ) )
								rMaterial->Set( "u_Materials.UseNormalMap", UseNormalMap ? 1.0f : 0.0f );
							
							ImGui::Text( "Roughness" );
							
							ImGui::Separator();
							
							float Roughness = rMaterial->Get< float >( "u_Materials.Roughness" );

							ImGui::DragFloat( "Roughness", &Roughness, 0.01f, 0.0f, 1.0f );

							if( Roughness != rMaterial->Get< float >( "u_Materials.Roughness" ) )
								rMaterial->Set( "u_Materials.Roughness", Roughness );

							// Roughness map
							{
								Ref< Texture2D > texture = rMaterial->GetResource( "u_RoughnessTexture" );

								if( texture )
								{
									if( texture && texture->GetDescriptorSet() )
										ImGui::Image( texture->GetDescriptorSet(), ImVec2( 100, 100 ) );
									else
										ImGui::Image( m_CheckerboardTexture->GetDescriptorSet(), ImVec2( 100, 100 ) );
								}
							}

							ImGui::Text( "Metalness" );

							ImGui::Separator();

							float Metallic = rMaterial->Get< float >( "u_Materials.Metalness" );
							
							ImGui::DragFloat( "Metalness", &Metallic, 0.01f, 0.0f, 1.0f );

							if( Metallic != rMaterial->Get< float >( "u_Materials.Metalness" ) )
								rMaterial->Set( "u_Materials.Metalness", Metallic );
							
							// Metalness map
							{

								Ref< Texture2D > texture = rMaterial->GetResource( "u_MetallicTexture" );

								if( texture )
								{
									if( texture && texture->GetDescriptorSet() )
										ImGui::Image( texture->GetDescriptorSet(), ImVec2( 100, 100 ) );
									else
										ImGui::Image( m_CheckerboardTexture->GetDescriptorSet(), ImVec2( 100, 100 ) );
								}
							}

							ImGui::PopID();
						}
					}
				}
			}
		}

		if( !HasEnvironmentVariable( "SATURN_PREMAKE_PATH" ) )
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
					path = Application::Get().OpenFile( ".exe\0*.exe;\0" );

				if( !path.empty() )
				{
					if( ImGui::Button( "Close" ) )
					{
						ImGui::CloseCurrentPopup();
						
						Saturn::SetEnvironmentVariable( "SATURN_PREMAKE_PATH", path.c_str() );
					}
				}

				ImGui::EndPopup();
			}

			ImGui::OpenPopup( "Missing Environment Variable" );
		}
		
		ImGui::End();
	}

	void EditorLayer::OnEvent( Event& rEvent )
	{
		EventDispatcher dispatcher( rEvent );
		dispatcher.Dispatch<KeyPressedEvent>( SAT_BIND_EVENT_FN( EditorLayer::OnKeyPressed ) );

		if ( m_Viewport->m_SendCameraEvents )
		{
			m_EditorCamera.AllowEvents( true );
			m_EditorCamera.OnEvent( rEvent );
		}
		else
		{	
			// HACK, I'll come back to this later

			if ( m_EditorCamera.HasEvents() )
			{
				//m_EditorCamera.Reset();
			}
			
			// Allow the camera to handle key released
			if ( rEvent.GetEventType() == EventType::KeyReleased )
			{
				m_EditorCamera.OnEvent( rEvent );
			}

			m_EditorCamera.AllowEvents( false );
		}
	}

	void EditorLayer::SaveFile( const std::string& FileName )
	{
		SceneSerialiser serialiser( m_EditorScene );
		serialiser.Serialise( FileName );
	}

	void EditorLayer::SaveFile()
	{
	}

	void EditorLayer::OpenFile( const std::string& FileName )
	{
		SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) PanelManager::Get().GetPanel( "Scene Hierarchy Panel" );
		Ref<Scene> newScene = Ref<Scene>::Create();

		SceneSerialiser serialiser( newScene );
		serialiser.Deserialise( FileName );

		newScene->CopyScene( m_EditorScene );

		newScene = nullptr;
		
		pHierarchyPanel->SetContext( m_EditorScene );
		pHierarchyPanel->SetSelected( {} );
	}

	void EditorLayer::OpenFile()
	{
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
				SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) PanelManager::Get().GetPanel( "Scene Hierarchy Panel" );

				if( pHierarchyPanel )
				{
					if( auto& rEntity = pHierarchyPanel->GetSelectionContext() )
					{
						m_EditorScene->DeleteEntity( rEntity );
						pHierarchyPanel->SetSelected( {} );
					}
				}
			} break;
		}

		if( Input::Get().KeyPressed( Key::LeftControl ) )
		{
			switch( rEvent.KeyCode() )
			{
				case Key::D:
				{
					SceneHierarchyPanel* pHierarchyPanel = ( SceneHierarchyPanel* ) PanelManager::Get().GetPanel( "Scene Hierarchy Panel" );
					
					if( pHierarchyPanel ) 
					{
						if( auto& rEntity = pHierarchyPanel->GetSelectionContext() )
						{
							m_EditorScene->DuplicateEntity( rEntity );
						}
					}

				} break;
			}
		}

		return true;
	}

}