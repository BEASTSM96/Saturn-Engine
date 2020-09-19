#pragma once

#include "Saturn/Renderer/Shader.h"
#include <string>
#include <glm/glm.hpp>

namespace Saturn {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		void UploadUniformFloat4(const std::string& name, const glm::vec4& val);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);

		void UploadUniformFloat2(const std::string& name, const glm::vec2& val);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& val);
		void UploadUniformInt(const std::string& name, const int val);
	private:
		uint32_t m_RendererID;
	};
}

