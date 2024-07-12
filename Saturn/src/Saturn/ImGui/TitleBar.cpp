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
#include "TitleBar.h"

#include "ImGuiAuxiliary.h"

#include "Saturn/Core/EnvironmentVariables.h"
#include "backends/imgui_impl_vulkan.h"

#include "Saturn/Core/Ruby/RubyWindow.h"

namespace Saturn {
	
	static bool ShowAcknowledgements = false;

	TitleBar::TitleBar()
		: m_Height( 0 )
	{
	}

	TitleBar::~TitleBar()
	{
	}

	void TitleBar::Draw()
	{
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 5, 5 ) );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 5, 5 ) );

		RubyWindow* pWindow = Application::Get().GetWindow();

		if( ImGui::BeginMainMenuBar() )
		{
			float height = ImGui::GetWindowHeight();

			if( m_Height != height )
			{
				m_Height = height;

				pWindow->SetTiltebarHeight( (uint32_t)height );
			}

			pWindow->SetTitlebarCondition( ImGui::IsAnyItemHovered() );

			for( auto&& rrFunc : m_MenuBarFunctions )
			{
				if( rrFunc )
					rrFunc();
			}

			// System buttons
			{
				const float  buttonSize = ImGui::GetFrameHeight();
				const float  iconMargin = buttonSize * 0.33f;
				ImRect       buttonRect = ImRect( ImGui::GetWindowPos() + ImVec2( ImGui::GetWindowWidth() - buttonSize, 0.0f ), ImGui::GetWindowPos() + ImGui::GetWindowSize() );
				ImDrawList*  drawList  = ImGui::GetWindowDrawList();

				// Exit button
				{
					bool hovered = false;
					bool held    = false;
					bool pressed = false;

					pressed = ImGui::ButtonBehavior( buttonRect, ImGui::GetID( "EXIT" ), &hovered, &held, ImGuiButtonFlags_PressedOnClick );

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
					{
						// Process on exit events to whatever layer owns this.
						if( m_OnExitFunction )
							m_OnExitFunction();

						Application::Get().Close();
					}

					buttonRect.Min.x -= buttonSize;
					buttonRect.Max.x -= buttonSize;
				}

				// Maximize button
				{
					bool hovered = false;
					bool held    = false;
					bool pressed = ImGui::ButtonBehavior( buttonRect, ImGui::GetID( "MAXIMIZE" ), &hovered, &held, ImGuiButtonFlags_PressedOnClick );

					if( hovered )
					{
						const ImU32 Color = ImGui::GetColorU32( held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered );
						drawList->AddRectFilled( buttonRect.Min, buttonRect.Max, Color );
					}

					// Render the box
					{
						const ImU32 Color = ImGui::GetColorU32( ImGuiCol_Text );
						drawList->AddRect( buttonRect.Min + ImVec2( iconMargin, iconMargin ), buttonRect.Max - ImVec2( iconMargin, iconMargin ), Color, 0.0f, 0, 1.0f );
					}

					if( pressed )
						Application::Get().GetWindow()->Maximize();

					buttonRect.Min.x -= buttonSize;
					buttonRect.Max.x -= buttonSize;
				}

				// Minimize button
				{
					bool hovered = false;
					bool held    = false;
					bool pressed = ImGui::ButtonBehavior( buttonRect, ImGui::GetID( "MINIMIZE" ), &hovered, &held, ImGuiButtonFlags_PressedOnClick );

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
						Application::Get().GetWindow()->Minimize();

					buttonRect.Min.x -= buttonSize;
					buttonRect.Max.x -= buttonSize;
				}
			}

			ImGui::EndMainMenuBar();
		}
		
		ImGui::PopStyleVar( 2 );
	}

	void TitleBar::AddMenuBarFunction( MenuBarFunction&& rrFunc )
	{
		m_MenuBarFunctions.push_back( rrFunc );
	}

	void TitleBar::AddOnExitFunction( std::function<void()>&& rrFunc )
	{
		m_OnExitFunction = std::move( rrFunc );
	}

}
