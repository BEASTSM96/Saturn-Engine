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
#include "Material.h"

#include "VulkanContext.h"

namespace Saturn {
	
	MaterialSpec::MaterialSpec()
	{
	}
	
	MaterialSpec::MaterialSpec( std::string Name, UUID ID, std::vector< ShaderUniform* > Uniforms )
		: m_Name( Name ), m_ID( ID )
	{
		for ( auto& Uniform : Uniforms )
		{
			if( Uniform->Type == ShaderDataType::Sampler2D )
			{
				m_Textures[ Uniform->Name ] = Uniform;
			}
			else
				m_Uniforms.push_back( Uniform );
		}
	}

	MaterialSpec::MaterialSpec( std::string Name, UUID ID ) : m_Name( Name ), m_ID( ID )
	{

	}
	
	MaterialSpec::MaterialSpec( const MaterialSpec& other ) : m_Name( other.m_Name ), m_ID( other.m_ID ), m_Uniforms( other.m_Uniforms )
	{
	}

	MaterialSpec::MaterialSpec( MaterialSpec&& other ) noexcept : m_Name( std::move( other.m_Name ) ), m_ID( std::move( other.m_ID ) ), m_Uniforms( std::move( other.m_Uniforms ) )
	{

	}

	MaterialSpec::~MaterialSpec()
	{
		Terminate();
	}

	void MaterialSpec::Terminate()
	{
		for ( auto& Uniform : m_Uniforms )
		{
			Uniform->Terminate();
			delete Uniform;
		}
	}


	bool MaterialSpec::operator==( MaterialSpec& rOther )
	{
		return ( m_Name == rOther.m_Name && m_ID == rOther.m_ID );
	}

	MaterialSpec& MaterialSpec::operator=( MaterialSpec&& other ) noexcept
	{
		m_ID = other.m_ID;
		m_Name = other.m_Name;
		m_Uniforms = other.m_Uniforms;

		return *this;
	}

	MaterialSpec& MaterialSpec::operator=( const MaterialSpec& other )
	{
		m_ID = other.m_ID;
		m_Name = other.m_Name;
		m_Uniforms = other.m_Uniforms;
		
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////

	Material::Material( Ref< Saturn::Shader> Shader, MaterialSpec* Spec )
	{
		m_Shader = Shader;
		m_Spec = Spec;
	}

	Material::~Material()
	{
		delete m_Spec;
	}

	void Material::Bind( Ref<Shader> Shader )
	{
		// Albedo Texture.
		//VulkanContext::Get().CreateDescriptorSet( m_Spec->ID, static_cast< Texture2D* >( ( Texture2D* )m_Spec->Albedo->pValue ) );
	}

	void Material::Unbind()
	{

	}

	void Material::SetAlbedo( const Texture2D& Albedo )
	{
		for( auto& Uniform : m_Spec->GetUniforms() )
		{
			if( Uniform->Name == "u_AlbedoTexture" )
			{
				//Uniform->pValue = ( void* )Albedo;
			}
		}
	}

	void Material::SetNormal( Ref<Texture2D> Normal )
	{
		for( auto& Uniform : m_Spec->GetUniforms() )
		{
			if( Uniform->Name == "u_NormalTexture" )
			{
				Uniform->pValue = ( void* ) Normal.Pointer();
			}
		}
	}

	void Material::SetMetallic( Ref<Texture2D> Metallic )
	{
		for( auto& Uniform : m_Spec->GetUniforms() )
		{
			if( Uniform->Name == "u_MetallicTexture" )
			{
				Uniform->pValue = ( void* ) Metallic.Pointer();
			}
		}
	}

	void Material::SetRoughness( Ref<Texture2D> Roughness )
	{
		for( auto& Uniform : m_Spec->GetUniforms() )
		{
			if( Uniform->Name == "u_RoughnessTexture" )
			{
				Uniform->pValue = ( void* ) Roughness.Pointer();
			}
		}
	}
}