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

	Material::Material( const MaterialSpec& Spec )
	{
		m_Spec = Spec;
	}

	Material::~Material()
	{
		m_Spec.Terminate();
	}

	void Material::Bind( Ref<Shader> Shader )
	{
		// Albedo Texture.
		VulkanContext::Get().CreateDescriptorSet( m_Spec.ID, static_cast< Texture* >( ( Texture* )m_Spec.Albedo.pValue ) );
	}

	void Material::Unbind()
	{

	}

	void Material::SetAlbedo( Ref<Texture> Albedo )
	{
		m_Spec.Albedo.Set< Texture >( *Albedo.Pointer() );
	}

	void Material::SetNormal( Ref<Texture> Normal )
	{
		m_Spec.Normal.Set< Texture >( *Normal.Pointer() );
	}

	void Material::SetMetallic( Ref<Texture> Metallic )
	{
		m_Spec.Metallic.Set< Texture >( *Metallic.Pointer() );
	}

	void Material::SetRoughness( Ref<Texture> Roughness )
	{
		m_Spec.Roughness.Set< Texture >( *Roughness.Pointer() );
	}

}