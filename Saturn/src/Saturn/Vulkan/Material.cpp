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

	Material::Material( const Ref< Saturn::Shader >& Shader, const std::string& MateralName )
	{
		m_Shader = Shader;
		m_Name = MateralName;

		for ( auto& rUniform : m_Shader->GetUniforms() )
		{
			if( rUniform.Type >= ShaderDataType::Sampler2D )
				continue;
			else
				m_Uniforms.push_back( rUniform );
		}
	}

	Material::~Material()
	{
		for ( auto& uniform : m_Uniforms )
		{
			uniform.Terminate();
		}
		
		for ( auto& [ key, texture ] : m_Textures )
		{
			texture->Terminate();
		}
	}

	void Material::Bind( Ref<Shader> Shader )
	{
	}

	void Material::Unbind()
	{
	}

	void Material::SetResource( const std::string& Name, const Ref< Saturn::Texture2D >& Texture )
	{
		m_Textures[ Name ] = Texture;
	}

	Ref< Texture2D > Material::GetResource( const std::string& Name )
	{
		if( m_Textures.size() > 0 )
			return m_Textures.at( Name );
		else
			return nullptr;
	}

}