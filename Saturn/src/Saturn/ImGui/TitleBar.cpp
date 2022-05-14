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
#include "TitleBar.h"

#include "Saturn/Core/Window.h"

#include "backends/imgui_impl_vulkan.h"

namespace Saturn {
	
	static bool ShowAcknowledgements = false;

	TitleBar::TitleBar()
	{
		m_pLogo = new Texture2D( "assets/Icons/SaturnLogov1.png", AddressingMode::Repeat );

		m_LogoDescSet = ( VkDescriptorSet )ImGui_ImplVulkan_AddTexture( m_pLogo->GetSampler(), m_pLogo->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
	}

	TitleBar::~TitleBar()
	{
		delete m_pLogo;	
	}

	void TitleBar::Draw()
	{
		//ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0, 5 ) );

		if( ImGui::BeginMainMenuBar() )
		{
			m_Height = ImGui::GetWindowHeight();

			if( ImGui::BeginMenu( "File" ) )
			{
				if( ImGui::MenuItem( "Exit", "Alt+F4" ) ) exit( 0 /*EXIT_SUCCESS*/ );

				ImGui::EndMenu();
			}

			if( ImGui::BeginMenu( "Acknowledgements" ) )
			{
				ShowAcknowledgements = true;
				
				ImGui::EndMenu();
			}
			
			if( ShowAcknowledgements )
			{
				ImGui::OpenPopup( "Acknowledgements Modal" );
			}

			if( ImGui::BeginPopupModal( "Acknowledgements Modal", &ShowAcknowledgements ) )
			{
				ImGui::TextWrapped( "Saturn Engine" );

				ImGui::Separator();

				ImGui::TextWrapped( "This engine contains code from Nvidia Corporation." );

				ImGui::Separator();

				ImGui::TextWrapped( "Yan Chernikov (Cherno), for providing his game engine series." );
				ImGui::TextWrapped( "Sebastian Kylander (Gaztin), for providing code for to allow for a native window to move without a title bar." );

				ImGui::Separator();

				ImGui::TextWrapped( "assimp, for loading models." );
				ImGui::TextWrapped( "entt, for a good and fast ECS." );
				ImGui::TextWrapped( "GLFW, for windowing." );
				ImGui::TextWrapped( "glm, for opengl math." );
				ImGui::TextWrapped( "Dear Imgui, for an immediate mode gui." );
				ImGui::TextWrapped( "physx, for 3d Physics." );
				ImGui::TextWrapped( "shaderc, for providing tools for tools for vulkan shader compilation." );
				ImGui::TextWrapped( "spdlog, for providing an easy and fast logging system." );
				ImGui::TextWrapped( "SPIRV-Reflect, for proving a reflection tool for SPRIV shaders." );
				ImGui::TextWrapped( "stbi, for loading PNG/TGA, etc. images." );
				ImGui::TextWrapped( "vulkan, for vulkan headers." );

				ImGui::Separator();

				ImGui::TextWrapped( "https://vkguide.dev/" );
				ImGui::TextWrapped( "https://vulkan-tutorial.com/" );

				if( ImGui::Button( "Close" ) )
				{
					ShowAcknowledgements = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
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
						exit( 0 /* EXIT_SUCCESS */ );

					buttonRect.Min.x -= buttonSize;
					buttonRect.Max.x -= buttonSize;
				}

				// Maximize button
				{
					bool hovered = false;
					bool held    = false;
					bool pressed = ImGui::ButtonBehavior( buttonRect, ImGui::GetID( "MAXIMIZE" ), &hovered, &held, 0 );

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
						Window::Get().Maximize();

					buttonRect.Min.x -= buttonSize;
					buttonRect.Max.x -= buttonSize;
				}

				// Minimize button
				{
					bool hovered = false;
					bool held    = false;
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

		//ImGui::PopStyleVar();
	}

}
