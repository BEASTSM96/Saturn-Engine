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
#include "Mesh.h"
#include "DescriptorSet.h"
#include "Renderer.h"
#include "Texture.h"

#include "VulkanContext.h"

// TODO: When we have an asset manager, this needs to be re-worked!

namespace Saturn {

	Material::Material( const Ref< Saturn::Shader >& Shader, const std::string& MateralName )
	{
		m_Shader = Shader;

		if( MateralName.empty() )
		{
			std::string NewName = Shader->GetName();
			NewName += " Unknown Material " + std::to_string( UUID() );

			m_Name = NewName;
		}
		else
			m_Name = MateralName;

		for ( auto&& texture : m_Shader->GetTextures() )
		{
			m_Textures[ texture.Name ] = nullptr;
		}

		for ( auto rUniform : m_Shader->GetUniforms())
		{
			m_Uniforms.push_back( { rUniform.GetName(), rUniform.GetLocation(), rUniform.GetType(), rUniform.GetSize(), rUniform.GetOffset(), rUniform.IsPushConstantData() } );
		}

		uint32_t Size = 0;
		
		for ( auto& rUniform : m_Uniforms )
		{
			if( rUniform.IsPushConstantData() ) 
			{
				Size += rUniform.GetSize();
			}
		}
		
		m_PushConstantData.Allocate( Size );
		m_PushConstantData.Zero_Memory();
	}

	Material::~Material()
	{
		for ( auto& uniform : m_Uniforms )
			uniform.Terminate();

		for( auto& [key, texture] : m_Textures )
		{
			if( !texture )
				continue;

			if( texture->IsRendererTexture() )
				continue;
			
			texture = nullptr;
		}
	}
	
	void Material::Bind( const Ref< Mesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader )
	{
		Ref< DescriptorSet > CurrentSet = rMesh->GetDescriptorSets().at( rSubmsh );
		
		for ( auto& [name, texture] : m_Textures )
		{
			VkDescriptorImageInfo ImageInfo = {};
			ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			ImageInfo.imageView = m_Textures[ name ]->GetImageView();
			ImageInfo.sampler = m_Textures[ name ]->GetSampler();

			Shader->WriteDescriptor( name, ImageInfo, CurrentSet->GetVulkanSet() );
		}
	}

	void Material::Unbind()
	{
	}

	void Material::SetResource( const std::string& Name, const Ref< Saturn::Texture2D >& Texture )
	{
		if( m_Textures[ Name ] )
			m_AnyValueChanged = true;

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