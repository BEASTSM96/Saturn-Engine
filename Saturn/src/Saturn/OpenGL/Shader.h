/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#pragma once

#include "Common.h"
#include <string>

namespace Saturn {

	enum class ShaderDomain
	{
		None = 0, Vertex = 0, Pixel = 1
	};

	class MaterialUniform;

	class Shader
	{
	public:
		Shader() = default;
		Shader( const std::string& filename );

		void Bind();
		RendererID GetRendererID() { return m_ID; }

		const std::string& Name() const { return m_Name; }
		std::string& Name() { return m_Name; }

		// Uniform Funcs

		void SetBool( const std::string& name, bool val );
		void SetInt( const std::string& name, int val );
		void SetIntArray( const std::string& name, int32_t* vals, uint32_t count );
		void SetFloat( const std::string& name, float value );
		void SetFloat2( const std::string& name, const glm::vec2& val );
		void SetFloat3( const std::string& name, const glm::vec3& val );
		void SetFloat4( const std::string& name, const glm::vec4& val );
		void SetMat4( const std::string& name, const glm::mat4& val );

		void BindMaterialTextures();

	private:

		void Load( const std::string& filepath );
		void Parse();

		void CompileAndUploadShader();
		std::string ReadShaderFromFile( const std::string& src );

		const char* FindToken( const char* shader, const std::string& token );
		std::string GetStatement( const char* str, const char** outPosition );

		void ParseUniform( const std::string& statement, ShaderDomain domain );

		std::unordered_map<unsigned int, std::string> DetermineShaderTypes( const std::string& filepath );

		unsigned int ShaderTypeFromString( const std::string& type );

	private:

		RendererID m_ID = 0;
		bool m_Loaded = false;
		bool m_IsCompute = false;
		std::string m_Name, m_Filepath;

		// We technically have 2 or more shaders in one file, so we make a map here
		std::unordered_map<unsigned int, std::string> m_Shaders;

		std::vector<MaterialUniform> m_ShaderUniforms;
	};
}