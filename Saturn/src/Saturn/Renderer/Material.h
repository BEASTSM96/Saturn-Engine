#pragma once

#include "Saturn/Core.h"
#include "3D/3dShader.h"
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Saturn {

<<<<<<< HEAD
	enum class MaterialFlag
	{
		None = BIT(0),
		DepthTest = BIT(1),
		Blend = BIT(2)
	};

	class SATURN_API Material : RefCounted
=======
	class SATURN_API Material
>>>>>>> parent of 0ef25b2... TestCommit
	{
	public:
		virtual ~Material() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SendToShader(GLuint ShaderID) = 0;

		virtual void SendToShader(DShader Shader) = 0;

		static Material* Create(glm::vec3 Ambient, glm::vec3 Diffuse, glm::vec3 Specular, GLuint DiffuseTexture, GLuint SpecularTexture);

	};

}