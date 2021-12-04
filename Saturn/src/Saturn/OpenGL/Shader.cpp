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

#include "Renderer.h"

#include "MaterialUniform.h"

#include "xGL.h"

#include <string>
#include <sstream>
#include <limits>

#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	const char* Shader::FindToken( const char* shader, const std::string& token )
	{
		const char* t = shader;

		// Return ptr of the first occurrence of token in t & check if not null
		while( t = strstr( t, token.c_str() ) )
		{
			bool left = shader == t || isspace( t[ -1 ] );

			bool right = !t[ token.size() ] || isspace( t[ token.size() ] );

			if( left && right )
				return t;

			t += token.size();
		}
		return nullptr;
	}

	const char* FindToken( const std::string& string, const std::string& token )
	{
		return FindToken( string.c_str(), token );
	}

	std::vector<std::string> SplitString( const std::string& string, const std::string& delimiters )
	{
		size_t start = 0;
		size_t end = string.find_first_of( delimiters );

		std::vector<std::string> result;

		while( end <= std::string::npos )
		{
			std::string token = string.substr( start, end - start );
			if( !token.empty() )
				result.push_back( token );

			if( end == std::string::npos )
				break;

			start = end + 1;
			end = string.find_first_of( delimiters, start );
		}

		return result;
	}

	std::vector<std::string> SplitString( const std::string& string, const char delimiter )
	{
		return SplitString( string, std::string( 1, delimiter ) );
	}

	std::vector<std::string> Tokenise( const std::string& string )
	{
		return SplitString( string, " \t\n\r" );
	}

	std::vector<std::string> GetLines( const std::string& string )
	{
		return SplitString( string, "\n" );
	}

	std::string GetBlock( const char* str, const char** outPosition )
	{
		const char* end = strstr( str, "}" );
		if( !end )
			return str;

		if( outPosition )
			*outPosition = end;
		uint32_t length = end - str + 1;
		return std::string( str, length );
	}

	bool StartsWith( const std::string& string, const std::string& start )
	{
		return string.find( start ) == 0;
	}

	static bool IsTypeStringResource( const std::string& type )
	{
		if( type == "sampler1D" )		return true;
		if( type == "sampler2D" )		return true;
		if( type == "sampler2DMS" )		return true;
		if( type == "samplerCube" )		return true;
		if( type == "sampler2DShadow" )	return true;
		return false;
	}

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
		SAT_CORE_INFO( "Loading shader at {0}...", filepath.c_str() );

		std::string src = ReadShaderFromFile( filepath );

		m_Shaders = DetermineShaderTypes( src );

		if( !m_IsCompute )
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

		while( token = FindToken( vstr, "uniform" ) )
			ParseUniform( GetStatement( token, &vstr ), ShaderDomain::Vertex );

		while( token = FindToken( fstr, "uniform" ) )
			ParseUniform( GetStatement( token, &fstr ), ShaderDomain::Pixel );
	}

	std::string Shader::GetStatement( const char* str, const char** outPosition )
	{
		const char* end = strstr( str, ";" );
		if( !end )
			return str;

		if( outPosition )
			*outPosition = end;
		uint32_t length = end - str + 1;
		return std::string( str, length );
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

	void Shader::BindMaterialTextures()
	{
		SetInt( "u_AlbedoTexture", 0 );
		SetInt( "u_SpecularTexture", 1 );
		SetInt( "u_NormalTexture", 2 );
	}

	void Shader::ParseUniform( const std::string& statement, ShaderDomain domain )
	{
		std::vector<std::string> tokens = Tokenise( statement );
		uint32_t index = 0;

		index++; // "uniform"
		std::string typeString = tokens[ index++ ];
		std::string name = tokens[ index++ ];
		// Strip ; from name if present
		if( const char* s = strstr( name.c_str(), ";" ) )
			name = std::string( name.c_str(), s - name.c_str() );

		std::string n( name );
		int32_t count = 1;
		const char* namestr = n.c_str();
		if( const char* s = strstr( namestr, "[" ) )
		{
			name = std::string( namestr, s - namestr );
			const char* end = strstr( namestr, "]" );
			std::string c( s + 1, end - s );
			count = atoi( c.c_str() );
		}

		if( IsTypeStringResource( typeString ) ) 
		{
		}
	}

	GLenum Shader::ShaderTypeFromString( const std::string& type )
	{
		if( type == "vertex" )
			return GL_VERTEX_SHADER;
		if( type == "fragment" || type == "pixel" )
			return GL_FRAGMENT_SHADER;
		if( type == "compute" )
			return GL_COMPUTE_SHADER;

		return GL_NONE;
	}

	std::unordered_map<GLenum, std::string> Shader::DetermineShaderTypes( const std::string& filepath )
	{
		// We technically have 2 or more shaders in one file, so we make a map here
		std::unordered_map<GLenum, std::string> shaderSources;

		// Find #type
		const char* typeToken = "#type";
		size_t tt_length = strlen( typeToken );
		size_t tt_pos = filepath.find( typeToken, 0 );

		while( tt_pos != std::string::npos )
		{
			// Fine first eol in Type Token pos
			size_t eol_CL_LF = filepath.find_first_of( __CR_LF__, tt_pos );
			SAT_CORE_ASSERT( eol_CL_LF != std::string::npos, "[Shader] Syntax Error!" );

			// Get shader type

			size_t begin = tt_pos + tt_length + 1; // Get next string AKA #type vertex

			std::string type = filepath.substr( begin, eol_CL_LF - begin );

			SAT_CORE_ASSERT( type == "vertex" || type == "fragment" || type == "pixel" || type == "compute", "Invalid shader type specified" );

			size_t nextLinePos = filepath.find_first_not_of( "\r\n", eol_CL_LF );
			tt_pos = filepath.find( typeToken, nextLinePos );
			auto shaderType = ShaderTypeFromString( type );
			shaderSources[ shaderType ] = filepath.substr( nextLinePos, tt_pos - ( nextLinePos == std::string::npos ? filepath.size() - 1 : nextLinePos ) );

			// Compute shaders cannot contain other types
			if( shaderType == GL_COMPUTE_SHADER )
			{
				m_IsCompute = true;
				break;
			}
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

	void Shader::SetIntArray( const std::string& name, int32_t* vals, uint32_t count )
	{
		glUniform1iv( glGetUniformLocation( m_ID, name.c_str() ), count, vals );
	}

	void Shader::SetFloat( const std::string& name, float val )
	{
		glUniform1f( glGetUniformLocation( m_ID, name.c_str() ), val );
	}

	void Shader::SetFloat2( const std::string& name, const glm::vec2& val )
	{
		glUniform2f( glGetUniformLocation( m_ID, name.c_str() ), val.x, val.y );
	}

	void Shader::SetFloat3( const std::string& name, const glm::vec3& val )
	{
		glUniform3f( glGetUniformLocation( m_ID, name.c_str() ), val.x, val.y, val.z );
	}

	void Shader::SetFloat4( const std::string& name, const glm::vec4& val )
	{
		glUniform4f( glGetUniformLocation( m_ID, name.c_str() ), val.x, val.y, val.z, val.w );
	}

	void Shader::SetMat4( const std::string& name, const glm::mat4& val )
	{
		glUniformMatrix4fv( glGetUniformLocation( m_ID, name.c_str() ), 1, GL_FALSE, ( const float* )&val );
	}

}
