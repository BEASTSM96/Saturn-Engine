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

#pragma once

#include "Saturn/Core/Base.h"

#include "MaterialUniform.h"

#include "Shader.h"
#include "Texture.h"
#include "VertexBuffer.h"

#if !defined( SAT_DONT_USE_GL ) 

namespace Saturn {

	enum class MaterialFlag
	{
		None = BIT( 0 ),
		DepthTest = BIT( 1 ),
		Blend = BIT( 2 ),
		TwoSided = BIT( 3 )
	};

	enum class MaterialTextureType
	{
		Albedo,
		Normal,
		Metalness,
		Roughness,
		Specular
	};

	class Material
	{
	public:
		Material( Ref<Shader> shader );
		~Material();

		void Bind();

		uint32_t Flags() const { return m_MaterialFlags; }
		bool IsFlagSet( MaterialFlag matf ) { return ( uint32_t )matf & m_MaterialFlags; }

		void SetFlag( MaterialFlag flag ) { m_MaterialFlags |= ( uint32_t )flag; }

		void Set( const std::string& name, Ref<Texture2D>& texture );
		void Set( const std::string& name, glm::vec3& array );

		void Add( const std::string& name, Ref<Texture2D>& tex, MaterialTextureType textureFormat );

		Ref<MaterialUniform>& Get( const std::string& name );
		void BindTextures();

		std::vector<Ref<MaterialUniform>>& Uniforms() { return m_Uniforms; }
		const std::vector<Ref<MaterialUniform>>& Uniforms() const { return m_Uniforms; }

		std::string& Name() { return m_Name; }
		const std::string& Name() const { return m_Name; }

		
		Ref<Shader>& GetShader() { return m_MaterialShader; }
		const Ref<Shader>& GetShader() const { return m_MaterialShader; }

	private:

		void SetPropChanged( const std::string& prop )
		{
			m_PropsChanged.push_back( prop );
		}

		std::string m_Name;

		Ref<Shader> m_MaterialShader;

		uint32_t m_MaterialFlags;

		std::vector<Ref<MaterialUniform>> m_Uniforms;
		std::vector<Ref<Texture>> m_Textures;
		std::vector<std::string> m_PropsChanged;
	};
}

#endif