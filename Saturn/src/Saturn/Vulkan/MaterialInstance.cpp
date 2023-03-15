/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "MaterialInstance.h"

#include "Mesh.h"
#include "Renderer.h"
#include "DescriptorSet.h"

// TODO: Come up with a proper way of handling uniforms and textures so we don't have to copy them.
// TODO: When we have an asset manager, this needs to be re-worked!
// SELF: Please create a material instance viewer!

namespace Saturn {

	MaterialInstance::MaterialInstance( const Ref< Material >& rMaterial, const std::string& rName )
		: m_Material( rMaterial ), m_Name( rName )
	{
		
		for( auto&& [ name, texture ] : m_Material->m_Textures )
		{
			m_Textures[ name ] = nullptr;
		}

		for( auto rUniform : m_Material->m_Uniforms )
		{
			m_Uniforms.push_back( { rUniform.GetName(), rUniform.GetLocation(), rUniform.GetType(), rUniform.GetSize(), rUniform.GetOffset(), rUniform.IsPushConstantData() } );
		}
		
		uint32_t Size = 0;

		for( auto& rUniform : m_Uniforms )
		{
			if( rUniform.IsPushConstantData() )
			{
				Size += rUniform.GetSize();
			}
		}

		m_PushConstantData.Allocate( Size );
		m_PushConstantData.Zero_Memory();
	}

	MaterialInstance::~MaterialInstance()
	{
		for( auto& uniform : m_Uniforms )
			uniform.Terminate();

		for( auto& [key, texture] : m_Textures )
		{
			if( !texture )
				continue;

			if( texture->IsRendererTexture() )
				continue;

			texture = nullptr;
		}

		m_Material = nullptr;
	}

	void MaterialInstance::Bind( const Ref< StaticMesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader )
	{
		Ref< DescriptorSet > CurrentSet = nullptr;

		for( auto& [name, texture] : m_Textures )
		{
			// Check if the texture even exists in the cache.
			if( m_TextureCache.find( name ) == m_TextureCache.end() )
			{
				m_TextureCache[ name ] = texture->GetDescriptorInfo();
			}
			else
			{
				VkDescriptorImageInfo ImageInfo = m_TextureCache.at( name );

				if( m_TextureCache.at( name ).imageView == ImageInfo.imageView || m_TextureCache.at( name ).sampler == ImageInfo.sampler )
				{
					// No need to update the descriptor set -- its the same.
					continue;
				}
				else // If the image view has changed, update the cache.
				{
					m_TextureCache[ name ] = texture->GetDescriptorInfo();
					Shader->WriteDescriptor( name, ImageInfo, CurrentSet->GetVulkanSet() );
					
					continue;
				}
			}

			// Fallback, just write the descriptor.
			VkDescriptorImageInfo ImageInfo = {};
			ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			if( m_Textures[ name ] )
			{
				ImageInfo.imageView = m_Textures[ name ]->GetImageView();
				ImageInfo.sampler = m_Textures[ name ]->GetSampler();
			}
			else
			{
				auto PinkTexture = Renderer::Get().GetPinkTexture();

				ImageInfo.imageView = PinkTexture->GetImageView();
				ImageInfo.sampler = PinkTexture->GetSampler();
			}

			Shader->WriteDescriptor( name, ImageInfo, CurrentSet->GetVulkanSet() );
		}
	}

	void MaterialInstance::SetResource( const std::string& Name, const Ref< Saturn::Texture2D >& Texture )
	{
		m_Textures[ Name ] = Texture;
	}

	Saturn::Ref< Saturn::Texture2D > MaterialInstance::GetResource( const std::string& Name )
	{
		return m_Textures[ Name ];
	}

}
