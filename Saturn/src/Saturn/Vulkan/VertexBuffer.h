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

#include "Base.h"

#include "ShaderDataType.h"

#include "VulkanAllocator.h"

#include <vulkan.h>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Saturn {
	
	struct BaseVertex
	{
		glm::vec3 Position;
		glm::vec2 Texcoord;
	};

	struct MeshVertex : public BaseVertex
	{
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
	};
	
	struct VertexBufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;

		VertexBufferElement() = default;

		VertexBufferElement( ShaderDataType type, const std::string& name )
			: Name( name ), Type( type ), Size( ShaderDataTypeSize( type ) ), Offset( 0 ) 
		{

		}

		uint32_t GetComponentCount() const
		{
			switch( Type )
			{
				case ShaderDataType::Float:       return 1;
				case ShaderDataType::Float2:      return 2;
				case ShaderDataType::Float3:      return 3;
				case ShaderDataType::Float4:      return 4;
				case ShaderDataType::Mat3:        return 3 * 3;
				case ShaderDataType::Mat4:        return 4 * 4;
				case ShaderDataType::Int:         return 1;
				case ShaderDataType::Int2:        return 2;
				case ShaderDataType::Int3:        return 3;
				case ShaderDataType::Int4:        return 4;
				case ShaderDataType::Bool:        return 1;
				case ShaderDataType::Sampler2D:   return 1;
				case ShaderDataType::SamplerCube: return 1;
			
				SAT_CORE_ASSERT( false, "Unknown ShaderDataType!" );
			}
		}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() { }
		
		VertexBufferLayout( const std::initializer_list< VertexBufferElement >& elements )
			: m_Elements( elements )
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
		
		inline VertexBufferLayout& operator=( VertexBufferLayout&& rrOther ) noexcept
		{
			m_Elements = std::move( rrOther.m_Elements );
			m_Stride = rrOther.m_Stride;
			
			return *this;
		}

		

		inline const std::vector< VertexBufferElement >& GetElements() const { return m_Elements; }
		inline uint32_t GetStride() const { return m_Stride; }
		
		// Vector helpers.

		std::vector< VertexBufferElement >::iterator begin() { return m_Elements.begin(); }
		std::vector< VertexBufferElement >::iterator end() { return m_Elements.end(); }
		std::vector< VertexBufferElement >::const_iterator begin() const { return m_Elements.begin(); }
		std::vector< VertexBufferElement >::const_iterator end() const { return m_Elements.end(); }

	private:
		std::vector< VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};
	
	// A vulkan vertex buffer.
	class VertexBuffer
	{
	public:
		VertexBuffer() : m_pData( nullptr ) { }

		VertexBuffer( void* pData, VkDeviceSize Size, VkBufferUsageFlags Usage = 0 );
		
		VertexBuffer( const VertexBuffer& ) = delete;

		~VertexBuffer();
		void Terminate();
		

		void Bind( VkCommandBuffer CommandBuffer );
		void Draw( VkCommandBuffer CommandBuffer );
		void BindAndDraw( VkCommandBuffer CommandBuffer );

	private:
		
		void CreateBuffer();

	private:
		
		void* m_pData = nullptr;
		size_t m_Size = 0;
		
		VertexBufferLayout Layout;
	
		VkBuffer m_Buffer;

		VmaAllocation m_Allocation;
	};
}