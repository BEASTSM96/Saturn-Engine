#pragma once

#include "Saturn/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Saturn {

	DISABLE_ALL_WARNINGS_BEGIN

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext( GLFWwindow* windowHandle );

		virtual void Init( void ) override;
		virtual void SwapBuffers( void ) override;

	private:
		GLFWwindow* m_WindowHandle;
	};

	DISABLE_ALL_WARNINGS_END

}