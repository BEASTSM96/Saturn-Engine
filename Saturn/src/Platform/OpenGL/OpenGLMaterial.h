#pragma once

#include "Saturn/Renderer/Material.h"
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Saturn {

	class OpenGLMaterial : public Material
	{
	public:
		OpenGLMaterial(glm::vec3 Ambient, glm::vec3 Diffuse, glm::vec3 Specular, GLuint DiffuseTexture, GLuint SpecularTexture);
		virtual ~OpenGLMaterial();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SendToShader(GLuint ShaderID) override;

		virtual void SendToShader(DShader Shader) override;

	private:
		glm::vec3 Ambient;
		glm::vec3 Diffuse;
		glm::vec3 Specular;

		GLuint DiffuseTexture;
		GLuint SpecularTexture;
	};
}