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

	class MaterialSpec
	{
	public:
		MaterialSpec();

		MaterialSpec( 
			std::string Name, UUID ID, std::vector< ShaderUniform* > Uniforms );

		MaterialSpec( std::string Name, UUID ID );

		// Copy
		MaterialSpec( const MaterialSpec& other );
		// Move
		MaterialSpec( MaterialSpec&& other ) noexcept;

		~MaterialSpec();

		void Terminate();

		// Copy assignment
		MaterialSpec& operator=( const MaterialSpec& other );
		
		// Move assignment
		MaterialSpec& operator=( MaterialSpec&& other ) noexcept;

		bool operator==( MaterialSpec& rOther );

	public:

		std::string& GetName() { return m_Name; }
		const std::string& GetName() const { return m_Name; }

		UUID& GetID() { return m_ID; }

		std::vector< ShaderUniform* >& GetUniforms() { return m_Uniforms; }
		const std::vector< ShaderUniform* >& GetUniforms() const { return m_Uniforms; }
		
		std::unordered_map< std::string, ShaderUniform* >& GetTextures() { return m_Textures; }
		
		const std::unordered_map< std::string, ShaderUniform* > GetTextures() const { return m_Textures; }

	private:
		std::string m_Name = "";
		
		UUID m_ID = 0;

		std::vector< ShaderUniform* > m_Uniforms;
		std::unordered_map< std::string, ShaderUniform* > m_Textures;
	};

	class Material
	{
	public:
		 Material( Ref< Saturn::Shader> Shader, MaterialSpec* Spec );
		~Material();

		void Bind( Ref< Saturn::Shader > Shader );

		void Unbind();

		void SetAlbedo( const Texture2D& Albedo );
		void SetNormal( Ref<Texture2D> Normal );
		void SetMetallic( Ref<Texture2D> Metallic );
		void SetRoughness( Ref<Texture2D> Roughness );

		/*
		template< typename Ty >
		void Set( std::string Name, Ty* Value ) 
		{
			for ( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name ) 
				{
					rUniform->pValue = ( void* )Value;
				}
			}
		}
		*/
		
		template< typename Ty >
		void Set( const std::string& Name, const Ty& Value )
		{
			for( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name )
				{
					rUniform->pValue = ( void* )&Value;
				}
			}
		}

		void Set( const std::string& Name, const Ref<Texture>& Value )
		{
			for( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name )
				{
					rUniform->pValue = ( void* ) &Value;
				}
			}
		}

		void Set( std::string Name, bool Value )
		{
			for( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name )
				{
					rUniform->pValue = ( void* ) Value;
				}
			}
		}

		void Set( std::string Name, glm::vec3 Value )
		{
			for( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name )
				{
					rUniform->pValue = ( void* )&Value;
				}
			}
		}
		
		void Set( std::string Name, glm::vec4 Value )
		{
			for( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name )
				{
					rUniform->pValue = ( void* ) &Value;
				}
			}
		}

		void Set( std::string Name, glm::mat4 Value )
		{
			for( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name )
				{
					rUniform->pValue = ( void* ) &Value;
				}
			}
		}

		template< typename Ty >
		Ty& Get( const std::string& Name )
		{
			for( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name )
				{
					return *( Ty* ) rUniform->pValue;
				}
			}
		}

		template< typename Ty >
		Ref< Ty > GetResource( const std::string& Name )
		{
			for( auto& rUniform : m_Spec->GetTextures() )
			{
				if( rUniform->Name == Name )
				{
					return rUniform->pValue;
				}
			}
		}

		bool Get( std::string Name )
		{
			for( auto& rUniform : m_Spec->GetUniforms() )
			{
				if( rUniform->Name == Name )
				{
					return ( bool ) rUniform->pValue;
				}
			}
		}
		
		Ref< Saturn::Shader >& GetShader() { return m_Shader; }

	private:
		MaterialSpec* m_Spec;
		Ref< Saturn::Shader > m_Shader;
	};

}