#include "sppch.h"
#include "Material.h"

#include "Renderer.h"

#include "Platform\OpenGL\OpenGLMaterial.h"

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////////////
	// Material
	//////////////////////////////////////////////////////////////////////////////////

	Material* Material::Create(std::string Name, glm::vec3 Ambient, glm::vec3 Diffuse, glm::vec3 Specular, GLuint DiffuseTexture, GLuint SpecularTexture)
	{

		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    SAT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLMaterial>::Create(spec);
		}


		SAT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}


	//////////////////////////////////////////////////////////////////////////////////
	// MaterialInstance
	//////////////////////////////////////////////////////////////////////////////////

	Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& material)
	{
		return Ref<MaterialInstance>::Create(material);
	}
}