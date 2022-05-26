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

#include "Texture.h"
#include "Shader.h"

#include <string>

namespace Saturn {

	class Mesh;
	class Submesh;

	class Material
	{
	public:
		 Material( const Ref< Saturn::Shader >& Shader, const std::string& MateralName );
		~Material();

		void Bind( const Ref< Mesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader );

		void Unbind();
		
		void SetResource( const std::string& Name, const Ref< Saturn::Texture2D >& Texture );

		template<typename Ty>
		void Set( const std::string& Name, const Ty& Value ) 
		{

			for ( auto& Uniform : m_Uniforms )
			{
				if ( Uniform.Name == Name )
				{
					Uniform.Set( ( uint8_t* )&Value, sizeof( Ty ) );
					return;
				}
			}
		}
		
		template<typename Ty>
		Ty& Get( const std::string& Name ) 
		{
			for ( auto& Uniform : m_Uniforms )
			{
				if ( Uniform.Name == Name )
				{
					return Uniform.Read< Ty >();
				}
			}
		}
		
		Ref< Texture2D > GetResource( const std::string& Name );

	public:

		Ref< Saturn::Shader >& GetShader() { return m_Shader; }
		
	private:
		std::string m_Name = "";
		Ref< Saturn::Shader > m_Shader;

		std::vector< ShaderUniform > m_Uniforms;
		std::unordered_map< std::string, Ref<Texture2D> > m_Textures;
	};

	class MaterialInstance
	{
	public:
		MaterialInstance( const Ref< Material >& rMaterial, const std::string& rName ) {}
		~MaterialInstance() {}
		
		template<typename Ty>
		void Set( const std::string& Name, const Ty& Value )
		{

			for( auto& Uniform : m_Uniforms )
			{
				if( Uniform.Name == Name )
				{
					Uniform.Set( ( uint8_t* ) &Value, sizeof( Ty ) );
					return;
				}
			}
		}

		template<typename Ty>
		Ty& Get( const std::string& Name )
		{
			for( auto& Uniform : m_Uniforms )
			{
				if( Uniform.Name == Name )
				{
					return Uniform.Read< Ty >();
				}
			}
		}

		void SetResource( const std::string& Name, const Ref< Saturn::Texture2D >& Texture ) {}
		Ref< Texture2D > GetResource( const std::string& Name ) {}

	private:
		
		std::string m_Name = "";
		Ref< Material > m_Material;

		std::vector< ShaderUniform > m_Uniforms;
		std::unordered_map< std::string, Ref<Texture2D> > m_Textures;

	private:
		friend class Material;
	};
}