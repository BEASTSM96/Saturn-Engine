#include "sppch.h"
#include "Shader.h"

#include "Renderer.h"

#include "Platform\OpenGL\OpenGLShader.h"

namespace Saturn {

	Shader* Shader::Create(std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None: SAT_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
			case RendererAPIType::OpenGL: return new OpenGLShader(vertexSrc, fragmentSrc);
		}

		SAT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
