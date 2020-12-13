#pragma once

#include "Saturn/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Saturn {

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext( GLFWwindow* windowHandle );

		virtual void Init( void ) override;
		virtual void SwapBuffers( void ) override;

	private:
		GLFWwindow* m_WindowHandle;
	};

}