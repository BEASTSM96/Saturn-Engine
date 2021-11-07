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

#include "ShaderDataType.h"

#include <string>

namespace Saturn {

	// This class is so that Materials know what uniforms to include when making the Material. We don't want a uniform thats not connected to the material an example of this is a LightPosition uniform as there is no need for that in a material.
	class MaterialUniform
	{
	public:
		MaterialUniform() {}

		MaterialUniform( const std::string& name, Ref<Texture2D>& texture, const std::string& textureName, const std::string& textureNameInShader ) 
		{
			m_Name = name;

			m_Data = texture;

			m_TextureName = textureName;
			m_ValueName = textureNameInShader;
		}

		~MaterialUniform()
		{
			m_Data.Delete();

			m_Name = m_TextureName = m_ValueName = "";
		}

		Ref<Texture2D> Data() { return m_Data; }
		const Ref<Texture2D> Data() const { return m_Data; }

		void SetData( Ref<Texture2D>& data ) 
		{
			m_Data = data;
		}

	private:

		bool IsStruct = false;
		std::string m_Name = "Unknown Uniform";

		// EXAMPLE - uniform sampler2D u_sampler2D. where value name is "u_sampler2D" and valueType is "sampler2D"

		std::string m_TextureName = "";
		std::string m_ValueName = "";

		Ref<Texture2D> m_Data;

		ShaderDataType m_Type = ShaderDataType::None;
	};
}