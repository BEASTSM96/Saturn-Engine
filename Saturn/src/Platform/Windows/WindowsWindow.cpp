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
#include "WindowsWindow.h"

#include "Saturn/Events/ApplicationEvent.h"
#include "Saturn/Events/Event.h"
#include "Saturn/Events/KeyEvent.h"
#include "Saturn/Events/MouseEvent.h"

#include "Platform/OpenGL/OpenGLContext.h"

#include "GLFW/glfw3.h"

#include "stb_image.h"

namespace Saturn {

	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback( int error, const char* description )
	{
		SAT_CORE_ERROR( "GLFW Error ({0}): {1}", error, description );
	}

	Window* Window::Create( const WindowProps& props )
	{
		return new WindowsWindow( props );
	}

	WindowsWindow::WindowsWindow( const WindowProps& props )
	{
		Init( props );
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init( const WindowProps& props )
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		SAT_CORE_INFO( "Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height );


		if( !s_GLFWInitialized )
		{
			// TODO: glfwTerminate on system shutdown
			int success = glfwInit();
			SAT_CORE_ASSERT( success, "Could not intialize GLFW!" );
			glfwSetErrorCallback( GLFWErrorCallback );
			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow( ( int )props.Width, ( int )props.Height, m_Data.Title.c_str(), nullptr, nullptr );

		m_Context = new OpenGLContext( m_Window );
		m_Context->Init();

		glfwSetWindowUserPointer( m_Window, &m_Data );
		SetVSync( true );

		// Set GLFW callbacks
		glfwSetWindowSizeCallback( m_Window, []( GLFWwindow* window, int width, int height )
			{
				WindowData& data = *( WindowData* )glfwGetWindowUserPointer( window );
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event( width, height );
				data.EventCallback( event );
			} );

		glfwSetWindowCloseCallback( m_Window, []( GLFWwindow* window )
			{
				WindowData& data = *( WindowData* )glfwGetWindowUserPointer( window );
				WindowCloseEvent event;
				data.EventCallback( event );
			} );

		glfwSetKeyCallback( m_Window, []( GLFWwindow* window, int key, int scancode, int action, int mods )
			{
				WindowData& data = *( WindowData* )glfwGetWindowUserPointer( window );

				switch( action )
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent event( key, 0 );
						data.EventCallback( event );
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent event( key );
						data.EventCallback( event );
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent event( key, 1 );
						data.EventCallback( event );
						break;
					}
				}
			} );

		glfwSetCharCallback( m_Window, []( GLFWwindow* window, unsigned int keycode )
			{
				WindowData& data = *( WindowData* )glfwGetWindowUserPointer( window );

				KeyTypedEvent event( keycode );
				data.EventCallback( event );
			} );

		glfwSetMouseButtonCallback( m_Window, []( GLFWwindow* window, int button, int action, int mods )
			{
				WindowData& data = *( WindowData* )glfwGetWindowUserPointer( window );

				switch( action )
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event( button );
						data.EventCallback( event );
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event( button );
						data.EventCallback( event );
						break;
					}
				}
			} );

		glfwSetScrollCallback( m_Window, []( GLFWwindow* window, double xOffset, double yOffset )
			{
				WindowData& data = *( WindowData* )glfwGetWindowUserPointer( window );

				MouseScrolledEvent event( ( float )xOffset, ( float )yOffset );
				data.EventCallback( event );
			} );

		glfwSetCursorPosCallback( m_Window, []( GLFWwindow* window, double xPos, double yPos )
			{
				WindowData& data = *( WindowData* )glfwGetWindowUserPointer( window );

				MouseMovedEvent event( ( float )xPos, ( float )yPos );
				data.EventCallback( event );
			} );
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow( m_Window );
		s_GLFWInitialized = false;
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync( bool enabled )
	{
		if( enabled )
			glfwSwapInterval( 1 );
		else
			glfwSwapInterval( 0 );

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

	void WindowsWindow::Maximize( void )
	{
		glfwMaximizeWindow( m_Window );
	}

	void WindowsWindow::Minimize( void )
	{
		SAT_CORE_ASSERT( false );
	}

	void WindowsWindow::Restore( void )
	{
		glfwRestoreWindow( m_Window );
	}

	void WindowsWindow::SetTitle( const std::string& title )
	{
		m_Data.Title = title;
		glfwSetWindowTitle( m_Window, m_Data.Title.c_str() );
	}

	void WindowsWindow::SetWindowImage( const char* filepath )
	{
		GLFWimage image[ 1 ];
		image[ 0 ].pixels = stbi_load( filepath, &image[ 0 ].width, &image[ 0 ].height, 0, 4 );
		glfwSetWindowIcon( m_Window, 1, image );
		stbi_image_free( image[ 0 ].pixels );
	}
}
