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

#include "ProjectBrowserLayer.h"

#include "ProjectBrowserApp.h"

#include <glfw/glfw3.h>

#include <imgui_internal.h>
#include <imgui.h>
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#include <Saturn/Project/Project.h>
#include <Saturn/Core/EngineSettings/EngineSettings.h>

namespace ProjectBrowser {

	ProjectBrowserLayer::ProjectBrowserLayer()
	{

	}

	void ProjectBrowserLayer::InitImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); ( void )io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			style.WindowRounding = 0.0f;
			style.Colors[ ImGuiCol_WindowBg ].w = 1.0f;
		}

		ProjectBrowserApp& app = ProjectBrowserApp::Get();
		GLFWwindow* window = static_cast< GLFWwindow* >( app.GetWindow().GetNativeWindow() );

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL( window, true );
		ImGui_ImplOpenGL3_Init( "#version 410" );
	}

	void ProjectBrowserLayer::Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ProjectBrowserLayer::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ProjectBrowserLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		ProjectBrowserApp& app = ProjectBrowserApp::Get();
		io.DisplaySize = ImVec2( ( float )app.GetWindow().GetWidth(), ( float )app.GetWindow().GetHeight() );

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent( backup_current_context );
		}
	}

	void ProjectBrowserLayer::OnImGuiRender( void )
	{
		static bool p_open = true;

		static bool opt_fullscreen_persistant = true;
		static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
		bool opt_fullscreen = opt_fullscreen_persistant;

		//Check if we have already have a startup project
		if ( Saturn::ProjectSettings::HasStartupProject() )
		{
			ProjectBrowserApp::Get().SetPendingClose( true );
		}

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
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
		if( ImGui::Begin( "DockSpace Demo", &p_open, window_flags ) )
		{
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

			ImGuiWindowFlags browser_flags = ImGuiWindowFlags_NoTitleBar; 
			browser_flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos( viewport->Pos );
			ImGui::SetNextWindowSize( viewport->Size );

			ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );

			if( ImGui::Begin( "##projectbrowser", &p_open, browser_flags ) )
			{
				if( ImGui::Button( "New Project" ) )
				{
					ImGui::OpenPopup( "new-project" );
				}

				if( ImGui::BeginPopupModal( "new-project" ) )
				{
					ImGui::Text( "Enter project Name" );
					if( m_ProjectName.length() > 256 )
					{
						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::InputText( "##name", ( char* )m_ProjectName.c_str(), 256, ImGuiInputTextFlags_ReadOnly );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();
						ImGui::SameLine();
						if( ImGui::Button( "Clear" ) ) 
							m_ProjectName = "";
					}
					else
					{
						char buffer[ 256 ];
						memset( buffer, 0, 256 );
						memcpy( buffer, m_ProjectName.c_str(), m_ProjectName.length() );
						if( ImGui::InputText( "##name", buffer, 256 ) )
						{
							m_ProjectName = std::string( buffer );
						}
					}

					ImGui::Text( "Enter project Directory" );
					if( m_ProjectDirectory.length() > 256 && !std::filesystem::exists( m_ProjectDirectory ) )
					{
						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::InputText( "##directory", ( char* )m_ProjectDirectory.c_str(), 256, ImGuiInputTextFlags_ReadOnly );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();
						ImGui::SameLine();
						if( ImGui::Button( "Clear" ) )
							m_ProjectDirectory = "";
					}
					else
					{
						char buffer[ 256 ];
						memset( buffer, 0, 256 );
						memcpy( buffer, m_ProjectDirectory.c_str(), m_ProjectDirectory.length() );
						if( ImGui::InputText( "##directory", buffer, 256 ) )
						{
							m_ProjectDirectory = "assets\\" + std::string( buffer );
						}
					}

					namespace fs = std::filesystem;

					if( ImGui::Button( "Create Project" ) )
					{
						if( m_ProjectDirectory != "" )
						{
							fs::create_directories( m_ProjectDirectory );
							Saturn::Ref<Saturn::Project> project = Saturn::Ref<Saturn::Project>::Create( m_ProjectDirectory, m_ProjectName );
							Saturn::ProjectSettings::SetCurrentProject( project );
							m_ProjectDirectory = "Null";
							m_ProjectName = "Null";
							ImGui::CloseCurrentPopup();
							Saturn::ProjectSettings::Save();
							Saturn::ProjectSettings::GetCurrentProject()->CopyAssets();
							ProjectBrowserApp::Get().SetPendingClose( true );
						}
					}

					ImGui::EndPopup();
				}
			}

			ImGui::PopStyleVar( 2 );

			ImGui::End();

		}
		ImGui::End();
	}

	void ProjectBrowserLayer::OnUpdate( Saturn::Timestep ts )
	{

	}

}