#include "sppch.h"
#include "VertexArray.h"

#include "Renderer.h"

#include "Platform\OpenGL\OpenGLVertexArray.h"

namespace Sparky {

	VertexArray * VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None: SP_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
		case RendererAPI::OpenGL: return new OpenGLVertexArray();
		}

		SP_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
