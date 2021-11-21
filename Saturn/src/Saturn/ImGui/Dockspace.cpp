/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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
#include "Dockspace.h"

#include "Saturn/Core/IO.h"

#include "Saturn/Discord/DiscordRPC.h"

#include "imgui.h"

namespace Saturn {

	ImGuiDockspace::ImGuiDockspace()
	{
		IO::Get().StdStreamRedirect();

		m_Scene = Ref<Scene>::Create();

		DiscordRPC::Get().m_SceneName = m_Scene->Name();

		m_TitleBar = new TitleBar();
		m_SceneHierarchyPanel = new SceneHierarchyPanel();
		m_Viewport = new Viewport();

		m_SceneHierarchyPanel->SetContext( m_Scene );
		m_SceneHierarchyPanel->SetSelectionChangedCallback( SAT_BIND_EVENT_FN( ImGuiDockspace::SelectionChanged ) );
	}

	void ImGuiDockspace::Draw()
	{
		// imgui_demo.cpp

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

		// Draw Widgets

		m_SceneHierarchyPanel->Draw();
		m_Viewport->Draw();
		m_TitleBar->Draw();
		
		// TEMP
		ImGui::Begin( "Output" );

		ImGui::Text( "%s", IO::Get().StdStreamBuffer().str().c_str() );

		ImGui::End();

		m_Scene->OnRenderEditor( Application::Get().Time() );

		ImGui::End();
	}

	void ImGuiDockspace::SelectionChanged( Entity e )
	{

	}

}