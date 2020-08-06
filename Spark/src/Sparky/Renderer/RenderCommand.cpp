#include "sppch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"


namespace Sparky {
	RendererAPI * RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}