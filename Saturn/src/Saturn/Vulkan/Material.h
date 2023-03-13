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

#pragma once

#include "Texture.h"
#include "Shader.h"

#include <string>

namespace Saturn {

	class StaticMesh;
	class Submesh;
	class MaterialInstance;

	class Material : public CountedObj
	{
	public:
		 Material( const Ref< Saturn::Shader >& Shader, const std::string& MateralName );
		~Material();

		void Copy( Ref<Material>& rOther );

		void Bind( const Ref< StaticMesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader );
		
		void RN_Update();
		void RN_Clean();

		void SetResource( const std::string& Name, const Ref< Saturn::Texture2D >& Texture );

		template<typename Ty>
		void Set( const std::string& Name, const Ty& Value ) 
		{
			for ( auto& Uniform : m_Uniforms )
			{
				if ( Uniform.GetName() == Name )
				{
					if( Uniform.IsPushConstantData() )
					{
						m_PushConstantData.Write( ( uint8_t* ) &Value, Uniform.GetSize(), Uniform.GetOffset() );
					}
					else
					{
						Uniform.GetBuffer().Write( ( uint8_t* ) &Value, Uniform.GetSize(), Uniform.GetOffset() );
					}
					
					m_AnyValueChanged = true;

					break;
				}
			}
		}
		
		template<typename Ty>
		Ty& Get( const std::string& Name ) 
		{
			for ( auto& Uniform : m_Uniforms )
			{
				if ( Uniform.GetName() == Name )
				{
					if ( Uniform.IsPushConstantData() )
					{
						return m_PushConstantData.Read< Ty >( Uniform.GetOffset() );
					}
					else
					{
						return Uniform.GetBuffer().Read< Ty >( Uniform.GetOffset() );
					}
				}
			}
		}
		
		Ref< Texture2D > GetResource( const std::string& Name );

		Ref< DescriptorSet >& GetDescriptorSet(uint32_t index = 0) { return m_DescriptorSets[index]; }

		bool HasAnyValueChanged() { return m_AnyValueChanged; };

		void SetName( const std::string& rName ) { m_Name = rName; }

	public:

		Ref< Saturn::Shader >& GetShader() { return m_Shader; }
		
		std::string& GetName() { return m_Name; }
		const std::string& GetName() const { return m_Name; }

	private:

		std::unordered_map< std::string, Ref<Texture2D> >& GetTextures() { return m_Textures; }
		const std::unordered_map< std::string, Ref<Texture2D> >& GetTextures() const { return m_Textures; }

	private:
		std::string m_Name = "";
		Ref< Saturn::Shader > m_Shader;

		bool m_AnyValueChanged = false;

		bool m_Updated[ MAX_FRAMES_IN_FLIGHT ];

		Buffer m_PushConstantData;
		
		std::vector< ShaderUniform > m_Uniforms;
		std::unordered_map< std::string, Ref<Texture2D> > m_Textures;

		Ref<DescriptorSet> m_DescriptorSets[ MAX_FRAMES_IN_FLIGHT ];

	private:
		friend class MaterialInstance;
		friend class MaterialAsset;
	};

	class MaterialTable
	{
		using MaterialTableList = std::unordered_map< std::string, Ref<Material> >;

		SINGLETON( MaterialTable );
	public:
		MaterialTable() {}
		~MaterialTable();

		void InsertMaterial( const Ref<Material>& rMaterial );
		void RemoveMaterial( const std::string& rName );

		// Apply changes to all materials with the same name.
		void ApplyChanges( const std::string& rMaterialName, Ref<Material>& rSourceMaterial );

	private:
		MaterialTableList m_Materials;
	};
}