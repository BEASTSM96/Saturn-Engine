#include "sppch.h"
#include "Material.h"

#include "Renderer.h"

#include "Platform\OpenGL\OpenGLMaterial.h"

namespace Saturn {

	Material * Material::Create(std::string Name, glm::vec3 Ambient, glm::vec3 Diffuse, glm::vec3 Specular, GLuint DiffuseTexture, GLuint SpecularTexture)
    {
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: SAT_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
			case RendererAPI::API::OpenGL: return new OpenGLMaterial(Name, Ambient, Diffuse, Specular, DiffuseTexture, SpecularTexture);
		}

		SAT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
    }
}