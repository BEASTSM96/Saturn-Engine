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

#include "ShaderReflector.h"

#include <cassert>

namespace Saturn {
	
	ShaderLibrary::ShaderLibrary()
	{
	}

	ShaderLibrary::~ShaderLibrary()
	{
		for ( auto&& [ key, shader ] : m_Shaders )
		{
			shader.Delete();
		}
	}

	void ShaderLibrary::Add( const Ref<Shader>& shader )
	{
		auto& rName = shader->GetName();
		
		SAT_CORE_ASSERT( m_Shaders.find( rName ) == m_Shaders.end() );

		m_Shaders[ rName ] = shader;
	}

	void ShaderLibrary::Load( const std::string& path )
	{
		auto shader = Ref<Shader>::Create( path );
	
		Add( shader );
	}

	void ShaderLibrary::Load( const std::string& name, const std::string& path )
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) == m_Shaders.end() );

		m_Shaders[ name ] = Ref<Shader>::Create( path );
	}

	const Ref<Shader>& ShaderLibrary::Find( const std::string& name ) const
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) != m_Shaders.end() );

		return m_Shaders.at( name );
	}

	//////////////////////////////////////////////////////////////////////////
	
	Shader::Shader( std::string Name, std::filesystem::path Filepath )
	{
		m_Filepath = std::move( Filepath );
		m_Name = std::move( Name );

		ReadFile();
		DetermineShaderTypes();
		
		CompileGlslToSpvAssembly();
				
		for ( auto&& [key, code] : m_SpvCode )
		{
			Reflect( code );
		}

		GetAvailableUniform();
	}

	Shader::Shader( std::filesystem::path Filepath )
	{
		m_Filepath = std::move( Filepath );
		m_Name = std::move( Filepath.filename().string() );

		ReadFile();
		DetermineShaderTypes();

		CompileGlslToSpvAssembly();

		for( auto&& [key, code] : m_SpvCode )
		{
			Reflect( code );
		}

		GetAvailableUniform();
	}

	Shader::Shader( std::string Filepath )
	{
		m_Filepath = std::move( Filepath );
		
		// Strip the extension from the filename
		auto name = m_Filepath.filename().string();
		auto pos = name.find_last_of( '.' );
		if ( pos != std::string::npos )
		{
			name = name.substr( 0, pos );
		}

		m_Name = std::move( name );

		ReadFile();
		DetermineShaderTypes();

		CompileGlslToSpvAssembly();

		for( auto&& [key, code] : m_SpvCode )
		{
			Reflect( code );
		}

		GetAvailableUniform();
	}

	Shader::~Shader()
	{

	}

	void Shader::ReadFile()
	{
		std::ifstream f( m_Filepath, std::ios::ate | std::ios::binary );

		m_FileSize = static_cast< size_t >( f.tellg() );
		std::vector<char> Buffer( m_FileSize );

		f.seekg( 0 );
		f.read( Buffer.data(), m_FileSize );

		f.close();

		m_FileContents = std::string( Buffer.begin(), Buffer.end() );
	}

	void Shader::GetAvailableUniform()
	{

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

	void Shader::Reflect( const std::vector<uint32_t>& rShaderData )
	{
		Timer t;

		SAT_CORE_INFO( "Shader Reflecting..." );
		
		ReflectOutput Output = ShaderReflector::Get().ReflectShader( this );

		SAT_CORE_INFO( "Reflecting took: {0}", t.ElapsedMilliseconds() );
		
		SAT_CORE_INFO( " {0} Descriptors", Output.Descriptors.size() );

		for( auto& rDescriptor : Output.Descriptors )
		{
			// I know the for loop has a boolean condition, but I am still going to do this.
			if( rDescriptor.Members.size() > 0 )
			{
				SAT_CORE_INFO( " {0} Is a uniform block.", rDescriptor.Name );

				for( auto& rMember : rDescriptor.Members )
				{
					SAT_CORE_INFO( "  {0}", rMember.Name );
					SAT_CORE_INFO( "   Offset {0}", rMember.Offset );
					SAT_CORE_INFO( "   Size {0}", rMember.Size );
					SAT_CORE_INFO( "   Type {0}", ShaderDataTypeToString( rMember.Type ) );

					m_AvailableUniforms.push_back( { rMember.Name, rDescriptor.Binding, rMember.Type } );
				}
			}
			else
			{
				SAT_CORE_INFO( " {0} Is a not uniform block.", rDescriptor.Name );

				m_AvailableUniforms.push_back( { rDescriptor.Name, rDescriptor.Binding, VulkanDescriptorToShaderDataType( rDescriptor.Type ) } );
			}
		}
		
	}

	void Shader::CompileGlslToSpvAssembly()
	{
		shaderc::Compiler       Compiler;
		shaderc::CompileOptions CompilerOptions;

		// We don use shaderc_optimization_level_zero, as will remove the uniform names, it took me 6 hours to figure out.
		CompilerOptions.SetOptimizationLevel( shaderc_optimization_level_zero );

		CompilerOptions.SetWarningsAsErrors();

		CompilerOptions.SetTargetEnvironment( shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2 );
		CompilerOptions.SetTargetSpirv( shaderc_spirv_version_1_5 );

		Timer CompileTimer;
		
		for ( auto&& [key, src] : m_ShaderSources )
		{
			const std::string& rShaderSrcCode = src.Source;

			const auto Res = Compiler.CompileGlslToSpv(
				rShaderSrcCode,
				src.Type == ShaderType::Vertex ? shaderc_shader_kind::shaderc_glsl_default_vertex_shader : shaderc_shader_kind::shaderc_glsl_default_fragment_shader,
				m_Filepath.string().c_str(),
				CompilerOptions );

			if( Res.GetCompilationStatus() != shaderc_compilation_status_success ) 
			{
				SAT_ASSERT( false, "Shader Compilation Failed" );
			}

			SAT_CORE_INFO( "Shader Warings {0}", Res.GetNumWarnings() );
			SAT_CORE_INFO( "Shader Error status {0}", Res.GetCompilationStatus() );
			SAT_CORE_INFO( "Shader Error messages {0}", Res.GetErrorMessage() );

			std::vector< uint32_t > SpvBinary( Res.begin(), Res.end() );

			m_SpvCode[ key ] = SpvBinary;
		}

		SAT_CORE_INFO( "Shader Compilation took {0} ms", CompileTimer.ElapsedMilliseconds() );
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