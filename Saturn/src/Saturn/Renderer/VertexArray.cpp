#include "sppch.h"
#include "VertexArray.h"

#include "Renderer.h"

#include "Platform\OpenGL\OpenGLVertexArray.h"

namespace Saturn {

	VertexArray * VertexArray::Create()
	{
		VertexArray * va = nullptr;
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: SAT_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
			case RendererAPI::API::OpenGL: return new OpenGLVertexArray();
		}

		SAT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
