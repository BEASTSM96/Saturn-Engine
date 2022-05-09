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

		MaterialSpec( std::string Name, UUID ID, std::vector< ShaderUniform* > Uniforms );

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

	private:
		std::string m_Name = "";
		
		UUID m_ID = 0;

		std::vector< ShaderUniform* > m_Uniforms;
	};

	class Material
	{
	public:
		 Material( MaterialSpec* Spec );
		~Material();

		void Bind( Ref<Shader> Shader );

		void Unbind();

		void SetAlbedo( Ref<Texture2D> Albedo );
		void SetNormal( Ref<Texture2D> Normal );
		void SetMetallic( Ref<Texture2D> Metallic );
		void SetRoughness( Ref<Texture2D> Roughness );

		template< typename Ty >
		void Set( std::string Name, Ty* Value ) 
		{
			for ( auto& rUniform : m_Spec->m_Uniforms )
			{
				if( rUniform->Name == Name ) 
				{
					rUniform->pValue = ( void* )Value;
				}
			}
		}

	private:
		MaterialSpec* m_Spec;
	};

}