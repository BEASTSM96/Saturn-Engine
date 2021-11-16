/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2021 BEAST                                                           		*
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
#include "Material.h"

namespace Saturn {

	Material::Material( Ref<Shader> shader ) : m_MaterialShader( shader )
	{
		SetFlag( MaterialFlag::DepthTest );
	}

	Material::~Material()
	{
		m_MaterialShader.Delete();
	}

	void Material::Bind()
	{
		BindTextures();

		m_MaterialShader->Bind();
	}

	void Material::Set( const std::string& name, Ref<Texture2D>& texture )
	{
		//SAT_CORE_ASSERT( m_Uniforms.find( name ) != m_Uniforms.end(), "Key not found!" );

		for( const auto& uni : m_Uniforms )
		{
			if( uni->m_Name == name )
				return;

			if( m_Uniforms[ uni ] )
				m_Uniforms[ uni ]->SetData( texture );
		}

		// Send it off to a list so when we are about the render we can tell the shader to one bind and two change the requested values.
		SetPropChanged( name );
	}

	void Material::Set( const std::string& name, glm::vec3& array )
	{
		//SAT_CORE_ASSERT( m_Uniforms.find( name ) != m_Uniforms.end(), "Key not found!" );

		for( const auto& uni : m_Uniforms )
		{
			if( uni->m_Name == name )
				return;

			if( m_Uniforms[ uni ] && m_Uniforms[ uni ]->IsTexture() )
				m_Uniforms[ uni ]->SetValue<glm::vec3>( array );
		}

		// Send it off to a list so when we are about the render we can tell the shader to one bind and two change the requested values.
		SetPropChanged( name );
	}

	void Material::Add( const std::string& name, Ref<Texture2D>& tex, MaterialTextureType textureFormat )
	{
		//SAT_CORE_ASSERT( m_Uniforms.find( name ) == m_Uniforms.end(), "Key was already found!" );

		switch( textureFormat )
		{
			case MaterialTextureType::Albedo:
			{
				Ref<MaterialUniform> uni = Ref<MaterialUniform>::Create( name, tex, tex->Filename(), "u_AlbedoTexture" );
				m_Uniforms.push_back( uni );
			} break;

			case MaterialTextureType::Normal:
			{
				Ref<MaterialUniform> uni = Ref<MaterialUniform>::Create( name, tex, tex->Filename(), "u_NormalTexture" );
				m_Uniforms.push_back( uni );
			} break;

			case MaterialTextureType::Metalness:
			{
				Ref<MaterialUniform> uni = Ref<MaterialUniform>::Create( name, tex, tex->Filename(), "u_MetalnessTexture" );
				m_Uniforms.push_back( uni );
			} break;

			case MaterialTextureType::Roughness:
			{
				Ref<MaterialUniform> uni = Ref<MaterialUniform>::Create( name, tex, tex->Filename(), "u_RoughnessTexture" );
				m_Uniforms.push_back( uni );
			} break;

			case MaterialTextureType::Specular:
			{
				Ref<MaterialUniform> uni = Ref<MaterialUniform>::Create( name, tex, tex->Filename(), "u_SpecularTexture" );
				m_Uniforms.push_back( uni );
			} break;

			default:
				break;
		}
	}

	Ref<MaterialUniform>& Material::Get( const std::string& name )
	{
		//SAT_CORE_ASSERT( m_Uniforms.find( name ) != m_Uniforms.end(), "Key not found!" );

		for( auto& uni : m_Uniforms )
		{
			if( uni->m_Name == name )
				return uni;
		}
	}

	void Material::BindTextures()
	{
		for( int i = 0; i < m_Uniforms.size(); i++ )
		{
			const auto& v = m_Uniforms[ i ];

			if( v->Data() )
				v->Data()->Bind( i );
		}
	}

}