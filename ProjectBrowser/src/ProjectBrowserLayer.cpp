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
#include <Saturn/ImGui/TitleBar.h>

#include <Saturn/Core/Window.h>

#include <glm/gtc/type_ptr.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h> 
#include <imgui_internal.h>

namespace Saturn {

	ProjectBrowserLayer::ProjectBrowserLayer()
	{
	}

	void ProjectBrowserLayer::OnAttach()
	{
		m_TilteBar = new TitleBar();
	}

	ProjectBrowserLayer::~ProjectBrowserLayer()
	{

	}
	
	void ProjectBrowserLayer::OnDetach()
	{
		delete m_TilteBar;
	}

	void ProjectBrowserLayer::OnUpdate( Timestep time )
	{

	}

	void ProjectBrowserLayer::OnImGuiRender()
	{
		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport( pViewport );

		// --- Title bar
		
		m_TilteBar->Draw();

		// ---
		
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;

		ImGui::Begin( "##PB", NULL, flags );

		if( ImGui::Button("Create project", ImVec2(ImGui::GetWindowWidth() / 2, 0) ) )
		{
			
		}

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