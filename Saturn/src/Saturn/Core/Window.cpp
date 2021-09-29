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
#include "Window.h"

#include "App.h"
#include "Saturn/ImGui/Styles.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <imgui.h>
#if !defined ( SAT_DONT_USE_GL )
#include "examples/imgui_impl_opengl3.h"
#include "examples/imgui_impl_glfw.h"
#endif

#if defined ( SAT_PLATFORM_WINDOWS )
#include <dwmapi.h>
#endif

namespace Saturn {

	void GLFWErrorCallback( int error, const char* desc )
	{
		SAT_CORE_ERROR( "GLFW Error {0}, {1}", error, desc );
	}

	Window::Window()
	{
		glfwSetErrorCallback( GLFWErrorCallback );

		if( glfwInit() == GLFW_FALSE )
			return;

		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	#if defined( SAT_PLATFORM_WINDOWS_A )
		glfwWindowHint( GLFW_DECORATED, GLFW_FALSE );
	#else
		glfwWindowHint( GLFW_DECORATED, GLFW_TRUE );
	#endif

		m_Window = glfwCreateWindow( m_Width, m_Height, m_Title.c_str(), nullptr, nullptr );

		// Make Current before loading OpenGL
		glfwMakeContextCurrent( m_Window );

	#if !defined ( SAT_DONT_USE_GL )
		int result = gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress );
		if( result == 0 )
		{
			SAT_CORE_ERROR( "Failed to load OpenGL with Glad!" );
		}
		SAT_CORE_INFO( "OpenGL Renderer: {2}, {0}, {1}", glGetString( GL_VENDOR ), glGetString( GL_RENDERER ), glGetString( GL_VERSION ) );
	#endif

		glfwSetWindowUserPointer( m_Window, this );
		glfwSwapInterval( GLFW_TRUE );	

		// Set GLFW events
		glfwSetWindowCloseCallback( m_Window, []( GLFWwindow* window ) { Application::Get().Close(); } );

		glfwSetWindowSizeCallback( m_Window, SizeCallback );

	#if defined( SAT_PLATFORM_WINDOWS ) && defined ( SAT_PLATFORM_WINDOWS_A )
		HWND      WindowHandle    = glfwGetWin32Window( m_Window );
		HINSTANCE Instance        = GetModuleHandle( nullptr );

		SetWindowLong( WindowHandle, GWL_STYLE, GetWindowLong( WindowHandle, GWL_STYLE ) | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX );

		// Fix missing drop shadow
		MARGINS ShadowMargins;
		ShadowMargins ={ 1, 1, 1, 1 };
		DwmExtendFrameIntoClientArea( WindowHandle, &ShadowMargins );

		// Override window procedure with custom one to allow native window moving behavior without a title bar
		SetWindowLongPtr( WindowHandle, GWLP_USERDATA, ( LONG_PTR )this );
		m_WindowProc = ( WNDPROC )SetWindowLongPtr( WindowHandle, GWLP_WNDPROC, ( LONG_PTR )WindowProc );
	#endif

		// Init ImGui

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		// ImGui Theme

		ImGuiIO& io = ImGui::GetIO(); ( void )io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;      
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			style.WindowRounding = 0.0f;
			style.Colors[ ImGuiCol_WindowBg ].w = 1.0f;
		}

		Styles::Dark();

	#if !defined ( SAT_DONT_USE_GL )
		ImGui_ImplGlfw_InitForOpenGL( m_Window, true );
		ImGui_ImplOpenGL3_Init( "#version 410" );
	#endif
	}

	Window::~Window()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();

		glfwDestroyWindow( m_Window );
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers( m_Window );
	}

	void Window::Maximize()
	{
		glfwMaximizeWindow( m_Window );
	}

	void Window::Restore()
	{
		glfwRestoreWindow( m_Window );
	}

	void Window::SetTitle( const std::string& title )
	{
		m_Title = title;
		glfwSetWindowTitle( m_Window, m_Title.c_str() );
	}

	void Window::NewFrame()
	{
		if( !Application::Get().Running() )
			return;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		bool demo = true;
		ImGui::ShowDemoWindow( &demo );
	}

	void Window::EndFrame()
	{
		if( !Application::Get().Running() )
			return;

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( ( float )m_Width, ( float )m_Height );

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

	void Window::SizeCallback( GLFWwindow* wind, int h, int w )
	{
		Window* window = ( Window* )glfwGetWindowUserPointer( wind );

		window->m_Height = h;
		window->m_Width = w;
	}

#if defined ( SAT_PLATFORM_WINDOWS )

#endif
}