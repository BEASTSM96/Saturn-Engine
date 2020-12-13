#include "sppch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>

#include <glad/glad.h>


namespace Saturn {

	OpenGLContext::OpenGLContext( GLFWwindow* windowHandle )
		: m_WindowHandle( windowHandle )
	{
		SAT_CORE_ASSERT( windowHandle, "Window handle is null!" )
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent( m_WindowHandle );
		int status = gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress );
		SAT_CORE_FATAL( "Failed to load Glad!" );
		SAT_CORE_ASSERT( status, "Failed to load Glad!" );

		SAT_CORE_INFO("OpenGl Renderer: {0} {1}", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	}

	void OpenGLContext::SwapBuffers()
	{

		glfwSwapBuffers( m_WindowHandle );
	}

}