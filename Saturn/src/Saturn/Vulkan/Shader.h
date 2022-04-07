/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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

#include "Base.h"
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>

class Shader;

class ShaderWorker
{
	SINGLETON( ShaderWorker );

public:
	ShaderWorker() {}
	~ShaderWorker() 
	{
		for ( auto pShader : m_Shaders )
		{
			delete pShader;
			pShader = nullptr;
		}

		m_Shaders.clear();
		m_ShaderCodes.clear();
	}

	void AddAndCompileShader( Shader* pShader );
	void AddShader( Shader* pShader );

	std::vector< uint32_t >& GetShaderCode( std::string Name ) 
	{
		if( m_ShaderCodes.find( Name ) != m_ShaderCodes.end() )
		{
			return m_ShaderCodes[ Name ];
		}
	}

	void CompileShader( Shader* pShader );

private:


	// Not a list of all compiled shaders.
	std::vector< Shader* > m_Shaders;
	std::unordered_map< std::string, std::vector<uint32_t> > m_ShaderCodes;
};

// #TODO: Make it so that more than one shader file can correspond to one shader class
class Shader
{
public:
	Shader() {}
	Shader( std::string Name, std::filesystem::path Filepath );
	~Shader();

private:

	std::string m_FileContents = "";

	std::string m_Name = "";

	std::filesystem::path m_Filepath = "";

	void ReadFile();

private:
	friend class ShaderWorker;
};