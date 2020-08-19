#pragma once

#include <string>

namespace Sparky {

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;


		static Shader* Create(std::string& vertexSrc, const std::string& fragmentSrc);

	};
}