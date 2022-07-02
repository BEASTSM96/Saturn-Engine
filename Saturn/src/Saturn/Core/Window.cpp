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

#include "Input.h"
#include "Saturn/ImGui/Panel/PanelManager.h"

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

#include "backends/imgui_impl_vulkan.h"
#include "Saturn/Vulkan/VulkanContext.h"

#include <vulkan.h>
#include <Saturn/Vulkan/Base.h>

#if defined ( SAT_WINDOWS )
#include <dwmapi.h>
#include <Windows.h>
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

		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE );
		glfwWindowHint( GLFW_TITLEBAR, GLFW_FALSE );

		GLFWmonitor* pPrimary = glfwGetPrimaryMonitor();	
		
		int x, y, w, h;
		glfwGetMonitorWorkarea( pPrimary, &x, &y, &w, &h );

		m_Width = 3 * w / 4;
		m_Height = 3 * h / 4;
		
		// Create a 10x10 base window, the when we call "RemoveBorder" we resize the window to the actual size
		// Kind of a hack
		m_Window = glfwCreateWindow( 10, 10, m_Title.c_str(), nullptr, nullptr );

		glfwSetWindowUserPointer( m_Window, this );

		// Set GLFW events
		glfwSetWindowCloseCallback( m_Window, []( GLFWwindow* window ) { Application::Get().Close(); } );

		glfwSetWindowSizeCallback( m_Window, []( GLFWwindow* window, int w, int h )
		{
			Window& win = *( Window* ) glfwGetWindowUserPointer( window );

			SizeCallback( window, w, h );

			WindowResizeEvent event( ( float ) w, ( float ) h );
			win.m_EventCallback( event );
		} );

		glfwSetScrollCallback( m_Window, []( GLFWwindow* window, double xOffset, double yOffset )
		{
			Window& win = *( Window* )glfwGetWindowUserPointer( window );

			MouseScrolledEvent event( ( float )xOffset, ( float )yOffset );
			win.m_EventCallback( event );
		} );
		
		glfwSetTitlebarHitTestCallback( m_Window, []( GLFWwindow* window, int x, int y, int* pOut )
		{
			Window& win = *( Window* ) glfwGetWindowUserPointer( window );
				
			if( auto EditorLayer = Application::Get().GetEditorLayer() )
			{
				if( auto tb = EditorLayer->GetTitleBar() )
				{
					auto TitleBarHeight = tb->Height();
					
					RECT windowRect;
					POINT mousePos;
					GetClientRect( glfwGetWin32Window( win.m_Window ), &windowRect );

					*pOut = 1;
					
					// Drag the menu bar to move the window
					if( !win.m_Maximized && !ImGui::IsAnyItemHovered() && ( y < ( windowRect.top + TitleBarHeight ) ) )
						*pOut = 1;
					else
						*pOut = 0;
				}
			}
			} );

		glfwSetCursorPosCallback( m_Window, []( GLFWwindow* window, double x, double y )
			{
				Window& win = *( Window* ) glfwGetWindowUserPointer( window );

				MouseMovedEvent event( ( float ) x, ( float ) y );
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

		HWND      windowHandle    = glfwGetWin32Window( m_Window );
		HINSTANCE instance        = GetModuleHandle( nullptr );

		// Fix missing drop shadow
		MARGINS shadowMargins;
		shadowMargins = { 1, 1, 1, 1 };
		DwmExtendFrameIntoClientArea( windowHandle, &shadowMargins );

		// Override window procedure with custom one to allow native window moving behavior without a title bar
		SetWindowLongPtr( windowHandle, GWLP_USERDATA, ( LONG_PTR )this );
		m_WindowProc = ( WNDPROC )SetWindowLongPtr( windowHandle, GWLP_WNDPROC, ( LONG_PTR )WindowProc );

	#endif
	}

	Window::~Window()
	{
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
		{
			m_PendingMinimize = false;
			m_PendingMaximized = true;
		}
		else
			Restore();
	}

	void Window::Minimize()
	{
		const bool wasMinimize = ( glfwGetWindowAttrib( m_Window, GLFW_ICONIFIED ) == GLFW_TRUE );

		if( !wasMinimize ) 
		{ 
			m_PendingMinimize = true;
			m_PendingMaximized = false;
		}
		else 
			Restore();
	}

	void Window::Restore()
	{
		m_Minimized = false;
		m_Maximized = false;

		m_PendingRestore = true;
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

		// Because in VulkanContext we have end the render in order to move on, to do so we must minimize a later time.
		if( m_PendingMinimize )
		{
			glfwIconifyWindow( m_Window );

			m_Minimized = true;

			m_PendingMinimize = false;
		}

		if ( m_PendingMaximized )
		{
			glfwMaximizeWindow( m_Window );
			
			m_Maximized = true;
			m_PendingMaximized = false;
		}
		
		if( m_PendingRestore ) 
		{
			glfwRestoreWindow( m_Window );
			m_PendingRestore = false;
		}

		m_Rendering = true;
				
		// The window does a lot of rendering am I right?

		m_Rendering = false;
	}

	void Window::Show()
	{
		glfwShowWindow( m_Window );
	}

	void Window::RemoveBorder()
	{
		glfwSetWindowSize( m_Window, m_Width, m_Height );
	}

	void Window::ImGuiInit()
	{

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

	void Window::SizeCallback( GLFWwindow* wind, int w, int h )
	{
		Window* window = ( Window* )glfwGetWindowUserPointer( wind );

		if( w == 0 && h == 0 )
		{
			window->m_Minimized = true;
			return;
		}

		window->m_Width = w;
		window->m_Height = h;

		WindowResizeEvent event( ( float ) w, ( float ) h );
		window->m_EventCallback( event );
	}

#if defined ( SAT_WINDOWS )
	
	LRESULT Window::WindowProc( HWND handle, UINT msg, WPARAM WParam, LPARAM LParam )
	{
		Window* self = ( Window* )GetWindowLongPtr( handle, GWLP_USERDATA );
		
		switch( msg )
		{
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

			case WM_SIZE: 
			{
				if( WParam == SIZE_RESTORED )
				{
					self->m_Minimized = false;
					self->m_Maximized = false;
				}
				else if( SIZE_MAXIMIZED ) 
				{
					self->m_Minimized = false;
					self->m_Maximized = true;
				}
				else if( SIZE_MINIMIZED )
				{
					self->m_Minimized = true;
					self->m_Maximized = false;
				}
			} break;
		}

		return CallWindowProc( self->m_WindowProc, handle, msg, WParam, LParam );
	}

#endif
}