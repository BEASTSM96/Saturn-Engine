#pragma once

#include <string>

namespace Sparky {

	class Shader
	{
	public:
		Shader(std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind() const;
		void Unbind() const;

	private:
		uint32_t m_RendererID;
	};
}