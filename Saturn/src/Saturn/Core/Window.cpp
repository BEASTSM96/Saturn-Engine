#include "sppch.h"
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

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
		glfwWindowHint( GLFW_DECORATED, GLFW_TRUE );

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

	#if defined( SAT_PLATFORM_WINDOWS )

	#endif
	}

	Window::~Window()
	{
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

}