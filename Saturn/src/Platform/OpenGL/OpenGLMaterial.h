#pragma once

#include "Saturn/Renderer/Material.h"
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Saturn/Renderer/3D/3dShader.h"

namespace Saturn {

	class OpenGLMaterial : public Material
	{
	public:
		OpenGLMaterial(std::string Name, glm::vec3 Ambient, glm::vec3 Diffuse, glm::vec3 Specular, GLuint DiffuseTexture, GLuint SpecularTexture);
		virtual ~OpenGLMaterial();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SendToShader(DShader Shader) override;
		virtual void SendToShader(DShader* Shader) override;

		std::string GetName() override {
			return MaterialName;
		}

	private:
		glm::vec3 Ambient;
		glm::vec3 Diffuse;
		glm::vec3 Specular;

		GLuint DiffuseTexture;
		GLuint SpecularTexture;

		std::string MaterialName;
	};
}