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

#include "Saturn/OpenGL/Renderer.h"
#include "Input.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#if !defined ( SAT_DONT_USE_GL )
#include "examples/imgui_impl_opengl3.h"
#include "examples/imgui_impl_glfw.h"
#endif

#if defined ( SAT_WINDOWS )
#include <dwmapi.h>
#include <Windows.h>
#endif

// TEMP
// FIXME
#include "Saturn/OpenGL/Framebuffer.h"

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

	#if defined ( SAT_DEBUG ) && !defined ( SAT_DONT_USE_GL )
		glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );
	#endif

	#if defined( SAT_WINDOWS_A )
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
		glfwSetWindowCloseCallback( m_Window, []( GLFWwindow* window )                            { Application::Get().Close();                                     } );
		glfwSetWindowSizeCallback( m_Window, SizeCallback );
		glfwSetFramebufferSizeCallback( m_Window, []( GLFWwindow* window, int width, int height ) { Renderer::Get().Resize( width, height );                        } );

		glfwSetScrollCallback( m_Window, []( GLFWwindow* window, double xOffset, double yOffset ) 
		{ 
			Window& win = *( Window* )glfwGetWindowUserPointer( window );

			MouseScrolledEvent event( ( float )xOffset, ( float )yOffset );
			win.m_EventCallback( event );
		} );

		glfwSetCursorPosCallback( m_Window, []( GLFWwindow* window, double x, double y ) 
		{ 
			Window& win = *( Window* )glfwGetWindowUserPointer( window );

			MouseMovedEvent event( ( float )x, ( float )y );
			win.m_EventCallback( event );
		} );

		glfwSetKeyCallback( m_Window, []( GLFWwindow* window, int key, int scancode, int action, int mods )
		{
			Window& win = *( Window* )glfwGetWindowUserPointer( window );

			switch( action )
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event( key, 0 );
					win.m_EventCallback( event );
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event( key );
					win.m_EventCallback( event );
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event( key, 1 );
					win.m_EventCallback( event );
					break;
				}
			}
	    } );

	#if defined( SAT_WINDOWS_A )

		// Thanks to Geno for this code https://github.com/Geno-IDE/Geno

		HWND      windowHandle    = glfwGetWin32Window( m_Window );
		HINSTANCE instance        = GetModuleHandle( nullptr );

		SetWindowLong( windowHandle, GWL_STYLE, GetWindowLong( windowHandle, GWL_STYLE ) | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX );

		// Fix missing drop shadow
		MARGINS shadowMargins;
		shadowMargins ={ 1, 1, 1, 1 };
		DwmExtendFrameIntoClientArea( windowHandle, &shadowMargins );

		// Override window procedure with custom one to allow native window moving behavior without a title bar
		SetWindowLongPtr( windowHandle, GWLP_USERDATA, ( LONG_PTR )this );
		m_WindowProc = ( WNDPROC )SetWindowLongPtr( windowHandle, GWLP_WNDPROC, ( LONG_PTR )WindowProc );
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

		m_TitleBar = new TitleBar();
	}

	Window::~Window()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();

		glfwDestroyWindow( m_Window );
	}

	void Window::OnUpdate()
	{
		if( Minimized )
			return;

		glfwPollEvents();
	}

	void Window::Maximize()
	{
		const bool wasMaximized = ( glfwGetWindowAttrib( m_Window, GLFW_MAXIMIZED ) == GLFW_TRUE );

		if( !wasMaximized ) { glfwMaximizeWindow( m_Window ); Minimized = false; }
		else Restore();
	}

	void Window::Minimize()
	{
		const bool wasMinimize = ( glfwGetWindowAttrib( m_Window, GLFW_ICONIFIED ) == GLFW_TRUE );

		if( !wasMinimize ) { Minimized = true; glfwIconifyWindow( m_Window ); }
		else Restore();
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

	void Window::Render()
	{
		// Was NewFrame

		if( !Application::Get().Running() )
			return;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		m_TitleBar->Draw();

		ImGui::Begin( "viewport" );
		{
			auto viewportSize = ImGui::GetContentRegionAvail();

			Renderer::Get().RendererCamera().SetProjectionMatrix( glm::perspectiveFov( glm::radians( 45.0f ), viewportSize.x, viewportSize.y, 0.1f, 10000.0f ) );
			Renderer::Get().RendererCamera().SetViewportSize( viewportSize.x, viewportSize.y );

			ImGui::Image( ( ImTextureID )Renderer::Get().TargetFramebuffer().ColorAttachmentRendererID(), viewportSize, { 0, 1 }, { 1, 0 } );
			ImGui::Image( ( ImTextureID )0, viewportSize, { 0, 1 }, { 1, 0 } );
		}

		ImGui::End();

		// Was EndFrame

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( ( float )m_Width, ( float )m_Height );

		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent( backup_current_context );
		}

		glfwSwapBuffers( m_Window );
	}

	void Window::SizeCallback( GLFWwindow* wind, int h, int w )
	{
		Window* window = ( Window* )glfwGetWindowUserPointer( wind );

		window->m_Height = h;
		window->m_Width = w;
	}

#if defined ( SAT_WINDOWS )

	// Thanks to Geno for this code https://github.com/Geno-IDE/Geno

	LRESULT Window::WindowProc( HWND handle, UINT msg, WPARAM WParam, LPARAM LParam )
	{
		Window* self = ( Window* )GetWindowLongPtr( handle, GWLP_USERDATA );

		switch( msg )
		{
			case WM_NCHITTEST:
			{
				POINT mousePos;
				RECT  windowRect;

				GetCursorPos( &mousePos );
				GetWindowRect( handle, &windowRect );

				if( PtInRect( &windowRect, mousePos ) )
				{
					const int borderX = GetSystemMetrics( SM_CXFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER ) + 2;
					const int borderY = GetSystemMetrics( SM_CYFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER ) + 2;

					if( mousePos.y < ( windowRect.top + borderY ) )
					{
						if( mousePos.x < ( windowRect.left + borderX ) )        { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); return HTTOPLEFT; }
						else if( mousePos.x >= ( windowRect.right - borderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); return HTTOPRIGHT; }
						else                                                    { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS );   return HTTOP; }
					}
					else if( mousePos.y >= ( windowRect.bottom - borderY ) )
					{
						if( mousePos.x < ( windowRect.left + borderX ) )        { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); return HTBOTTOMLEFT; }
						else if( mousePos.x >= ( windowRect.right - borderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); return HTBOTTOMRIGHT; }
						else                                                    { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS );   return HTBOTTOM; }
					}
					else if( mousePos.x < ( windowRect.left + borderX ) )
					{
						ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeEW );
						return HTLEFT;
					}
					else if( mousePos.x >= ( windowRect.right - borderX ) )
					{
						ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeEW );
						return HTRIGHT;
					}
					else
					{
						// Drag the menu bar to move the window
						if( !ImGui::IsAnyItemHovered() && ( mousePos.y < ( windowRect.top + self->m_TitleBar->Height() ) ) )
							return HTCAPTION;
					}
				}
				break;
			}


			case WM_NCCALCSIZE:
			{
				// Preserve the old client area and align it with the upper-left corner of the new client area
				return 0;
				break;
			} 

			case WM_ENTERSIZEMOVE:
			{
				SetTimer( handle, 1, USER_TIMER_MINIMUM, NULL );
				break;
			} 

			case WM_EXITSIZEMOVE:
			{
				KillTimer( handle, 1 );
				break;
			} 

			case WM_TIMER:
			{
				const UINT_PTR timerID = ( UINT_PTR )WParam;

				if( timerID == 1 )
				{
					self->Render();
				}
				break;
			} 

			case WM_SIZE:
			case WM_MOVE:
			{
				self->Render();
				break;
			}
		}

		return CallWindowProc( self->m_WindowProc, handle, msg, WParam, LParam );
	}

#endif
}