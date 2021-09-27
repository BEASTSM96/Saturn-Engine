#include "sppch.h"
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Saturn {

	void GLFWErrorCallback( int error, const char* desc )
	{
		printf( "GLFW Error %i, %c", error, desc );
	}

	Window::Window()
	{
		glfwSetErrorCallback( GLFWErrorCallback );

		if( glfwInit() == GLFW_FALSE )
			return;
	#if defined ( SAT_PLATFORM_WINDOWS )
		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	#endif
		glfwWindowHint( GLFW_DECORATED, GLFW_TRUE );

		m_Window = glfwCreateWindow( m_Width, m_Height, m_Title.c_str(), nullptr, nullptr );

	#if !defined ( SAT_DONT_USE_GL )
		int result = gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress );
		if( result == 0 )
		{
			// Log Fail
		}
	#endif

		glfwSetWindowUserPointer( m_Window, this );
		glfwMakeContextCurrent( m_Window );
		glfwSwapInterval( GLFW_TRUE );	
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