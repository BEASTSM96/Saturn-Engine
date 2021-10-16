/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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

#include "Shader.h"
#include "Texture.h"
#include "VertexBuffer.h"

namespace Saturn {

	// This class is so that Materials know what uniforms to include when making the Material. We don't want a uniform thats not connected to the material an example of this is a LightPosition uniform as there is no need for that in a material.
	class MaterialUniform
	{
	public:
		MaterialUniform() {}
		MaterialUniform( const std::string& name, ShaderDataType& type ) : m_Name( name ), m_Type( type ) {}

		~MaterialUniform() = default;

	private:
		bool IsStruct = false;
		std::string m_Name = "Unknown Uniform";
		ShaderDataType m_Type = ShaderDataType::None;
	};

	enum class MaterialFlag
	{
		None = BIT( 0 ),
		DepthTest = BIT( 1 ),
		Blend = BIT( 2 ),
		TwoSided = BIT( 3 )
	};

	class Material
	{
	public:
		Material( Ref<Shader> shader );
		~Material();

		void Bind();

		uint32_t GetFlags() const { return m_MaterialFlags; }
		void SetFlag( MaterialFlag flag ) { m_MaterialFlags |= ( uint32_t )flag; }

		template <typename T>
		void Set( const std::string& name, const T& value ) 
		{
			
		}

	private:

		void BindTextures();

		Ref<Shader> m_MaterialShader;

		uint32_t m_MaterialFlags;

		std::vector<Ref<MaterialUniform>> m_Uniforms;
		std::vector<Ref<Texture>> m_Textures;
	};

}