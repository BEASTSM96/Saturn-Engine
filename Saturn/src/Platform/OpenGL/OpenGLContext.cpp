#include "sppch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>

#include <glad/glad.h>


namespace Saturn {

	DISABLE_ALL_WARNINGS_BEGIN

	OpenGLContext::OpenGLContext( GLFWwindow* windowHandle )
		: m_WindowHandle( windowHandle )
	{
		SAT_CORE_ASSERT( windowHandle, "Window handle is null!" )
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent( m_WindowHandle );
		int status = gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress );
		if (status == 0)
		{
			SAT_CORE_FATAL( "Failed to load Glad!" );
		}
		SAT_CORE_ASSERT( status, "Failed to load Glad!" );

		SAT_CORE_INFO("OpenGL Renderer: {2}, {0}, {1}", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{

		glfwSwapBuffers( m_WindowHandle );
	}

	DISABLE_ALL_WARNINGS_END

}