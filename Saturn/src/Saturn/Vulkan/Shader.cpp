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

namespace Saturn {

	void ShaderWorker::AddAndCompileShader( Shader* pShader )
	{
		AddShader( pShader );
		CompileShader( pShader );
	}

	void ShaderWorker::AddShader( Shader* pShader )
	{
		m_Shaders[ pShader->GetName() ] = pShader;
	}

	void ShaderWorker::CompileShader( Shader* pShader )
	{
		shaderc::Compiler       Compiler;
		shaderc::CompileOptions CompilerOptions;

		shaderc::SpvCompilationResult ShaderResult;

		CompilerOptions.SetOptimizationLevel( shaderc_optimization_level_performance );
		
	#if defined( _DEBUG )
		//CompilerOptions.SetGenerateDebugInfo();
	#endif
		
		for( auto [key, src] : pShader->m_ShaderSources )
		{
			auto ShaderSrcCode = src.Source;

			auto Result = Compiler.CompileGlslToSpvAssembly( 
				ShaderSrcCode.c_str(),
				ShaderSrcCode.size(),
				src.Type == ShaderType::Vertex ? shaderc_shader_kind::shaderc_glsl_default_vertex_shader : shaderc_shader_kind::shaderc_glsl_default_fragment_shader, 
				ShaderSrcCode.c_str(),
				CompilerOptions
			);
			
			SAT_CORE_INFO( "Shader Warings {0}", Result.GetNumWarnings() );
			SAT_CORE_INFO( "Shader Error status {0}", Result.GetCompilationStatus() );
			SAT_CORE_INFO( "Shader Error messages {0}", Result.GetErrorMessage() );

			std::string ResultString( Result.begin(), Result.end() );
			
			auto AssembleResult = Compiler.AssembleToSpv( ResultString.c_str(), 4 * ResultString.size() );

			if( AssembleResult.GetCompilationStatus() != shaderc_compilation_status_success )
			{
				assert( 0 ); // Shader compilation failed.
			}

			// Save code for later if we need it.
			std::vector<uint32_t> Code( AssembleResult.begin(), AssembleResult.end() );
			
			//m_ShaderCodes.insert( { std::string( pShader->m_ShaderCodenames[] ) ), Code } );
			
			m_ShaderCodes.insert( { std::string( pShader->m_Name + "/" + ShaderTypeToString( src.Type ) + "/" + std::to_string( src.Index ) ), Code } );

			SAT_CORE_INFO("===== SHADER OUTPUT: =====\n{0}\n", ResultString.c_str() );
			SAT_CORE_INFO( "==========================\n \n" );
		}
	}

	//////////////////////////////////////////////////////////////////////////

	Shader::Shader( std::string Name, std::filesystem::path Filepath )
	{
		m_Filepath = std::move( Filepath );
		m_Name = std::move( Name );

		ReadFile();
		DetermineShaderTypes();
	}

	Shader::~Shader()
	{

	}

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

	void Shader::DetermineShaderTypes()
	{
		int VertexShaders = -1;
		int FragmentShaders = -1;
		int ComputeShaders = -1;

		const char* TypeToken = "#type";
		size_t TypeTokenLength = strlen( TypeToken );
		size_t TypeTokenPosition = m_FileContents.find( TypeToken, 0 );
		
		while ( TypeTokenPosition != std::string::npos )
		{
			std::string FileCopy;
			
			std::copy( m_FileContents.begin(), m_FileContents.end(), std::back_inserter( FileCopy ) );
			
			size_t TypeTokenEnd = FileCopy.find( "\r\n", TypeTokenPosition );

			size_t Begin = TypeTokenPosition + TypeTokenLength + 1;

			std::string Type = FileCopy.substr( Begin, TypeTokenEnd - Begin );
			
			size_t NextLinePos = FileCopy.find_first_not_of( "\r\n", TypeTokenEnd );
			TypeTokenPosition = FileCopy.find( TypeToken, NextLinePos );
			
			auto RawShaderCode = FileCopy.substr( NextLinePos, TypeTokenPosition - ( NextLinePos == std::string::npos ? FileCopy.size() - 1 : NextLinePos ) );

			auto Shader_Type = ShaderTypeFromString( Type );

			if( Shader_Type == ShaderType::Fragment )
				FragmentShaders++;
			else if( Shader_Type == ShaderType::Vertex )
				VertexShaders++;
			else if( Shader_Type == ShaderType::Compute )
				ComputeShaders++;
			
			int Index = Shader_Type == ShaderType::Vertex ? VertexShaders : ( Shader_Type == ShaderType::Fragment ? FragmentShaders : ComputeShaders );

			ShaderSource src = ShaderSource( RawShaderCode, Shader_Type, Index );
			m_ShaderSources[ ShaderSourceKey( Shader_Type, Index ) ] = src;
			
			m_ShaderCodenames.push_back( { 
				.m_UUID = UUID(), 
				.m_Filename = m_Filepath.filename().string(), 
				.m_Type = std::move( ShaderTypeToString( Shader_Type ) )
			} );
		}
	}

	ShaderType ShaderTypeFromString( std::string Str )
	{
		if( Str == "vertex" )
		{
			return ShaderType::Vertex;
		}
		else if( Str == "fragment" )
		{
			return ShaderType::Fragment;
		}
		else if( Str == "compute" )
		{
			return ShaderType::Compute;
		}
		else if( Str == "geometry" )
		{
			return ShaderType::Geometry;
		}
		else
		{
			return ShaderType::Vertex;
		}
	}

	std::string ShaderTypeToString( ShaderType Type )
	{
		switch( Type )
		{
			case Saturn::ShaderType::Vertex: 
				return "Vertex";
				break;
			case Saturn::ShaderType::Fragment:
				return "Fragment";
				break;
			case Saturn::ShaderType::Geometry:
				return "Geometry";
				break;
			case Saturn::ShaderType::Compute:
				return "Compute";
				break;
			default:
				break;
		}
	}

}