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

#include "sppch.h"
#include "Shader.h"

#include <glad/glad.h>

#include <string>
#include <sstream>
#include <limits>

#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	Shader::Shader( const std::string& filename ) : m_Filepath( filename )
	{
		size_t found = filename.find_last_of( "/\\" );
		m_Name = found != std::string::npos ? filename.substr( found + 1 ) : filename;
		found = m_Name.find_last_of( "." );
		m_Name = found != std::string::npos ? m_Name.substr( 0, found ) : m_Name;

		Load( filename );
	}

	void Shader::Bind()
	{
		glUseProgram( m_ID );
	}

	void Shader::Load( const std::string& filepath )
	{
		std::string src = ReadShaderFromFile( filepath );

		m_Shaders = DetermineShaderTypes( src );
		Parse();

		if( m_ID )
			glDeleteProgram( m_ID );

		CompileAndUploadShader();

		m_Loaded = true;
	}

	void Shader::Parse()
	{
		const char* token;
		
		// Vertex and Fragment strings
		const char* vstr;
		const char* fstr;

		auto& vertexSource = m_Shaders[ GL_VERTEX_SHADER ];
		auto& fragmentSource = m_Shaders[ GL_FRAGMENT_SHADER ];

		// Vertex Shader
		vstr = vertexSource.c_str();
		// Fragment Shader
		fstr = fragmentSource.c_str();

	}

	void Shader::CompileAndUploadShader()
	{
		std::vector<GLuint> shaderRendererIDs;

		GLuint program = glCreateProgram();
		// Compile all shaders in file
		for( auto& kv : m_Shaders )
		{
			GLenum type = kv.first;
			std::string& source = kv.second;

			GLuint shaderRendererID = glCreateShader( type );

			const GLchar* sourceCstr = ( const GLchar* )source.c_str();
			glShaderSource( shaderRendererID, 1, &sourceCstr, 0 );

			glCompileShader( shaderRendererID );

			GLint isCompiled = 0;
			glGetShaderiv( shaderRendererID, GL_COMPILE_STATUS, &isCompiled );
			if( isCompiled == GL_FALSE )
			{
				GLint maxLength = 0;
				glGetShaderiv( shaderRendererID, GL_INFO_LOG_LENGTH, &maxLength );

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog( maxLength );
				glGetShaderInfoLog( shaderRendererID, maxLength, &maxLength, &infoLog[ 0 ] );

				SAT_CORE_ERROR( "Shader compilation failed ({0}):\n{1}", m_Filepath, &infoLog[ 0 ] );

				// We don't need the shader anymore.
				glDeleteShader( shaderRendererID );

				SAT_CORE_ASSERT( false, "Failed" );
			}

			shaderRendererIDs.push_back( shaderRendererID );
			glAttachShader( program, shaderRendererID );
		}

		// Link our program
		glLinkProgram( program );

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv( program, GL_LINK_STATUS, ( int* )&isLinked );
		if( isLinked == GL_FALSE )
		{
			GLint maxLength = 0;
			glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog( maxLength );
			glGetProgramInfoLog( program, maxLength, &maxLength, &infoLog[ 0 ] );
			SAT_CORE_ERROR( "Shader linking failed ({0}):\n{1}", m_Filepath, &infoLog[ 0 ] );

			// We don't need the program anymore.
			glDeleteProgram( program );
			// Don't leak shaders either.
			for( auto id : shaderRendererIDs )
				glDeleteShader( id );
		}

		// Always detach shaders after a successful link.
		for( auto id : shaderRendererIDs )
			glDetachShader( program, id );

		m_ID = program;
	}

	std::string Shader::ReadShaderFromFile( const std::string& src )
	{
		// C++11 Like

		std::string result;
		std::ifstream in( src, std::ios::in | std::ios::binary );
		if( in )
		{
			in.seekg( 0, std::ios::end );
			result.resize( in.tellg() );
			in.seekg( 0, std::ios::beg );
			in.read( &result[ 0 ], result.size() );
		}
		else
		{
			SAT_CORE_ASSERT( false, "Could not load shader!" );
		}
		in.close();
		return result;
	}

	const char* Shader::FindToken( const char* shader, const std::string& token )
	{
		const char* t = shader;

		// Return ptr of the first occurrence of token in t & check if not null
		while ( t = strstr( t, token.c_str() ) )
		{
			bool left = shader == t || isspace( t[ -1 ] );

			bool right = !t[ token.size() ] || isspace( t[ token.size() ] );

			if( left && right )
				return t;

			t += token.size();
		}	
		return nullptr;
	}

	std::string Shader::GetStatement( const char* str, const char** outPosition )
	{
		return "";
	}

	void Shader::ParseUniform( const std::string& statement, int domain )
	{

	}

	GLenum Shader::ShaderTypeFromString( const std::string& type )
	{
		if( type == "vertex" )
			return GL_VERTEX_SHADER;
		if( type == "fragment" )
			return GL_FRAGMENT_SHADER;

		return GL_NONE;
	}

	std::unordered_map<GLenum, std::string> Shader::DetermineShaderTypes( const std::string& filepath )
	{
		// We technically have 2 shaders in one file, so we make a map here
		std::unordered_map<GLenum, std::string> shaderSources;

		// Find #type
		const char* typeToken = "#type";
		size_t tt_length = strlen( typeToken );
		size_t tt_pos = filepath.find( typeToken, 0 );

		while ( tt_pos != std::string::npos )
		{
			// Fine first eol in Type Token pos
			size_t eol_CL_LF = filepath.find_first_of( __CR_LF__, tt_pos );
			SAT_CORE_ASSERT( eol_CL_LF != std::string::npos, "[Shader] Syntax Error!" );

			// Get shader type

			size_t begin = tt_pos + tt_length + 1; // Get next string AKA #type vertex

			std::string type = filepath.substr( begin, eol_CL_LF - begin );

			// TODO: Pixel and compute shaders

			SAT_CORE_ASSERT( type == "vertex" || type == "fragment", "Invalid shader type specified" );

			size_t nextLinePos = filepath.find_first_not_of( "\r\n", eol_CL_LF );
			tt_pos = filepath.find( typeToken, nextLinePos );
			auto shaderType = ShaderTypeFromString( type );
			shaderSources[ shaderType ] = filepath.substr( nextLinePos, tt_pos - ( nextLinePos == std::string::npos ? filepath.size() - 1 : nextLinePos ) );
		}

		return shaderSources;
	}

	void Shader::SetBool( const std::string& name, bool val )
	{
		glUniform1i( glGetUniformLocation( m_ID, name.c_str() ), ( int )val );
	}

	void Shader::SetInt( const std::string& name, int val )
	{
		glUniform1i( glGetUniformLocation( m_ID, name.c_str() ), val );
	}

	void Shader::SetFloat( const std::string& name, float val )
	{
		glUniform1f( glGetUniformLocation( m_ID, name.c_str() ), val );
	}
}
