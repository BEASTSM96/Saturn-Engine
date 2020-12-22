/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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

#include "RendererAPI.h"
#include "Saturn/Log.h"
#include "Saturn/Core/Base.h"
#include "Saturn/Core/Ref.h"

#include <vector>
#include <string>

namespace Saturn {

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	//TODO: maybe move this into a new file?
	static uint32_t ShaderDataTypeSize( ShaderDataType type )
	{
		switch( type )
		{
			case ShaderDataType::Float:    return 4;
			case ShaderDataType::Float2:   return 4 * 2;
			case ShaderDataType::Float3:   return 4 * 3;
			case ShaderDataType::Float4:   return 4 * 4;
			case ShaderDataType::Mat3:     return 4 * 3 * 3;
			case ShaderDataType::Mat4:     return 4 * 4 * 4;
			case ShaderDataType::Int:      return 4;
			case ShaderDataType::Int2:     return 4 * 2;
			case ShaderDataType::Int3:     return 4 * 3;
			case ShaderDataType::Int4:     return 4 * 4;
			case ShaderDataType::Bool:     return 1;
		}

		SAT_CORE_ERROR( "ShaderDataTypes are Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4 and Bool" );
		SAT_CORE_ASSERT( false, "Unknown ShaderDataType!" );
		return 0;
	}

	struct VertexBufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		VertexBufferElement() = default;

		VertexBufferElement( ShaderDataType type, const std::string& name, bool normalized = false )
			: Name( name ), Type( type ), Size( ShaderDataTypeSize( type ) ), Offset( 0 ), Normalized( normalized )
		{
		}

		uint32_t GetComponentCount() const
		{
			switch( Type )
			{
				case ShaderDataType::Float:   return 1;
				case ShaderDataType::Float2:  return 2;
				case ShaderDataType::Float3:  return 3;
				case ShaderDataType::Float4:  return 4;
				case ShaderDataType::Mat3:    return 3 * 3;
				case ShaderDataType::Mat4:    return 4 * 4;
				case ShaderDataType::Int:     return 1;
				case ShaderDataType::Int2:    return 2;
				case ShaderDataType::Int3:    return 3;
				case ShaderDataType::Int4:    return 4;
				case ShaderDataType::Bool:    return 1;
			}

			SAT_CORE_ASSERT( false, "Unknown ShaderDataType!" );
			return 0;
		}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() { }

		VertexBufferLayout( const std::initializer_list<VertexBufferElement>& elements )
			: m_Elements( elements )
		{
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }

		std::vector<VertexBufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<VertexBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride( void )
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for( auto& element : m_Elements )
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	enum class VertexBufferUsage
	{
		None = 0, Static = 1, Dynamic = 2
	};

	class VertexBuffer : public RefCounted
	{
	public:
		virtual ~VertexBuffer() { }

		virtual void SetData( void* buffer, uint32_t size, uint32_t offset = 0 ) = 0;
		virtual void Bind( void ) const = 0;

		virtual const VertexBufferLayout& GetLayout( void ) const = 0;
		virtual void SetLayout( const VertexBufferLayout& layout ) = 0;

		virtual unsigned int GetSize( void ) const = 0;
		virtual RendererID GetRendererID( void ) const = 0;

		static Ref<VertexBuffer> Create( void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static );
		static Ref<VertexBuffer> Create( uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic );
	};

}