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
#include "Saturn/Log.h"

#include <string>
#include <vector>

namespace Saturn {

	enum class ShaderDomain
	{
		None = 0, Vertex = 0, Pixel = 1
	};

	class ShaderUniformDeclaration
	{
	private:
		friend class Shader;
		friend class OpenGLShader;
		friend class ShaderStruct;
	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetOffset() const = 0;
		virtual ShaderDomain GetDomain() const = 0;
	protected:
		virtual void SetOffset( uint32_t offset ) = 0;
	};

	typedef std::vector<ShaderUniformDeclaration*> ShaderUniformList;

	class ShaderUniformBufferDeclaration : public RefCounted
	{
	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetRegister() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual const ShaderUniformList& GetUniformDeclarations() const = 0;

		virtual ShaderUniformDeclaration* FindUniform( const std::string& name ) = 0;
	};

	typedef std::vector<ShaderUniformBufferDeclaration*> ShaderUniformBufferList;

	class ShaderStruct
	{
	private:
		friend class Shader;
	private:
		std::string m_Name;
		std::vector<ShaderUniformDeclaration*> m_Fields;
		uint32_t m_Size;
		uint32_t m_Offset;
	public:
		ShaderStruct( const std::string& name )
			: m_Name( name ), m_Size( 0 ), m_Offset( 0 )
		{
		}

		void AddField( ShaderUniformDeclaration* field )
		{
			m_Size += field->GetSize();
			uint32_t offset = 0;
			if( m_Fields.size() )
			{
				ShaderUniformDeclaration* previous = m_Fields.back();
				offset = previous->GetOffset() + previous->GetSize();
			}
			field->SetOffset( offset );
			m_Fields.push_back( field );
		}

		inline void SetOffset( uint32_t offset ) { m_Offset = offset; }

		inline const std::string& GetName() const { return m_Name; }
		inline uint32_t GetSize() const { return m_Size; }
		inline uint32_t GetOffset() const { return m_Offset; }
		inline const std::vector<ShaderUniformDeclaration*>& GetFields() const { return m_Fields; }
	};

	typedef std::vector<ShaderStruct*> ShaderStructList;

	class ShaderResourceDeclaration
	{
	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetRegister() const = 0;
		virtual uint32_t GetCount() const = 0;
	};

	typedef std::vector<ShaderResourceDeclaration*> ShaderResourceList;
}