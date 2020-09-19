#include "sppch.h"
#include "Shader.h"

#include "Renderer.h"

#include "Platform\OpenGL\OpenGLShader.h"

namespace Saturn {

	Shader* Shader::Create(std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: SAT_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
			case RendererAPI::API::OpenGL: return new OpenGLShader(vertexSrc, fragmentSrc);
		}

		SAT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
