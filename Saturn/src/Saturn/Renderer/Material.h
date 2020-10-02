#pragma once

#include "Saturn/Core.h"
#include "3D/3dShader.h"
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Saturn {

	class SATURN_API Material : RefCounted
	{
	public:
		virtual ~Material() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SendToShader(DShader Shader) = 0;
		virtual void SendToShader(DShader* Shader) = 0;

		virtual std::string GetName() = 0;

		static Material* Create(std::string Name, glm::vec3 Ambient, glm::vec3 Diffuse, glm::vec3 Specular, GLuint DiffuseTexture, GLuint SpecularTexture);

	};


}