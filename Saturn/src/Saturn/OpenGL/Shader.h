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

	class Shader
	{
	public:

		Shader() = default;
		Shader( const std::string& filename );

		void Bind();
		RendererID GetRendererID() { return m_ID; }

		const std::string& GetName() { return m_Name; }

		// Uniform Funcs

		void SetBool( const std::string& name, bool val );
		void SetInt( const std::string& name, int val );
		void SetFloat( const std::string& name, float val );

	private:

		void Load( const std::string& filepath );
		void Parse();

		void CompileAndUploadShader();
		std::string ReadShaderFromFile( const std::string& src );

		const char* FindToken( const char* shader, const std::string& token );
		std::string GetStatement( const char* str, const char** outPosition );

		void ParseUniform( const std::string& statement, int domain );

		std::unordered_map<unsigned int, std::string> DetermineShaderTypes( const std::string& filepath );

		unsigned int ShaderTypeFromString( const std::string& type );

	private:

		RendererID m_ID = 0;
		bool m_Loaded = false;
		std::string m_Name, m_Filepath;

		// We technically have 2 shaders in one file, so we make a map here
		std::unordered_map<unsigned int, std::string> m_Shaders;

	private:

		const char* FTLSD_VertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
			"}\0";

		const char* FTLSD_FragmentShaderSource = "#version 330 core\n"
			"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
			"}\n\0";
	};

}