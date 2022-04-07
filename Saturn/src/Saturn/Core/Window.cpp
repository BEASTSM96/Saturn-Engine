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
#include "Window.h"

#include "App.h"
#include "Saturn/ImGui/Styles.h"

#if defined( SAT_DONT_USE_GL )
// dx
#else
#include "Saturn/OpenGL/Renderer.h"
#endif

#include "Input.h"

#include "Saturn/OpenGL/xGL.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include "backends/imgui_impl_glfw.h"

#if !defined ( SAT_DONT_USE_GL )
#include "backends/imgui_impl_opengl3.h"
#else
#include "backends/imgui_impl_dx12.h"
#endif

#include <vulkan.h>
#include <Saturn/Vulkan/Base.h>

#if defined ( SAT_WINDOWS )
#include <dwmapi.h>
#include <Windows.h>
#endif

// TEMP
// FIXME
#include "Saturn/OpenGL/Framebuffer.h"

#include <dxgidebug.h>
#include <D3d12SDKLayers.h>
#pragma comment(lib, "dxguid.lib")

namespace Saturn {

	ImFont* s_DefualtFont;

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
	#elif defined ( SAT_DONT_USE_GL ) && defined ( SAT_DONT_USE_DX ) && !defined ( SAT_DONT_USE_VK )
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	#endif

		glfwWindowHint( GLFW_DECORATED, GLFW_FALSE );

		m_Window = glfwCreateWindow( m_Width, m_Height, m_Title.c_str(), nullptr, nullptr );

		// Make Current before loading OpenGL
		glfwMakeContextCurrent( m_Window );

	#if !defined ( SAT_DONT_USE_GL )
		int result = xGL::LoadGL();
		if( result == 0 )
		{
			SAT_CORE_ERROR( "Failed to load OpenGL with xGL!" );
		}
		SAT_CORE_INFO( "OpenGL Renderer: {2}, {0}, {1}", glGetString( GL_VENDOR ), glGetString( GL_RENDERER ), glGetString( GL_VERSION ) );
	#endif

		glfwSetWindowUserPointer( m_Window, this );
		glfwSwapInterval( GLFW_TRUE );

		// Set GLFW events
		glfwSetWindowCloseCallback( m_Window, []( GLFWwindow* window ) { Application::Get().Close(); } );

		glfwSetWindowSizeCallback( m_Window, []( GLFWwindow* window, int w, int h )
		{
			Window& win = *( Window* )glfwGetWindowUserPointer( window );

			SizeCallback( window, w, h );

			WindowResizeEvent event( ( float )w, ( float )h );
			win.m_EventCallback( event );
		} );

		//glfwSetFramebufferSizeCallback( m_Window, []( GLFWwindow* window, int width, int height ) {                         } );

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

		glfwSetCharCallback( m_Window, []( GLFWwindow* window, unsigned int keycode )
		{
			Window& win = *( Window* )glfwGetWindowUserPointer( window );

			KeyTypedEvent event( keycode );
			win.m_EventCallback( event );
		} );

		glfwSetMouseButtonCallback( m_Window, []( GLFWwindow* window, int button, int action, int mods )
		{
			Window& win = *( Window* )glfwGetWindowUserPointer( window );

			switch( action )
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event( button );
					win.m_EventCallback( event );
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event( button );
					win.m_EventCallback( event );
					break;
				}
			}
		} );


	#if defined ( SAT_WINDOWS )

		// Thanks to Geno for this code https://github.com/Geno-IDE/Geno

		HWND      windowHandle    = glfwGetWin32Window( m_Window );
		HINSTANCE instance        = GetModuleHandle( nullptr );

		SetWindowLong( windowHandle, GWL_STYLE, GetWindowLong( windowHandle, GWL_STYLE ) | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX  );

		// Fix missing drop shadow
		MARGINS shadowMargins;
		shadowMargins ={ 1, 1, 1, 1 };
		DwmExtendFrameIntoClientArea( windowHandle, &shadowMargins );

		// Override window procedure with custom one to allow native window moving behavior without a title bar
		SetWindowLongPtr( windowHandle, GWLP_USERDATA, ( LONG_PTR )this );
		m_WindowProc = ( WNDPROC )SetWindowLongPtr( windowHandle, GWLP_WNDPROC, ( LONG_PTR )WindowProc );

	#endif

	}

	Window::~Window()
	{
	#if !defined ( SAT_DONT_USE_GL )

		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();

	#elif !defined ( SAT_DONT_USE_DX )

		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplDX12_Shutdown();

	#endif

		glfwDestroyWindow( m_Window );
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
	}

	void Window::Maximize()
	{
		const bool wasMaximized = ( glfwGetWindowAttrib( m_Window, GLFW_MAXIMIZED ) == GLFW_TRUE );

		if( !wasMaximized )
			glfwMaximizeWindow( m_Window ); 
		else
			Restore();
	}

	void Window::Minimize()
	{
		const bool wasMinimize = ( glfwGetWindowAttrib( m_Window, GLFW_ICONIFIED ) == GLFW_TRUE );

		if( !wasMinimize ) { m_Minimized = true; glfwIconifyWindow( m_Window ); }
		else Restore();
	}

	void Window::Restore()
	{
		m_Minimized = false;
		m_Maximized = false;

		glfwRestoreWindow( m_Window );
	}

	void Window::SetTitle( const std::string& title )
	{
		m_Title = title;
		glfwSetWindowTitle( m_Window, m_Title.c_str() );
	}

	void Window::Render()
	{
		if( !Application::Get().Running() )
			return;

		if( m_Rendering )
			return;

		if( !m_Maximized && m_Minimized )
			m_Minimized = false;

		if( m_Maximized && m_Minimized )
			m_Minimized = false;

		m_Rendering = true;

	#if !defined ( SAT_DONT_USE_GL )
		ImGui_ImplOpenGL3_NewFrame();
	#elif !defined( SAT_DONT_USE_DX )
		//dx
	#endif

		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();

		//m_Dockspace->Draw();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( ( float )m_Width, ( float )m_Height );

		//ImGui::Render();

	#if !defined ( SAT_DONT_USE_GL )
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
	#else
		// DX is in Renderer.h
	#endif

		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			//ImGui::UpdatePlatformWindows();
			//ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent( backup_current_context );
		}

		//glfwSwapBuffers( m_Window );

		m_Rendering = false;
	}

	void Window::ImGuiInit()
	{
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

		s_DefualtFont = io.Fonts->AddFontFromFileTTF( "assets\\Fonts\\NotoSans-Regular.ttf", 18.0f );
		io.FontDefault = io.Fonts->Fonts.back();

		Styles::Dark();

		// Call backends depend on Rendering API

		m_Dockspace = new ImGuiDockspace();
	}

	std::vector<const char*> Window::GetRequiredExtensions()
	{		
		uint32_t Count;
		const char** ppExtensions;
		
		ppExtensions = glfwGetRequiredInstanceExtensions( &Count );

		std::vector<const char*> Extensions( ppExtensions, ppExtensions + Count );

		Extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

		return Extensions;
	}

	VkResult Window::CreateWindowSurface( VkInstance& rInstance, VkSurfaceKHR* pSurface )
	{
		return glfwCreateWindowSurface( rInstance, m_Window, nullptr, pSurface );
	}

#if defined( _WIN32 )

	HWND Window::PlatformWindow()
	{
		return glfwGetWin32Window( m_Window );
	}

#endif

	void Window::GetSize( uint32_t* pWidth, uint32_t* pHeight )
	{
		glfwGetWindowSize( m_Window, ( int* )pWidth, ( int* )pHeight );
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

				if( !self->m_Maximized && PtInRect( &windowRect, mousePos ) )
				{
					const int borderX = GetSystemMetrics( SM_CXFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER );
					const int borderY = GetSystemMetrics( SM_CYFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER );

					if( mousePos.y < ( windowRect.top + borderY ) )
					{
						if( mousePos.x < ( windowRect.left + borderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); return HTTOPLEFT; }
						else if( mousePos.x >= ( windowRect.right - borderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); return HTTOPRIGHT; }
						else { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS );   return HTTOP; }
					}
					else if( mousePos.y >= ( windowRect.bottom - borderY ) )
					{
						if( mousePos.x < ( windowRect.left + borderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); return HTBOTTOMLEFT; }
						else if( mousePos.x >= ( windowRect.right - borderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); return HTBOTTOMRIGHT; }
						else { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS );   return HTBOTTOM; }
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
						if( !self->m_Maximized && !ImGui::IsAnyItemHovered() && ( mousePos.y < ( windowRect.top + self->m_Dockspace->GetTitleBar().Height() ) ) )
							return HTCAPTION;
					}
				}
			} break;

			case WM_NCCALCSIZE:
			{
				if( WParam /* TRUE */ )
				{
					WINDOWPLACEMENT windowPlacement { sizeof( WINDOWPLACEMENT ) };

					if( GetWindowPlacement( handle, &windowPlacement ) && windowPlacement.showCmd == SW_SHOWMAXIMIZED )
					{
						NCCALCSIZE_PARAMS& params = *reinterpret_cast< LPNCCALCSIZE_PARAMS >( LParam );
						const int borderX = GetSystemMetrics( SM_CXFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER );
						const int borderY = GetSystemMetrics( SM_CYFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER );

						params.rgrc[ 0 ].left += borderX;
						params.rgrc[ 0 ].top  += borderX;
						params.rgrc[ 0 ].right -= borderY;
						params.rgrc[ 0 ].bottom -= borderY;

						return WVR_VALIDRECTS;
					}
				}

				// Preserve the old client area and align it with the upper-left corner of the new client area
				return 0;
			} break;

			case WM_ENTERSIZEMOVE:
			{
				SetTimer( handle, 1, USER_TIMER_MINIMUM, NULL );
			} break;

			case WM_EXITSIZEMOVE:
			{
				KillTimer( handle, 1 );
			} break;

			case WM_TIMER:
			{
				const UINT_PTR TimerID = ( UINT_PTR )WParam;

				if( TimerID == 1 )
				{
					self->Render();
				}
			} break;

			case WM_SIZE: 
			{
				RECT clientRect ={};
				GetClientRect( handle, &clientRect );

				self->SizeCallback( self->m_Window, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top );
				self->Render();
			} break;

			case WM_MOVE:
			{
				self->Render();
			} break;

		}
		return CallWindowProc( self->m_WindowProc, handle, msg, WParam, LParam );
	}

#endif
}