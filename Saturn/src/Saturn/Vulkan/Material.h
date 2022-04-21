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

	struct MaterialSpec
	{
	public:
		MaterialSpec() 
		{
			Name = "";
		}
		
		~MaterialSpec() 
		{
			Terminate();
		}			
		
		void Terminate() 
		{
			Albedo.Terminate();
			Normal.Terminate();
			Roughness.Terminate();
			Metallic.Terminate();
			
		}

		MaterialSpec( 
			std::string Name, 
			UUID ID, 
			ShaderUniform& Albedo,
			ShaderUniform& Normal,
			ShaderUniform& Metallic,
			ShaderUniform& Roughness )
		{
			this->Name = std::move( Name );
			this->ID = ID;
			
			// Copy shader uniforms.
			memcpy( &this->Albedo, &Albedo, sizeof( ShaderUniform ) );
			memcpy( &this->Normal, &Normal, sizeof( ShaderUniform ) );
			memcpy( &this->Metallic, &Metallic, sizeof( ShaderUniform ) );
			memcpy( &this->Roughness, &Roughness, sizeof( ShaderUniform ) );
		}
		
		// Copy
		MaterialSpec( const MaterialSpec& other )
		{
			Name = other.Name;
			ID = other.ID;
			
			// Copy shader uniforms.
			memcpy( &Albedo, &other.Albedo, sizeof( ShaderUniform ) );
			memcpy( &Normal, &other.Normal, sizeof( ShaderUniform ) );
			memcpy( &Metallic, &other.Metallic, sizeof( ShaderUniform ) );
			memcpy( &Roughness, &other.Roughness, sizeof( ShaderUniform ) );
		}

		// Move
		MaterialSpec( MaterialSpec&& other ) noexcept
		{
			Name = std::move( other.Name );
			ID = other.ID;
			
			// Copy shader uniforms.
			memcpy( &Albedo, &other.Albedo, sizeof( ShaderUniform ) );
			memcpy( &Normal, &other.Normal, sizeof( ShaderUniform ) );
			memcpy( &Metallic, &other.Metallic, sizeof( ShaderUniform ) );
			memcpy( &Roughness, &other.Roughness, sizeof( ShaderUniform ) );
		}
		
		// Copy assignment
		MaterialSpec& operator=( const MaterialSpec& other )
		{
			Name = other.Name;
			ID = other.ID;
			
			// Copy shader uniforms.
			memcpy( &Albedo, &other.Albedo, sizeof( ShaderUniform ) );
			memcpy( &Normal, &other.Normal, sizeof( ShaderUniform ) );
			memcpy( &Metallic, &other.Metallic, sizeof( ShaderUniform ) );
			memcpy( &Roughness, &other.Roughness, sizeof( ShaderUniform ) );
			
			return *this;
		}

		// Move assignment
		MaterialSpec& operator=( MaterialSpec&& other ) noexcept
		{
			Name = std::move( other.Name );
			ID = other.ID;
			
			// Copy shader uniforms.
			memcpy( &Albedo, &other.Albedo, sizeof( ShaderUniform ) );
			memcpy( &Normal, &other.Normal, sizeof( ShaderUniform ) );
			memcpy( &Metallic, &other.Metallic, sizeof( ShaderUniform ) );
			memcpy( &Roughness, &other.Roughness, sizeof( ShaderUniform ) );
			
			return *this;
		}
		
		bool operator==( MaterialSpec& rOther ) 
		{
			return ( ID == rOther.ID );
		}

	public:
		std::string Name = "";
		
		UUID ID = 0;

		ShaderUniform Albedo;
		ShaderUniform Normal;
		ShaderUniform Metallic;
		ShaderUniform Roughness;
	};

	class Material
	{
	public:
		 Material( const MaterialSpec& Spec );
		~Material();

		void Bind( Ref<Shader> Shader );

		void Unbind();

		void SetAlbedo( Ref<Texture> Albedo );
		void SetNormal( Ref<Texture> Normal );
		void SetMetallic( Ref<Texture> Metallic );
		void SetRoughness( Ref<Texture> Roughness );

	private:
		MaterialSpec m_Spec;
	};
}