/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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

#include "Platform/OpenGL/OpenGLShader.h"

#include "Saturn/Core/Base.h"

namespace Saturn {

	std::vector<Ref<Shader>> Shader::s_AllShaders;

	Ref<Shader> Shader::Create( const std::string& filepath )
	{
		Ref<Shader> result = nullptr;

		switch( RendererAPI::Current() )
		{
			case RendererAPIType::None: return nullptr;
			case RendererAPIType::OpenGL: result = Ref<OpenGLShader>::Create( filepath );
		}
		s_AllShaders.push_back( result );
		return result;
	}

	Ref<Shader> Shader::CreateFromString( const std::string& source )
	{
		Ref<Shader> result = nullptr;

		switch( RendererAPI::Current() )
		{
			case RendererAPIType::None: return nullptr;
			case RendererAPIType::OpenGL: result = OpenGLShader::CreateFromString( source );
		}
		s_AllShaders.push_back( result );
		return result;
	}

	ShaderLibrary::ShaderLibrary()
	{
	}

	ShaderLibrary::~ShaderLibrary()
	{
	}

	void ShaderLibrary::Add( const Saturn::Ref<Shader>& shader )
	{
		auto& name = shader->GetName();
		SAT_CORE_ASSERT( m_Shaders.find( name ) == m_Shaders.end() );
		m_Shaders[ name ] = shader;
	}

	void ShaderLibrary::Load( const std::string& path )
	{
		auto shader = Ref<Shader>( Shader::Create( path ) );
		auto& name = shader->GetName();
		SAT_CORE_ASSERT( m_Shaders.find( name ) == m_Shaders.end() );
		m_Shaders[ name ] = shader;
	}

	void ShaderLibrary::Load( const std::string& name, const std::string& path )
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) == m_Shaders.end() );
		m_Shaders[ name ] = Ref<Shader>( Shader::Create( path ) );
	}

	const Ref<Shader>& ShaderLibrary::Get( const std::string& name ) const
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) != m_Shaders.end() );
		return m_Shaders.at( name );
	}
}
