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
#include "ImGuiLayer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#include "Saturn/Application.h"
#include "Saturn/Core/Timestep.h"
#include "Saturn/Log.h"

// TEMPORARY
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define EDITOR
#ifdef EDITOR

#include "Saturn/Core/Serialisation/Serialiser.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Saturn/Scene/Components.h"

#include "ImGuizmo.h"

#include "Saturn/Scene/Components.h"
#include "Saturn/Renderer/SceneRenderer.h"
#include "Saturn/Renderer/Renderer2D.h"
#include "Saturn/Renderer/Renderer.h"
#include "Saturn/Core/Base.h"
#include "Saturn/MouseButtons.h"
#include "Saturn/Scene/SceneManager.h"

#include "Saturn/Input.h"

#include "Saturn/Scene/ScriptableEntity.h"

#include "SceneHierarchyPanel.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


#endif // EDITOR

#include "IconsForkAwesome.h"

namespace Saturn {

	///////////////////////////////////////
	////////////FORALLPLATFORMS///////////
	/////////////////////////////////////

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
		SAT_PROFILE_FUNCTION();

	}

	ImGuiLayer::ImGuiLayer(const std::string& name)
	{

	}

	void ImGuiLayer::OnAttach()
	{
		SAT_PROFILE_FUNCTION();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); ( void )io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
																	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
																	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
																	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;																	// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			style.WindowRounding = 0.0f;
			style.Colors[ ImGuiCol_WindowBg ].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast< GLFWwindow* >( app.GetWindow().GetNativeWindow() );

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL( window, true );

		SetTheme( m_Theme );

		static const ImWchar icons_ranges[] ={ ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config; 
		icons_config.MergeMode = true; 
		icons_config.PixelSnapH = true;

		ImFont* pFont = io.Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\segoeui.ttf", 18.0f );
		io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges );
		io.FontDefault = io.Fonts->Fonts.back();

		ImGui_ImplOpenGL3_Init( "#version 410" );
	}

	void ImGuiLayer::OnDetach()
	{
		SAT_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		SAT_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		SAT_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::OnImGuiRender()
	{
	}

	void ImGuiLayer::SetTheme( int theme )
	{
		ImVec4* colors = ImGui::GetStyle().Colors;

		if( theme == 0 )
		{
			m_Theme = theme;

			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4* colors = style->Colors;

			colors[ ImGuiCol_Text ]                   = ImVec4( 1.000f, 1.000f, 1.000f, 1.000f );
			colors[ ImGuiCol_TextDisabled ]           = ImVec4( 0.500f, 0.500f, 0.500f, 1.000f );
			colors[ ImGuiCol_WindowBg ]               = ImVec4( 0.180f, 0.180f, 0.180f, 1.000f );
			colors[ ImGuiCol_ChildBg ]                = ImVec4( 0.280f, 0.280f, 0.280f, 0.000f );
			colors[ ImGuiCol_PopupBg ]                = ImVec4( 0.313f, 0.313f, 0.313f, 1.000f );
			colors[ ImGuiCol_Border ]                 = ImVec4( 0.266f, 0.266f, 0.266f, 1.000f );
			colors[ ImGuiCol_BorderShadow ]           = ImVec4( 0.000f, 0.000f, 0.000f, 0.000f );
			colors[ ImGuiCol_FrameBg ]                = ImVec4( 0.160f, 0.160f, 0.160f, 1.000f );
			colors[ ImGuiCol_FrameBgHovered ]         = ImVec4( 0.200f, 0.200f, 0.200f, 1.000f );
			colors[ ImGuiCol_FrameBgActive ]          = ImVec4( 0.280f, 0.280f, 0.280f, 1.000f );
			colors[ ImGuiCol_TitleBg ]                = ImVec4( 0.148f, 0.148f, 0.148f, 1.000f );
			colors[ ImGuiCol_TitleBgActive ]          = ImVec4( 0.148f, 0.148f, 0.148f, 1.000f );
			colors[ ImGuiCol_TitleBgCollapsed ]       = ImVec4( 0.148f, 0.148f, 0.148f, 1.000f );
			colors[ ImGuiCol_MenuBarBg ]              = ImVec4( 0.195f, 0.195f, 0.195f, 1.000f );
			colors[ ImGuiCol_ScrollbarBg ]            = ImVec4( 0.160f, 0.160f, 0.160f, 1.000f );
			colors[ ImGuiCol_ScrollbarGrab ]          = ImVec4( 0.277f, 0.277f, 0.277f, 1.000f );
			colors[ ImGuiCol_ScrollbarGrabHovered ]   = ImVec4( 0.300f, 0.300f, 0.300f, 1.000f );
			colors[ ImGuiCol_ScrollbarGrabActive ]    = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_CheckMark ]              = ImVec4( 1.000f, 1.000f, 1.000f, 1.000f );
			colors[ ImGuiCol_SliderGrab ]             = ImVec4( 0.391f, 0.391f, 0.391f, 1.000f );
			colors[ ImGuiCol_SliderGrabActive ]       = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_Button ]                 = ImVec4( 1.000f, 1.000f, 1.000f, 0.000f );
			colors[ ImGuiCol_ButtonHovered ]          = ImVec4( 1.000f, 1.000f, 1.000f, 0.156f );
			colors[ ImGuiCol_ButtonActive ]           = ImVec4( 1.000f, 1.000f, 1.000f, 0.391f );
			colors[ ImGuiCol_Header ]                 = ImVec4( 0.313f, 0.313f, 0.313f, 1.000f );
			colors[ ImGuiCol_HeaderHovered ]          = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
			colors[ ImGuiCol_HeaderActive ]           = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
			colors[ ImGuiCol_Separator ]              = colors[ ImGuiCol_Border ];
			colors[ ImGuiCol_SeparatorHovered ]       = ImVec4( 0.391f, 0.391f, 0.391f, 1.000f );
			colors[ ImGuiCol_SeparatorActive ]        = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_ResizeGrip ]             = ImVec4( 1.000f, 1.000f, 1.000f, 0.250f );
			colors[ ImGuiCol_ResizeGripHovered ]      = ImVec4( 1.000f, 1.000f, 1.000f, 0.670f );
			colors[ ImGuiCol_ResizeGripActive ]       = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_Tab ]                    = ImVec4( 0.098f, 0.098f, 0.098f, 1.000f );
			colors[ ImGuiCol_TabHovered ]             = ImVec4( 0.352f, 0.352f, 0.352f, 1.000f );
			colors[ ImGuiCol_TabActive ]              = ImVec4( 0.195f, 0.195f, 0.195f, 1.000f );
			colors[ ImGuiCol_TabUnfocused ]           = ImVec4( 0.098f, 0.098f, 0.098f, 1.000f );
			colors[ ImGuiCol_TabUnfocusedActive ]     = ImVec4( 0.195f, 0.195f, 0.195f, 1.000f );
			colors[ ImGuiCol_DockingPreview ]         = ImVec4( 1.000f, 0.391f, 0.000f, 0.781f );
			colors[ ImGuiCol_DockingEmptyBg ]         = ImVec4( 0.180f, 0.180f, 0.180f, 1.000f );
			colors[ ImGuiCol_PlotLines ]              = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
			colors[ ImGuiCol_PlotLinesHovered ]       = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_PlotHistogram ]          = ImVec4( 0.586f, 0.586f, 0.586f, 1.000f );
			colors[ ImGuiCol_PlotHistogramHovered ]   = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_TextSelectedBg ]         = ImVec4( 1.000f, 1.000f, 1.000f, 0.156f );
			colors[ ImGuiCol_DragDropTarget ]         = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_NavHighlight ]           = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_NavWindowingHighlight ]  = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
			colors[ ImGuiCol_NavWindowingDimBg ]      = ImVec4( 0.000f, 0.000f, 0.000f, 0.586f );
			colors[ ImGuiCol_ModalWindowDimBg ]       = ImVec4( 0.000f, 0.000f, 0.000f, 0.586f );

			style->ChildRounding = 4.0f;
			style->FrameBorderSize = 1.0f;
			style->FrameRounding = 2.0f;
			style->GrabMinSize = 7.0f;
			style->PopupRounding = 2.0f;
			style->ScrollbarRounding = 12.0f;
			style->ScrollbarSize = 13.0f;
			style->TabBorderSize = 1.0f;
			style->TabRounding = 0.0f;
			style->WindowRounding = 4.0f;
			style->TabRounding = 4.0f;
		}

		if( theme == 4 )
		{
			m_Theme = theme;
			ImGui::StyleColorsLight();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
} //namespace