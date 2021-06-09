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
#include "Saturn/Core/Modules/Module.h"
#include "Saturn/Core/Modules/ModuleManager.h"
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
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
																	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
																	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
																	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;																	// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);

		ImVec4* colors = ImGui::GetStyle().Colors;

		if (m_Theme == 0)
		{
			colors[ ImGuiCol_Text ] = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.5f, 0.5f, 0.5f, 1.0f );
			colors[ ImGuiCol_WindowBg ] = ImVec4( 0.18f, 0.18f, 0.18f, 1.0f ); // Window background
			colors[ ImGuiCol_ChildBg ] = ImVec4( 1.0f, 1.0f, 1.0f, 0.0f );
			colors[ ImGuiCol_PopupBg ] = ImVec4( 0.08f, 0.08f, 0.08f, 0.94f );
			colors[ ImGuiCol_Border ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.5f );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.0f, 0.0f, 0.0f, 0.0f );
			colors[ ImGuiCol_FrameBg ] = ImVec4( 0.3f, 0.3f, 0.3f, 0.5f ); // Widget backgrounds
			colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.4f, 0.4f, 0.4f, 0.4f );
			colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.4f, 0.4f, 0.4f, 0.6f );
			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.04f, 0.04f, 0.04f, 1.0f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.29f, 0.29f, 0.29f, 1.0f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.0f, 0.0f, 0.0f, 0.51f );
			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.0f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.0f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.0f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.0f );
			colors[ ImGuiCol_CheckMark ] = ImVec4( 0.94f, 0.94f, 0.94f, 1.0f );
			colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.51f, 0.51f, 0.51f, 0.7f );
			colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.66f, 0.66f, 0.66f, 1.0f );
			colors[ ImGuiCol_Button ] = ImVec4( 0.44f, 0.44f, 0.44f, 0.4f );
			colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.46f, 0.47f, 0.48f, 1.0f );
			colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.42f, 0.42f, 0.42f, 1.0f );
			colors[ ImGuiCol_Header ] = ImVec4( 0.7f, 0.7f, 0.7f, 0.31f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.7f, 0.7f, 0.7f, 0.8f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.48f, 0.5f, 0.52f, 1.0f );
			colors[ ImGuiCol_Separator ] = ImVec4( 0.43f, 0.43f, 0.5f, 0.5f );
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.72f, 0.72f, 0.72f, 0.78f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.0f );
			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.91f, 0.91f, 0.91f, 0.25f );
			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.81f, 0.81f, 0.81f, 0.67f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.46f, 0.46f, 0.46f, 0.95f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.0f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.0f, 0.43f, 0.35f, 1.0f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.73f, 0.6f, 0.15f, 1.0f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.0f, 0.6f, 0.0f, 1.0f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.87f, 0.87f, 0.87f, 0.35f );
			colors[ ImGuiCol_ModalWindowDarkening ] = ImVec4( 0.8f, 0.8f, 0.8f, 0.35f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.0f, 1.0f, 0.0f, 0.9f );
			colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.60f, 0.6f, 0.6f, 1.0f );
			colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.0f, 1.0f, 1.0f, 0.7f );
		}

		if (m_Theme == 1)
		{
			colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.40f, 0.40f, 0.40f, 1.00f );
			colors[ ImGuiCol_ChildBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_WindowBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_PopupBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_Border ] = ImVec4( 0.12f, 0.12f, 0.12f, 0.71f );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.06f );
			colors[ ImGuiCol_FrameBg ] = ImVec4( 0.42f, 0.42f, 0.42f, 0.54f );
			colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.42f, 0.42f, 0.42f, 0.40f );
			colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.56f, 0.56f, 0.56f, 0.67f );
			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.19f, 0.19f, 0.19f, 1.00f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.22f, 0.22f, 0.22f, 1.00f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.17f, 0.17f, 0.17f, 0.90f );
			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.335f, 0.335f, 0.335f, 1.000f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.24f, 0.24f, 0.24f, 0.53f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.52f, 0.52f, 0.52f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.76f, 0.76f, 0.76f, 1.00f );
			colors[ ImGuiCol_CheckMark ] = ImVec4( 0.65f, 0.65f, 0.65f, 1.00f );
			colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.52f, 0.52f, 0.52f, 1.00f );
			colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.64f, 0.64f, 0.64f, 1.00f );
			colors[ ImGuiCol_Button ] = ImVec4( 0.54f, 0.54f, 0.54f, 0.35f );
			colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.52f, 0.52f, 0.52f, 0.59f );
			colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.76f, 0.76f, 0.76f, 1.00f );
			colors[ ImGuiCol_Header ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.47f, 0.47f, 0.47f, 1.00f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.76f, 0.76f, 0.76f, 0.77f );
			colors[ ImGuiCol_Separator ] = ImVec4( 0.000f, 0.000f, 0.000f, 0.137f );
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.700f, 0.671f, 0.600f, 0.290f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.702f, 0.671f, 0.600f, 0.674f );
			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.25f );
			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.67f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.73f, 0.73f, 0.73f, 0.35f );
			colors[ ImGuiCol_ModalWindowDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
			colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
			colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
			colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );

			colors[ ImGuiCol_DockingEmptyBg ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_Tab ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_TabHovered ] = ImVec4( 0.40f, 0.40f, 0.40f, 1.00f );
			colors[ ImGuiCol_TabActive ] = ImVec4( 0.33f, 0.33f, 0.33f, 1.00f );
			colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.33f, 0.33f, 0.33f, 1.00f );
			colors[ ImGuiCol_DockingPreview ] = ImVec4( 0.85f, 0.85f, 0.85f, 0.28f );
		}

		if (m_Theme == 2)
		{
			colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
			colors[ ImGuiCol_WindowBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
			colors[ ImGuiCol_ChildBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
			colors[ ImGuiCol_PopupBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
			colors[ ImGuiCol_Border ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
			colors[ ImGuiCol_FrameBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
			colors[ ImGuiCol_CheckMark ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
			colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
			colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.08f, 0.50f, 0.72f, 1.00f );
			colors[ ImGuiCol_Button ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
			colors[ ImGuiCol_Header ] = ImVec4( 0.22f, 0.22f, 0.22f, 1.00f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
			colors[ ImGuiCol_Separator ] = colors[ ImGuiCol_Border ];
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.41f, 0.42f, 0.44f, 1.00f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.29f, 0.30f, 0.31f, 0.67f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
			colors[ ImGuiCol_Tab ] = ImVec4( 0.08f, 0.08f, 0.09f, 0.83f );
			colors[ ImGuiCol_TabHovered ] = ImVec4( 0.33f, 0.34f, 0.36f, 0.83f );
			colors[ ImGuiCol_TabActive ] = ImVec4( 0.23f, 0.23f, 0.24f, 1.00f );
			colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
			colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
			colors[ ImGuiCol_DockingPreview ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.70f );
			colors[ ImGuiCol_DockingEmptyBg ] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
			colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
			colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
			colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
			colors[ ImGuiCol_ModalWindowDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
		}


		if (m_Theme == 3)
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 5.3f;
			style.FrameRounding = 2.3f;
			style.ScrollbarRounding = 0;

			//From the theme one
			colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
			colors[ ImGuiCol_WindowBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
			colors[ ImGuiCol_ChildBg ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
			colors[ ImGuiCol_PopupBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 0.94f );
			colors[ ImGuiCol_Border ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
			colors[ ImGuiCol_FrameBg ] = ImVec4( 0.33f, 0.33f, 0.33f, 0.54f );
			colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.49f, 0.49f, 0.49f, 0.40f );
			colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.41f, 0.41f, 0.41f, 0.67f );
			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.18f, 0.18f, 0.18f, 1.00f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
			colors[ ImGuiCol_CheckMark ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.39f, 0.39f, 0.39f, 1.00f );
			colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.49f, 0.49f, 0.49f, 1.00f );
			colors[ ImGuiCol_Button ] = ImVec4( 0.38f, 0.38f, 0.38f, 0.40f );
			colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
			colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
			colors[ ImGuiCol_Header ] = ImVec4( 0.65f, 0.65f, 0.65f, 0.31f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.34f, 0.34f, 0.34f, 0.80f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_Separator ] = ImVec4( 0.54f, 0.54f, 0.54f, 0.50f );
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.55f, 0.55f, 0.55f, 0.78f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.40f, 0.40f, 0.40f, 0.25f );
			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.44f, 0.44f, 0.44f, 0.67f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 0.95f );
			colors[ ImGuiCol_Tab ] = ImVec4( 0.21f, 0.21f, 0.21f, 0.86f );
			colors[ ImGuiCol_TabHovered ] = ImVec4( 0.27f, 0.27f, 0.27f, 0.80f );
			colors[ ImGuiCol_TabActive ] = ImVec4( 0.35f, 0.35f, 0.35f, 1.00f );
			colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.19f, 0.19f, 0.19f, 0.97f );
			colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_DockingPreview ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.70f );
			colors[ ImGuiCol_DockingEmptyBg ] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.44f, 0.69f, 1.00f, 1.00f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 0.66f, 0.71f, 1.00f, 1.00f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
			colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
			colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
			colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
			colors[ ImGuiCol_ModalWindowDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
		}

		if (m_Theme == 4)
		{
			ImGui::StyleColorsLight();
		}

		io.Fonts->AddFontDefault();

		static const ImWchar icons_ranges[] ={ ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\segoeui.ttf", 18.0f );
		io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges );
		io.FontDefault = io.Fonts->Fonts.back();

		ImGui_ImplOpenGL3_Init("#version 410");
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

			colors[ ImGuiCol_Text ] = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.5f, 0.5f, 0.5f, 1.0f );
			colors[ ImGuiCol_WindowBg ] = ImVec4( 0.18f, 0.18f, 0.18f, 1.0f ); // Window background
			colors[ ImGuiCol_ChildBg ] = ImVec4( 1.0f, 1.0f, 1.0f, 0.0f );
			colors[ ImGuiCol_PopupBg ] = ImVec4( 0.08f, 0.08f, 0.08f, 0.94f );
			colors[ ImGuiCol_Border ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.5f );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.0f, 0.0f, 0.0f, 0.0f );
			colors[ ImGuiCol_FrameBg ] = ImVec4( 0.3f, 0.3f, 0.3f, 0.5f ); // Widget backgrounds
			colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.4f, 0.4f, 0.4f, 0.4f );
			colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.4f, 0.4f, 0.4f, 0.6f );
			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.04f, 0.04f, 0.04f, 1.0f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.29f, 0.29f, 0.29f, 1.0f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.0f, 0.0f, 0.0f, 0.51f );
			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.0f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.0f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.0f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.0f );
			colors[ ImGuiCol_CheckMark ] = ImVec4( 0.94f, 0.94f, 0.94f, 1.0f );
			colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.51f, 0.51f, 0.51f, 0.7f );
			colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.66f, 0.66f, 0.66f, 1.0f );
			colors[ ImGuiCol_Button ] = ImVec4( 0.44f, 0.44f, 0.44f, 0.4f );
			colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.46f, 0.47f, 0.48f, 1.0f );
			colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.42f, 0.42f, 0.42f, 1.0f );
			colors[ ImGuiCol_Header ] = ImVec4( 0.7f, 0.7f, 0.7f, 0.31f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.7f, 0.7f, 0.7f, 0.8f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.48f, 0.5f, 0.52f, 1.0f );
			colors[ ImGuiCol_Separator ] = ImVec4( 0.43f, 0.43f, 0.5f, 0.5f );
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.72f, 0.72f, 0.72f, 0.78f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.0f );
			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.91f, 0.91f, 0.91f, 0.25f );
			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.81f, 0.81f, 0.81f, 0.67f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.46f, 0.46f, 0.46f, 0.95f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.0f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.0f, 0.43f, 0.35f, 1.0f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.73f, 0.6f, 0.15f, 1.0f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.0f, 0.6f, 0.0f, 1.0f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.87f, 0.87f, 0.87f, 0.35f );
			colors[ ImGuiCol_ModalWindowDarkening ] = ImVec4( 0.8f, 0.8f, 0.8f, 0.35f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.0f, 1.0f, 0.0f, 0.9f );
			colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.60f, 0.6f, 0.6f, 1.0f );
			colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.0f, 1.0f, 1.0f, 0.7f );
		}

		if( theme == 1 )
		{
			m_Theme = theme;

			colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.40f, 0.40f, 0.40f, 1.00f );
			colors[ ImGuiCol_ChildBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_WindowBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_PopupBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_Border ] = ImVec4( 0.12f, 0.12f, 0.12f, 0.71f );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.06f );
			colors[ ImGuiCol_FrameBg ] = ImVec4( 0.42f, 0.42f, 0.42f, 0.54f );
			colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.42f, 0.42f, 0.42f, 0.40f );
			colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.56f, 0.56f, 0.56f, 0.67f );
			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.19f, 0.19f, 0.19f, 1.00f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.22f, 0.22f, 0.22f, 1.00f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.17f, 0.17f, 0.17f, 0.90f );
			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.335f, 0.335f, 0.335f, 1.000f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.24f, 0.24f, 0.24f, 0.53f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.52f, 0.52f, 0.52f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.76f, 0.76f, 0.76f, 1.00f );
			colors[ ImGuiCol_CheckMark ] = ImVec4( 0.65f, 0.65f, 0.65f, 1.00f );
			colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.52f, 0.52f, 0.52f, 1.00f );
			colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.64f, 0.64f, 0.64f, 1.00f );
			colors[ ImGuiCol_Button ] = ImVec4( 0.54f, 0.54f, 0.54f, 0.35f );
			colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.52f, 0.52f, 0.52f, 0.59f );
			colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.76f, 0.76f, 0.76f, 1.00f );
			colors[ ImGuiCol_Header ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.47f, 0.47f, 0.47f, 1.00f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.76f, 0.76f, 0.76f, 0.77f );
			colors[ ImGuiCol_Separator ] = ImVec4( 0.000f, 0.000f, 0.000f, 0.137f );
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.700f, 0.671f, 0.600f, 0.290f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.702f, 0.671f, 0.600f, 0.674f );
			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.25f );
			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.67f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.73f, 0.73f, 0.73f, 0.35f );
			colors[ ImGuiCol_ModalWindowDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
			colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
			colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
			colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );

			colors[ ImGuiCol_DockingEmptyBg ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_Tab ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_TabHovered ] = ImVec4( 0.40f, 0.40f, 0.40f, 1.00f );
			colors[ ImGuiCol_TabActive ] = ImVec4( 0.33f, 0.33f, 0.33f, 1.00f );
			colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.33f, 0.33f, 0.33f, 1.00f );
			colors[ ImGuiCol_DockingPreview ] = ImVec4( 0.85f, 0.85f, 0.85f, 0.28f );
		}

		if( theme == 2 )
		{
			m_Theme = theme;

			colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
			colors[ ImGuiCol_WindowBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
			colors[ ImGuiCol_ChildBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
			colors[ ImGuiCol_PopupBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
			colors[ ImGuiCol_Border ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
			colors[ ImGuiCol_FrameBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
			colors[ ImGuiCol_CheckMark ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
			colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
			colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.08f, 0.50f, 0.72f, 1.00f );
			colors[ ImGuiCol_Button ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
			colors[ ImGuiCol_Header ] = ImVec4( 0.22f, 0.22f, 0.22f, 1.00f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
			colors[ ImGuiCol_Separator ] = colors[ ImGuiCol_Border ];
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.41f, 0.42f, 0.44f, 1.00f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.29f, 0.30f, 0.31f, 0.67f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
			colors[ ImGuiCol_Tab ] = ImVec4( 0.08f, 0.08f, 0.09f, 0.83f );
			colors[ ImGuiCol_TabHovered ] = ImVec4( 0.33f, 0.34f, 0.36f, 0.83f );
			colors[ ImGuiCol_TabActive ] = ImVec4( 0.23f, 0.23f, 0.24f, 1.00f );
			colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
			colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
			colors[ ImGuiCol_DockingPreview ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.70f );
			colors[ ImGuiCol_DockingEmptyBg ] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
			colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
			colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
			colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
			colors[ ImGuiCol_ModalWindowDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
		}


		if( theme == 3 )
		{
			m_Theme = theme;

			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 5.3f;
			style.FrameRounding = 2.3f;
			style.ScrollbarRounding = 0;

			//From the theme one
			colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
			colors[ ImGuiCol_WindowBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
			colors[ ImGuiCol_ChildBg ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
			colors[ ImGuiCol_PopupBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 0.94f );
			colors[ ImGuiCol_Border ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
			colors[ ImGuiCol_FrameBg ] = ImVec4( 0.33f, 0.33f, 0.33f, 0.54f );
			colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.49f, 0.49f, 0.49f, 0.40f );
			colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.41f, 0.41f, 0.41f, 0.67f );
			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.18f, 0.18f, 0.18f, 1.00f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
			colors[ ImGuiCol_CheckMark ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.39f, 0.39f, 0.39f, 1.00f );
			colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.49f, 0.49f, 0.49f, 1.00f );
			colors[ ImGuiCol_Button ] = ImVec4( 0.38f, 0.38f, 0.38f, 0.40f );
			colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
			colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
			colors[ ImGuiCol_Header ] = ImVec4( 0.65f, 0.65f, 0.65f, 0.31f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.34f, 0.34f, 0.34f, 0.80f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
			colors[ ImGuiCol_Separator ] = ImVec4( 0.54f, 0.54f, 0.54f, 0.50f );
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.55f, 0.55f, 0.55f, 0.78f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.40f, 0.40f, 0.40f, 0.25f );
			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.44f, 0.44f, 0.44f, 0.67f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 0.95f );
			colors[ ImGuiCol_Tab ] = ImVec4( 0.21f, 0.21f, 0.21f, 0.86f );
			colors[ ImGuiCol_TabHovered ] = ImVec4( 0.27f, 0.27f, 0.27f, 0.80f );
			colors[ ImGuiCol_TabActive ] = ImVec4( 0.35f, 0.35f, 0.35f, 1.00f );
			colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.19f, 0.19f, 0.19f, 0.97f );
			colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
			colors[ ImGuiCol_DockingPreview ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.70f );
			colors[ ImGuiCol_DockingEmptyBg ] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.44f, 0.69f, 1.00f, 1.00f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 0.66f, 0.71f, 1.00f, 1.00f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
			colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
			colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
			colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
			colors[ ImGuiCol_ModalWindowDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
		}

		if( theme == 4 )
		{
			m_Theme = theme;
			ImGui::StyleColorsLight();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
} //namespace