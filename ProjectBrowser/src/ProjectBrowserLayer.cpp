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
#include "ProjectBrowserLayer.h"

#include <Saturn/ImGui/Panel/Panel.h>
#include <Saturn/ImGui/Panel/PanelManager.h>

#include <Saturn/Core/Window.h>

#include <glm/gtc/type_ptr.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h> 
#include <imgui_internal.h>

namespace Saturn {

	ProjectBrowserLayer::ProjectBrowserLayer()
	{

	}

	ProjectBrowserLayer::~ProjectBrowserLayer()
	{

	}

	void ProjectBrowserLayer::OnUpdate( Timestep time )
	{

	}

	void ProjectBrowserLayer::OnImGuiRender()
	{
		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport( pViewport );

		// --- Title bar
		//     TODO: Maybe we can make the title bar a bit more verbose so we can use it between editor and browser
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 5, 5 ) );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 5, 5 ) );

		if( ImGui::BeginMainMenuBar() )
		{
			// System buttons
			{
				const float  buttonSize = ImGui::GetFrameHeight();
				const float  iconMargin = buttonSize * 0.33f;
				ImRect       buttonRect = ImRect( ImGui::GetWindowPos() + ImVec2( ImGui::GetWindowWidth() - buttonSize, 0.0f ), ImGui::GetWindowPos() + ImGui::GetWindowSize() );
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				// Exit button
				{
					bool hovered = false;
					bool held = false;
					bool pressed = ImGui::ButtonBehavior( buttonRect, ImGui::GetID( "EXIT" ), &hovered, &held, 0 );

					if( hovered )
					{
						const ImU32 Color = ImGui::GetColorU32( held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered );
						drawList->AddRectFilled( buttonRect.Min, buttonRect.Max, Color );
					}

					// Render the cross
					{
						const ImU32 Color = ImGui::GetColorU32( ImGuiCol_Text );
						drawList->AddLine( buttonRect.Min + ImVec2( iconMargin, iconMargin ), buttonRect.Max - ImVec2( iconMargin, iconMargin ), Color, 1.0f );
						drawList->AddLine( buttonRect.Min + ImVec2( buttonRect.GetWidth() - iconMargin, iconMargin ), buttonRect.Max - ImVec2( buttonRect.GetWidth() - iconMargin, iconMargin ), Color, 1.0f );
					}

					if( pressed )
						Application::Get().Close();

					buttonRect.Min.x -= buttonSize;
					buttonRect.Max.x -= buttonSize;
				}

				// Maximize button
				{
					// Push disabled
					ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );

					bool hovered = false;
					bool held = false;
					bool pressed = ImGui::ButtonBehavior( buttonRect, ImGui::GetID( "MAXIMIZE" ), &hovered, &held, 0 );


					// Render the box
					{
						const ImU32 Color = ImGui::GetColorU32( ImGuiCol_Text );
						drawList->AddRect( buttonRect.Min + ImVec2( iconMargin, iconMargin ), buttonRect.Max - ImVec2( iconMargin, iconMargin ), Color, 0.0f, 0, 1.0f );
					}

					buttonRect.Min.x -= buttonSize;
					buttonRect.Max.x -= buttonSize;

					ImGui::PopStyleVar();
				}

				// Minimize button
				{
					bool hovered = false;
					bool held = false;
					bool pressed = ImGui::ButtonBehavior( buttonRect, ImGui::GetID( "MINIMIZE" ), &hovered, &held, 0 );

					if( hovered )
					{
						const ImU32 Color = ImGui::GetColorU32( held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered );
						drawList->AddRectFilled( buttonRect.Min, buttonRect.Max, Color );
					}

					// Render the Line
					{
						const ImU32 Color = ImGui::GetColorU32( ImGuiCol_Text );
						drawList->AddLine( buttonRect.Min + ImVec2( iconMargin, buttonRect.GetHeight() / 2 ), buttonRect.Max - ImVec2( iconMargin, buttonRect.GetHeight() / 2 ), Color, 1.0f );
					}

					if( pressed )
						Window::Get().Minimize();

					buttonRect.Min.x -= buttonSize;
					buttonRect.Max.x -= buttonSize;
				}
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::PopStyleVar( 2 );
		
		// ---
		
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;

		ImGui::Begin( "##PB", NULL, flags );

		ImGui::Button("Create project", ImVec2(ImGui::GetWindowWidth() / 2, 0));
		ImGui::SameLine();
		ImGui::Button("Import project", ImVec2(ImGui::GetWindowWidth() / 2, 0));

		ImGui::Separator();

		static std::string path = "";
		
		ImGui::Text( "Project Location:" );
		ImGui::SameLine();
		ImGui::InputText("##pl", ( char* )path.c_str(), 1024);
		
		ImGui::Text( "Project Name:" );
		ImGui::SameLine();
		ImGui::InputText("##pn", ( char* )path.c_str(), 1024 );

		// Draw an open button on the bottom left
		ImGui::SetCursorPosY( ImGui::GetWindowHeight() - ImGui::GetStyle().WindowPadding.y * 2 );
		ImGui::SetCursorPosX( ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 2 - ImGui::GetFrameHeight() );

		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
		
		if( ImGui::Button( "Open", ImVec2( ImGui::GetFrameHeight(), 0 ) ) )
		{
		}
		
		ImGui::PopStyleVar();
		
		ImGui::End();
	}

	void ProjectBrowserLayer::OnEvent( Event& rEvent )
	{

	}

	bool ProjectBrowserLayer::OnKeyPressed( KeyPressedEvent& rEvent )
	{
		return true;
	}

}