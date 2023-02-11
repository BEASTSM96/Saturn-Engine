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
#include "ProjectBrowserLayer.h"

#include <Saturn/ImGui/Panel/Panel.h>
#include <Saturn/ImGui/Panel/PanelManager.h>
#include <Saturn/ImGui/TitleBar.h>

#include <Saturn/Core/StringUtills.h>
#include <Saturn/Core/AppData.h>
#include <Saturn/Core/EnvironmentVariables.h>

#include <Saturn/Serialisation/ProjectSerialiser.h>
#include <Saturn/Serialisation/UserSettingsSerialiser.h>

#include <Saturn/Core/Window.h>

#include <glm/gtc/type_ptr.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h> 
#include <imgui_internal.h>

#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

namespace Saturn {

	static char* s_SaturnDirBuffer = new char[ 1024 ];
	static std::string s_SaturnDir = "";

	static char* s_ProjectNameBuffer = new char[ 1024 ];
	static char* s_ProjectFilePathBuffer = new char[ 1024 ];

	static bool s_ShowNewProjectPopup = false;
	static bool s_ShouldThreadTerminate = false;
	
	static std::thread s_RecentProjectThread;
	
	static std::vector< std::filesystem::path > s_RecentProjects;

	static void ReplaceToken( std::string& str, const char* token, const std::string& value )
	{
		size_t pos = 0;
		while( ( pos = str.find( token, pos ) ) != std::string::npos )
		{
			str.replace( pos, strlen( token ), value );
			pos += strlen( token );
		}
	}

	ProjectBrowserLayer::ProjectBrowserLayer()
	{
		m_HasSaturnDir = Auxiliary::HasEnvironmentVariable( "SATURN_DIR" );

		if( m_HasSaturnDir )
		{
			s_SaturnDir = Auxiliary::GetEnvironmentVariable( "SATURN_DIR" );
		}

		memset( s_SaturnDirBuffer, 0, 1024 );

		memset( s_ProjectFilePathBuffer, 0, 1024 );
		memset( s_ProjectNameBuffer, 0, 1024 );

		auto& userSettings = GetUserSettings();
		
		UserSettingsSerialiser userSettingsSerialiser;
		userSettingsSerialiser.Deserialise( userSettings );

		for ( auto&& path : userSettings.RecentProjects )
		{
			if( std::filesystem::exists( path ) )
				s_RecentProjects.push_back( path );
		}
		
		s_RecentProjectThread = std::thread( []() 
		{
			do
			{
				auto& userSettings = GetUserSettings();

				for( auto& path : userSettings.RecentProjects )
				{
					// Check if the path exists in out recent projects list.

					bool exists = false;

					for( auto& recentProject : s_RecentProjects )
					{
						if( recentProject == path )
						{
							exists = true;
							break;
						}
					}

					if( !exists )
					{
						if( !path.empty() )
							s_RecentProjects.push_back( path );
					}
				}
			} while( !s_ShouldThreadTerminate );
		} );
	}

	void ProjectBrowserLayer::OnAttach()
	{
		m_TitleBar = new TitleBar();
		
		// This is still not perfect.
		// Why does the project browser need to do this?
		Window::Get().SetTitlebarHitTest( [&]( int x, int y ) -> bool
		{
			if( !m_TitleBar )
				return false;

			auto TitleBarHeight = m_TitleBar->Height();

			RECT windowRect;
			POINT mousePos;
			GetClientRect( glfwGetWin32Window( ( GLFWwindow* ) Window::Get().NativeWindow() ), &windowRect );

				// Drag the menu bar to move the window
			if( !Window::Get().Maximized() && !ImGui::IsAnyItemHovered() && ( y < ( windowRect.top + TitleBarHeight ) ) )
				return true;
			else
				return false;
		} );

		m_TitleBar->AddOnExitFunction( []() 
		{
			s_ShouldThreadTerminate = true;

			using namespace std::literals::chrono_literals;

			std::this_thread::sleep_for( 1ms );

			s_RecentProjectThread.join();
		} );	
	}

	ProjectBrowserLayer::~ProjectBrowserLayer()
	{
	}
	
	void ProjectBrowserLayer::OnDetach()
	{
		Window::Get().SetTitlebarHitTest( [&]( int x, int y ) -> bool { return false; } );

		delete m_TitleBar;
	}

	void ProjectBrowserLayer::OnUpdate( Timestep time )
	{

	}

	void ProjectBrowserLayer::OnImGuiRender()
	{
		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport( pViewport );

		// --- Title bar
		
		m_TitleBar->Draw();

		// --- Check if Saturn directory is set, if not show prompt to set it
		if( !m_HasSaturnDir )
		{
			if( ImGui::BeginPopupModal( "Saturn directory not set", NULL, ImGuiWindowFlags_AlwaysAutoResize ) ) 
			{
				ImGui::Text( "No Saturn directory set. Please set the SATURN_DIR environment variable." );
				
				ImGui::InputText( "", ( char* )s_SaturnDir.c_str(), 1024, ImGuiInputTextFlags_ReadOnly );
				ImGui::SameLine();
				if( ImGui::Button( "...##dir" ) )
				{
					auto res = Application::Get().OpenFolder();
					s_SaturnDir = res;
				}
				
				if( !s_SaturnDir.empty() && std::filesystem::exists( s_SaturnDir ) )
				{
					if( ImGui::Button( "Set" ) )
					{
						Auxiliary::SetEnvironmentVariable( "SATURN_DIR", s_SaturnDir.c_str() );
						m_HasSaturnDir = true;
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}

			ImGui::OpenPopup( "Saturn directory not set" );
		}
		
		// Begin main project browser.
		ImGui::SetNextWindowSize( pViewport->WorkSize, ImGuiCond_Always );
		ImGui::Begin( "##project_browser", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove );

		ImGui::Columns( 2 );
		ImGui::SetColumnWidth( 0, pViewport->WorkSize.x / 1.5f );

		ImGui::SetWindowSize( { pViewport->WorkSize.x / 1.5f / 2, pViewport->WorkPos.y } );

		// Recent projects.
		ImGui::BeginChild( "##prj_center_window" );
		{
			for ( auto& rPath : s_RecentProjects )
			{
				if( ImGui::Selectable( rPath.string().c_str(), false ) )
				{
					OpenProject( rPath.string() );
					
					s_ShouldThreadTerminate = true;
					s_RecentProjectThread.join();

					Application::Get().Close();
				}
			}
		}
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild( "##prj_right_window" );
		{
			if( ImGui::Button( "Create a project" ) )
			{
				s_ShowNewProjectPopup = true;
			}
		}
		ImGui::EndChild();

		if( s_ShowNewProjectPopup )
		{
			ImGui::OpenPopup( "New project" );
			s_ShowNewProjectPopup = false;
		}

		auto center = pViewport->GetCenter();

		ImGui::SetNextWindowPos( center, ImGuiCond_Once, ImVec2( 0.5f, 0.5f ) );

		if( ImGui::BeginPopupModal( "New project", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ) )
		{
			ImGui::InputTextWithHint( "##project_name", "Project name", s_ProjectNameBuffer, 1024 );

			ImGui::SameLine();
			ImGui::Text( ".sproject" );

			ImGui::InputTextWithHint( "##project_loc", "Project location", s_ProjectFilePathBuffer, 1024 );
			ImGui::SameLine();

			if( ImGui::SmallButton( "...##location" ) )
			{
				auto res = Application::Get().OpenFolder();
				memcpy( s_ProjectFilePathBuffer, res.data(), res.size() );
			}

			ImGui::Separator();

			auto drawDisabledBtn = [&]( const char* n ) 
			{
				ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
				ImGui::Button( n );
				ImGui::PopStyleVar( 1 );
				ImGui::PopItemFlag();
			};

			auto createButtonFunc = [&]
			{
				if( s_ProjectNameBuffer == nullptr && s_ProjectFilePathBuffer == nullptr )
				{
					drawDisabledBtn( "Create" );
				}
				else if( !std::filesystem::exists( s_ProjectFilePathBuffer ) )
				{
					drawDisabledBtn( "Create" );
				}
				else 
				{
					if( ImGui::Button( "Create" ) ) 
					{
						std::string path = std::string( s_ProjectFilePathBuffer ) + "/" + std::string( s_ProjectNameBuffer );
						
						std::filesystem::path p( path );

						CreateProject( path );
						
						auto& us = GetUserSettings();
						us.RecentProjects.push_back( p );
						
						UserSettingsSerialiser uss;
						uss.Serialise( us );

						ImGui::CloseCurrentPopup();
					}
				}
			};

			createButtonFunc();

			ImGui::SameLine();

			if( ImGui::Button( "Cancel" ) )
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void ProjectBrowserLayer::CreateProject( const std::string& rPath )
	{
		std::filesystem::path ProjectPath = rPath;
		std::filesystem::path ProjectName = rPath + ".sproject";

		if( !std::filesystem::exists( ProjectPath ) )
			std::filesystem::create_directories( ProjectPath );

		// Copy files.
		std::filesystem::copy( s_SaturnDir + "/Titan/assets/Templates/Base", ProjectPath, std::filesystem::copy_options::recursive );

		// New Project ref
		Ref<Project> newProject = Ref<Project>::Create();

		// Project file
		{
			std::ifstream stream( ProjectPath / "Project.sproject" );
			std::stringstream ss;
			ss << stream.rdbuf();
			stream.close();

			std::string str = ss.str();
			ReplaceToken( str, "/REPLACE_WITH_PROJECT_NAME/", s_ProjectNameBuffer );

			std::ofstream out( ProjectPath / "Project.sproject" );
			out << str;
			out.close();

			newProject->m_Config.Name = std::string( s_ProjectNameBuffer );

			std::string name = std::string( s_ProjectNameBuffer ) + ".sproject";

			newProject->m_Config.Path = ProjectPath.string();

			std::filesystem::rename( ProjectPath / "Project.sproject", ProjectPath / name );
		}

		std::filesystem::create_directory( ProjectPath / "Assets" );

		std::filesystem::create_directories( ProjectPath / "Assets" / "Shaders" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Textures" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Meshes" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Materials" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Scenes" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Sound" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Sound" / "Source" );
		std::filesystem::create_directory( ProjectPath / "Scripts" );

		Project::SetActiveProject( newProject );

		ProjectSerialiser ps;
		ps.Serialise( newProject->m_Config.Path + "\\" + newProject->m_Config.Name );

		Project::SetActiveProject( nullptr );
	}

	void ProjectBrowserLayer::OpenProject( const std::string& rPath )
	{
		// Create saturn process
		STARTUPINFOA StartupInfo = {};
		StartupInfo.cb = sizeof( StartupInfo );

		PROCESS_INFORMATION ProcessInfo;
		
		std::string RootDir = s_SaturnDir;

		std::replace( RootDir.begin(), RootDir.end(), '\\', '/' );
		std::string WorkingDir = RootDir + "/Titan";
#if defined( _DEBUG )
		RootDir += "/bin/Debug-windows-x86_64/Titan/Titan.exe";
#else
		RootDir += "/bin/Release-windows-x86_64/Titan/Titan.exe";
#endif
		RootDir += " " + rPath;
		
		bool res = CreateProcessA( nullptr, RootDir.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, WorkingDir.data(), &StartupInfo, &ProcessInfo );
		
		if( !res )
		{
			SAT_CORE_ERROR( "Unable to start saturn process" );
		}

		CloseHandle( ProcessInfo.hThread );
		CloseHandle( ProcessInfo.hProcess );
	}

	void ProjectBrowserLayer::OnEvent( Event& rEvent )
	{

	}

	bool ProjectBrowserLayer::OnKeyPressed( KeyPressedEvent& rEvent )
	{
		return true;
	}

}