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
#include "ProjectBrowserLayer.h"

#include <Saturn/ImGui/Panel/Panel.h>
#include <Saturn/ImGui/Panel/PanelManager.h>
#include <Saturn/ImGui/ImGuiAuxiliary.h>
#include <Saturn/ImGui/TitleBar.h>
#include <Saturn/Core/Ruby/RubyWindow.h>

#include <Saturn/Core/StringAuxiliary.h>
#include <Saturn/Core/EnvironmentVariables.h>
#include <Saturn/Core/Process.h>

#include <Saturn/Serialisation/ProjectSerialiser.h>
#include <Saturn/Serialisation/EngineSettingsSerialiser.h>

#include <Saturn/Core/JobSystem.h>

#include <glm/gtc/type_ptr.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h> 
#include <imgui_internal.h>

namespace Saturn {

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
			m_SaturnDir = Auxiliary::GetEnvironmentVariable( "SATURN_DIR" );

		memset( m_SaturnDirBuffer, 0, 1024 );
		memset( m_ProjectFilePathBuffer, 0, 1024 );
		memset( m_ProjectNameBuffer, 0, 1024 );

		EngineSettingsSerialiser::Deserialise();
		
		Application::Get().GetWindow()->Show();

		m_RecentProjectThread = std::thread( [this]() 
		{
			SetThreadDescription( GetCurrentThread(), L"RecentProjectThread" );
			auto& userSettings = EngineSettings::Get();

			bool projectsNeedSorting = false;

			while( !m_ShouldThreadTerminate )
			{
				for( auto& path : userSettings.RecentProjects )
				{
					auto Itr = std::find_if( m_RecentProjects.begin(), m_RecentProjects.end(), 
						[path](const auto& rInfo)
						{
							return rInfo.Filepath == path;
						} );

					if( Itr == m_RecentProjects.end() )
					{
						// Deserialise the project.
						ProjectSerialiser ps;
						ps.Deserialise( path );

						Ref<Project> project = Project::GetActiveProject();
						
						if( project )
						{
							ProjectInformation info{};
							info.Filepath = path;
							info.Name = project->GetConfig().Name;
							info.AssetPath = project->GetFullAssetPath();
							info.LastWriteTime = std::format( "{0}", std::filesystem::last_write_time( path ) );
							info.LastWriteTime = info.LastWriteTime.substr( 0, info.LastWriteTime.find_first_of( " " ) );

							m_RecentProjects.push_back( info );

							// Reset.
							Project::SetActiveProject( nullptr );
						
							projectsNeedSorting = true;
						}
					}

					if( projectsNeedSorting )
					{
						std::sort( m_RecentProjects.begin(), m_RecentProjects.end(), 
							[]( const auto& rA, const auto& rB )
							{
								if( rA.LastWriteTime > rB.LastWriteTime )
									return true;
								else
									return false;
							} );

						projectsNeedSorting = false;
					}
				}

				std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
			}
		} );

		Application::Get().GetWindow()->ChangeTitle( "Saturn Project Browser" );
	}

	void ProjectBrowserLayer::OnAttach()
	{	
		m_TitleBar = new TitleBar();
		m_TitleBar->AddOnExitFunction( [this]() 
		{
			m_ShouldThreadTerminate = true;

			using namespace std::literals::chrono_literals;

			std::this_thread::sleep_for( 1ms );

			m_RecentProjectThread.join();
		} );

		m_NoIconTexture = Ref<Texture2D>::Create( "content/textures/NoIcon.png" );
	}

	ProjectBrowserLayer::~ProjectBrowserLayer()
	{
		m_NoIconTexture = nullptr;
		delete m_TitleBar;
	}
	
	void ProjectBrowserLayer::OnDetach()
	{
		m_ShouldThreadTerminate = true;
	
		if( m_RecentProjectThread.joinable() )
			m_RecentProjectThread.join();
	}

	void ProjectBrowserLayer::OnUpdate( Timestep time )
	{

	}

	void ProjectBrowserLayer::OnImGuiRender()
	{
		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGuiID dockspaceID = ImGui::DockSpaceOverViewport( pViewport, ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoDockingOverMe | ImGuiDockNodeFlags_NoUndocking );

		// --- Title bar
		
		m_TitleBar->Draw();

		// --- Check if Saturn directory is set, if not show prompt to set it
		if( !m_HasSaturnDir )
		{
			if( ImGui::BeginPopupModal( "Saturn directory not set", NULL, ImGuiWindowFlags_AlwaysAutoResize ) ) 
			{
				ImGui::Text( "No Saturn directory set. Please set the SATURN_DIR environment variable." );
				
				ImGui::InputText( "", ( char* )m_SaturnDir.c_str(), 1024, ImGuiInputTextFlags_ReadOnly );
				ImGui::SameLine();
				if( ImGui::Button( "...##dir" ) )
				{
					auto res = Application::Get().OpenFolder();
					m_SaturnDir = res;
				}
				
				if( !m_SaturnDir.empty() && std::filesystem::exists( m_SaturnDir ) )
				{
					if( ImGui::Button( "Set" ) )
					{
						Auxiliary::SetEnvironmentVariable( "SATURN_DIR", m_SaturnDir.string() );
						
						m_HasSaturnDir = true;
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}

			ImGui::OpenPopup( "Saturn directory not set" );
		}

		ImGui::Begin( "##project_browser", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar );

		ImGui::SetWindowDock( ImGui::GetCurrentWindow(), dockspaceID, ImGuiCond_FirstUseEver );

		// Recent Projects
		ImGui::BeginHorizontal( "##recentProjects" );

		for( auto& rProjectInfo : m_RecentProjects )
		{
			DrawRecentProject( rProjectInfo );
		
			ImGui::Spring();
		}

		ImGui::EndHorizontal();

		constexpr float bottomBarHeight = 48.0f;
		ImGui::Dummy( ImVec2( 0.0f, ImGui::GetContentRegionAvail().y - bottomBarHeight ) );

		ImGui::SetCursorPosY( ImGui::GetWindowHeight() - bottomBarHeight );

		ImGui::Separator();

		ImGui::BeginHorizontal( "##project_browser_bottom" );

		ImGui::Button( "Browse", ImVec2( bottomBarHeight, bottomBarHeight ) );
		ImGui::Spring();

		if( ImGui::Button( "Create New", ImVec2( bottomBarHeight, bottomBarHeight ) ) ) 
			m_ShowNewProjectPopup = true;

		ImGui::EndHorizontal();

		if( m_ShowNewProjectPopup )
		{
			ImGui::OpenPopup( "New project" );
			m_ShowNewProjectPopup = false;
		}

		auto center = pViewport->GetCenter();
		ImGui::SetNextWindowPos( center, ImGuiCond_FirstUseEver, ImVec2( 0.5f, 0.5f ) );

		if( ImGui::BeginPopupModal( "New project", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ) )
		{
			ImGui::InputTextWithHint( "##project_name", "Project name", m_ProjectNameBuffer, 1024 );

			ImGui::SameLine();
			ImGui::Text( ".sproject" );

			ImGui::InputTextWithHint( "##project_loc", "Project location", m_ProjectFilePathBuffer, 1024 );
			ImGui::SameLine();

			if( ImGui::SmallButton( "...##location" ) )
			{
				auto res = Application::Get().OpenFolder();
				memcpy( m_ProjectFilePathBuffer, res.data(), res.size() );
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
					if( m_ProjectNameBuffer == nullptr && m_ProjectFilePathBuffer == nullptr )
					{
						drawDisabledBtn( "Create" );
					}
					else if( !std::filesystem::exists( m_ProjectFilePathBuffer ) )
					{
						drawDisabledBtn( "Create" );
					}
					else
					{
						if( ImGui::Button( "Create" ) )
						{
							std::filesystem::path fullPath = std::string( m_ProjectFilePathBuffer );
							fullPath /= m_ProjectFilePathBuffer;
							fullPath.replace_extension( ".sproject" );

							CreateProject( fullPath );

							auto& us = EngineSettings::Get();
							us.RecentProjects.push_back( fullPath );

							EngineSettingsSerialiser uss;
							uss.Serialise();

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

	void ProjectBrowserLayer::DrawRecentProject( const ProjectInformation& rProject )
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		
		const std::string name = std::format( "##{0}", rProject.Name );

		const ImVec2 projectNameTextSize = ImGui::CalcTextSize( rProject.Name.c_str() );
		const ImVec2 lastWriteTextSize = ImGui::CalcTextSize( rProject.LastWriteTime.c_str() );

		const ImVec2 imageSize = ImVec2( 156.0f, 128.0f );

		ImVec2 buttonSize = ImVec2( 156.0f, 156.0f );
		const float extraSizeNeeded = projectNameTextSize.y + lastWriteTextSize.y;
		buttonSize.y += extraSizeNeeded;

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton( name.c_str(), buttonSize );

		ImRect buttonBoundingBox( pos, ImVec2( pos.x + buttonSize.x, pos.y + buttonSize.y ) );

		bool hovered = ImGui::IsItemHovered();
		if( hovered )
		{
			pDrawList->AddRect( buttonBoundingBox.Min, buttonBoundingBox.Max, ImGui::GetColorU32( ImGuiCol_ButtonHovered ), 5.0f, ImDrawFlags_RoundCornersAll );
		
			if( ImGui::BeginTooltip() ) 
			{
				ImGui::Text( "%s", rProject.Name.c_str() );
				ImGui::Text( "Project Path: %s", rProject.Filepath.string().c_str() );
				ImGui::Text( "Asset Path: %s", rProject.AssetPath.string().c_str() );
				ImGui::Text( "Last Modified: %s", rProject.LastWriteTime.c_str() );

				ImGui::EndTooltip();
			}

			if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
			{
				OpenProject( rProject );
			}
		}

		// Draw the button background
		pDrawList->AddRectFilled( buttonBoundingBox.Min, buttonBoundingBox.Max, ImGui::GetColorU32( hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button ), 5.0f, ImDrawFlags_RoundCornersAll );

		ImVec2 imagePos = ImVec2( 
			buttonBoundingBox.Min.x, 
			buttonBoundingBox.Min.y );

		pDrawList->AddImage( 
			m_NoIconTexture->GetDescriptorSet(), 
			imagePos, 
			ImVec2( imagePos.x + imageSize.x, imagePos.y + imageSize.y ), { 0, 1 }, { 1,0 } );

		float available_height = buttonSize.y - imageSize.y - ImGui::GetStyle().FramePadding.y * 2;
		float line_height = available_height / 2;

		ImVec2 projectNameTextPos = ImVec2(
			buttonBoundingBox.Min.x + ( buttonSize.x - projectNameTextSize.x ) * 0.5f,
			imagePos.y + imageSize.y + ImGui::GetStyle().FramePadding.y * 0.5f + 4.0f
		);
		pDrawList->AddText( projectNameTextPos, ImGui::GetColorU32( ImGuiCol_Text ), rProject.Name.c_str() );

		ImVec2 lastWriteTimeTextPos = ImVec2(
			buttonBoundingBox.Min.x + ImGui::GetStyle().FramePadding.x + ( buttonSize.x - lastWriteTextSize.x ) * 0.5f,
			projectNameTextPos.y + line_height
		);
		pDrawList->AddText( lastWriteTimeTextPos, ImGui::GetColorU32( ImGuiCol_TextDisabled ), rProject.LastWriteTime.c_str() );
	}

	void ProjectBrowserLayer::CreateProject( const std::filesystem::path& rPath )
	{
		std::filesystem::path ProjectPath = rPath.parent_path();
		std::string ProjectName = rPath.filename().string();

		if( !std::filesystem::exists( ProjectPath ) )
			std::filesystem::create_directories( ProjectPath );

		// Copy files.
		std::filesystem::path targetPath = m_SaturnDir;
		targetPath /= "Saturn-Editor";
		targetPath /= "content";
		targetPath /= "Templates";
		targetPath /= "Base";

		std::filesystem::copy( targetPath, ProjectPath, std::filesystem::copy_options::recursive );

		// New Project ref
		Ref<Project> newProject = Ref<Project>::Create();

		// Project file
		{
			std::ifstream stream( ProjectPath / "Project.sproject" );
			std::stringstream ss;
			ss << stream.rdbuf();
			stream.close();

			std::string str = ss.str();
			ReplaceToken( str, "/REPLACE_WITH_PROJECT_NAME/", ProjectName );

			std::ofstream out( ProjectPath / "Project.sproject" );
			out << str;
			out.close();

			newProject->GetConfig().Name = ProjectName;
			newProject->GetConfig().Path = ProjectPath.string();

			std::filesystem::rename( ProjectPath / "Project.sproject", ProjectPath / ProjectName );
		}

		std::filesystem::create_directory( ProjectPath / "Assets" );

		std::filesystem::create_directories( ProjectPath / "Assets" / "Shaders" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Textures" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Meshes" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Materials" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Scenes" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Sound" );
		std::filesystem::create_directories( ProjectPath / "Assets" / "Sound" / "Source" );
		
		std::filesystem::create_directory( ProjectPath / "Source" );
		std::filesystem::create_directory( ProjectPath / "Build" );
		std::filesystem::create_directory( ProjectPath / "Cache" );

		std::filesystem::create_directories( ProjectPath / "Source" / newProject->GetConfig().Name );

		{
			std::filesystem::path targetFilePath = m_SaturnDir;
			targetFilePath /= "Saturn-Editor";
			targetFilePath /= "content";
			targetFilePath /= "Templates";
			targetFilePath /= "%PROJECT_NAME%.Load.cpp";

			std::filesystem::copy( targetFilePath, ProjectPath / "Build" );
			
			std::string filename = "%PROJECT_NAME%.Load.cpp";
			std::string newName = std::format( "{0}.Load.cpp", newProject->GetConfig().Name );

			std::filesystem::rename( ProjectPath / "Build" / filename, ProjectPath / "Build" / newName );
		}

		Project::SetActiveProject( newProject );

		std::filesystem::path outputPath = newProject->GetConfig().Path;
		outputPath /= newProject->GetConfig().Name;

		ProjectSerialiser ps;
		ps.Serialise( outputPath );

		Project::SetActiveProject( nullptr );
	}

	void ProjectBrowserLayer::OpenProject( const ProjectInformation& rProject )
	{
		std::filesystem::path commandLine = m_SaturnDir;

		std::filesystem::path workingDir = commandLine / "Saturn-Editor";
#if defined( SAT_DEBUG )
		commandLine += "\\bin\\Debug-windows-x86_64\\Saturn-Editor\\Saturn-Editor.exe";
#else
		commandLine += "\\bin\\Release-windows-x86_64\\Saturn-Editor\\Saturn-Editor.exe";
#endif
		// We want the root path of the project not the entire path leading to the .sproject file
		commandLine += " " + rProject.Filepath.parent_path().string();

		DeatchedProcess dp( commandLine, workingDir );

		Application::Get().Close();
	}

	void ProjectBrowserLayer::OnEvent( RubyEvent& rEvent )
	{
	}

	bool ProjectBrowserLayer::OnKeyPressed( RubyKeyEvent& rEvent )
	{
		return true;
	}

}