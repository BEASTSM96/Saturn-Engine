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

#include "Saturn/ImGui/Toolbar.h"
#include "Saturn/Vulkan/SceneRenderer.h"
#include "Saturn/ImGui/TitleBar.h"

namespace Saturn {

	EditorLayer::EditorLayer()
	{
		m_EditorScene = Ref<Scene>::Create();
		m_RuntimeScene = nullptr;

		m_TitleBar = new TitleBar();
		m_SceneHierarchyPanel = new SceneHierarchyPanel();
		m_Viewport = new Viewport();
		m_Toolbar = new Toolbar();

		m_SceneHierarchyPanel->SetContext( m_EditorScene );
		m_SceneHierarchyPanel->SetSelectionChangedCallback( SAT_BIND_EVENT_FN( EditorLayer::SelectionChanged ) );
		
		m_EditorCamera = EditorCamera( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 10000.0f ) );

		m_EditorCamera.AllowEvents( true );
		m_EditorCamera.SetActive( true );

	}

	void EditorLayer::OnUpdate( Timestep time )
	{
		//m_EditorCamera = EditorCamera( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 10000.0f ) );
		
		if( m_Viewport->m_SendCameraEvents )
			m_EditorCamera.OnUpdate( time );

		SceneRenderer::Get().SetEditorCamera( m_EditorCamera );

		if( m_Toolbar->WantsToStartRuntime ) 
		{
			if( !m_RuntimeScene )
			{
				m_RuntimeScene = Ref<Scene>::Create();
				
				m_EditorScene->CopyScene( m_RuntimeScene );

				m_SceneHierarchyPanel->SetContext( m_RuntimeScene );

				m_RuntimeScene->m_RuntimeRunning = true;
			}
		}
		else
		{
			if( m_RuntimeScene && m_RuntimeScene->m_RuntimeRunning )
			{
				m_RuntimeScene = nullptr;

				m_SceneHierarchyPanel->SetContext( m_EditorScene );
			}
		}

		if( m_RuntimeScene )
			m_RuntimeScene->OnRenderEditor( m_EditorCamera, Application::Get().Time() );
		else
			m_EditorScene->OnRenderEditor( m_EditorCamera, Application::Get().Time() );
	}

	void EditorLayer::OnImGuiRender()
	{
		// Draw dockspace.
		bool p_open = true;

		static bool opt_fullscreen_persistant = true;
		static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
		bool opt_fullscreen = opt_fullscreen_persistant;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if( opt_fullscreen )
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos( viewport->Pos );
			ImGui::SetNextWindowSize( viewport->Size );
			ImGui::SetNextWindowViewport( viewport->ID );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		//if (opt_flags & ImGuiDockNodeFlags_PassthruDockspace)
		//	window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
		ImGui::Begin( "DockSpace Demo", &p_open, window_flags );
		ImGui::PopStyleVar();

		if( opt_fullscreen )
			ImGui::PopStyleVar( 2 );

		// Dockspace
		ImGuiIO& io = ImGui::GetIO();
		if( io.ConfigFlags & ImGuiConfigFlags_DockingEnable )
		{
			ImGuiID dockspace_id = ImGui::GetID( "MyDockspace" );
			ImGui::DockSpace( dockspace_id, ImVec2( 0.0f, 0.0f ), opt_flags );
		}

		// Draw widgets.
		m_TitleBar->Draw();
		m_Toolbar->Draw();
		m_SceneHierarchyPanel->Draw();
		m_Viewport->Draw();

		ImGui::Begin( "Renderer" );

		ImGui::Text( "Frame Time: %.2f ms", Application::Get().Time().Milliseconds() );

		for( const auto& devices : VulkanContext::Get().GetPhysicalDeviceProperties() )
		{
			ImGui::Text( "Device Name: %s", devices.DeviceProps.deviceName );
			ImGui::Text( "API Version: %i", devices.DeviceProps.apiVersion );
			ImGui::Text( "Vendor ID: %i", devices.DeviceProps.vendorID );
		}
		
		ImGui::End();

		ImGui::End();
	}

	void EditorLayer::OnEvent( Event& rEvent )
	{
		if ( m_Viewport->m_SendCameraEvents )
		{
			m_EditorCamera.AllowEvents( true );
			m_EditorCamera.OnEvent( rEvent );
		}
		else
			m_EditorCamera.AllowEvents( false );
		
	}

}