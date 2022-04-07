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


#include "sppch.h"
#include "Shader.h"

#include <istream>
#include <fstream>
#include <iostream>

#include <shaderc/shaderc.hpp>
#include <shaderc/shaderc.h>
#include <cassert>

void ShaderWorker::AddAndCompileShader( Shader* pShader )
{
	AddShader( pShader );
	CompileShader( pShader );
}

void ShaderWorker::AddShader( Shader* pShader )
{
	m_Shaders.push_back( pShader );
}

void ShaderWorker::CompileShader( Shader* pShader )
{
	shaderc::Compiler       Compiler;
	shaderc::CompileOptions CompilerOptions;

	shaderc::SpvCompilationResult ShaderResult;

	CompilerOptions.SetOptimizationLevel( shaderc_optimization_level_performance );

	auto ShaderString = pShader->m_Filepath.string();
	auto ShaderCStr = ShaderString.c_str();

	auto Result = Compiler.CompileGlslToSpvAssembly( 
		pShader->m_FileContents.c_str(),
		pShader->m_FileContents.size(), 
		pShader->m_Filepath.extension() == ".vert" ? shaderc_shader_kind::shaderc_glsl_default_vertex_shader : shaderc_shader_kind::shaderc_glsl_default_fragment_shader, 
		ShaderCStr,
		CompilerOptions 
	);

	std::string ResultString( Result.begin(), Result.end() );

	auto AssembleResult = Compiler.AssembleToSpv( ResultString.c_str(), 4 * ResultString.size() );

	if( AssembleResult.GetCompilationStatus() != shaderc_compilation_status_success )
	{
		assert( 0 ); // Shader compilation failed.
	}

	// Save code for later if we need it.
	std::vector<uint32_t> Code( AssembleResult.begin(), AssembleResult.end() );
	m_ShaderCodes.insert( { pShader->m_Name, Code } );

	printf( "===== SHADER OUTPUT: =====\n%s\n", ResultString.c_str() );
	printf( "==========================\n \n" );
}

Shader::Shader( std::string Name, std::filesystem::path Filepath )
{
	m_Filepath = std::move( Filepath );
	m_Name = std::move( Name );

	ReadFile();
}

Shader::~Shader()
{

}

// I'm not even sure if this is good or not, and I think that using std::filesystem::file_size yields a differnet file size on other platforms.
void Shader::ReadFile()
{
	std::ifstream f( m_Filepath, std::ios::ate | std::ios::binary );

	size_t FileSize = static_cast< size_t >( f.tellg() );
	std::vector<char> Buffer( FileSize );

	f.seekg( 0 );
	f.read( Buffer.data(), FileSize );

	f.close();

	m_FileContents = std::string( Buffer.begin(), Buffer.end() );
}