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