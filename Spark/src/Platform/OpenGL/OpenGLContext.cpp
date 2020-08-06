#include "sppch.h"
#include "OpenGLContext.h"

#include <GLFW\glfw3.h>

#include <glad\glad.h>
#include <gl\GL.h>


namespace Sparky {

	OpenGLContext::OpenGLContext(GLFWwindow * windowHandle)
		: m_WindowHandle(windowHandle)
	{
		SP_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		SP_CORE_ASSERT(status, "Failed to load Glad!");

		//SP_CORE_INFO("OpenGl Renderer: {0} {1}", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	}

	void OpenGLContext::SwapBuffers()
	{

		glfwSwapBuffers(m_WindowHandle);
	}

}