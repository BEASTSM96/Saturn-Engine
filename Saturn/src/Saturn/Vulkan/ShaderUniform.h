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

#include <string>

#include "Texture.h"

#include "ShaderDataType.h"

#include "Saturn/Core/Memory/Buffer.h"

namespace Saturn {
	
	// A shader uniform represents a uniform variable in a shader.
	class ShaderUniform : public CountedObj
	{
	public:
		ShaderUniform() { m_Data = Buffer(); }
		
		~ShaderUniform()
		{
			Terminate();
		}

		ShaderUniform( const std::string& name, int location, ShaderDataType type, size_t size, uint32_t offset, bool isPushConstantData = false )
			: m_Name( name ), m_Location( location ), m_Type( type ), m_IsPushConstantData( isPushConstantData ), m_Size( (uint32_t)size ), m_Offset( offset )
		{
			m_Data.Allocate( size );
			m_Data.Zero_Memory();
		}

		void Terminate()
		{	
			m_Location = -1;
			m_Type = ShaderDataType::None;
		}

		const std::string& GetName() const { return m_Name; }
		int GetLocation() const { return m_Location; }
		ShaderDataType GetType() const { return m_Type; }
		bool IsPushConstantData() const { return m_IsPushConstantData; }

		Buffer& GetBuffer() { return m_Data; }
		const Buffer& GetBuffer() const { return m_Data; }

		uint32_t GetOffset() { return m_Offset; }
		size_t GetSize() { return m_Size; }

	private:
		std::string m_Name = "";
		int m_Location = -1;
		ShaderDataType m_Type = ShaderDataType::None;
		bool m_IsPushConstantData = false;

		uint32_t m_Offset = 0;
		uint32_t m_Size = 0;

		Buffer m_Data;
	};

}