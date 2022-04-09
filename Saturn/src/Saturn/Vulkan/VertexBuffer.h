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
#include "Buffer.h"

#include <vulkan.h>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Saturn {

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 Texcoord;
	};

	enum class FormatType
	{
		Float,
		Vector2,
		Vector3,
		Vector4,

		IVec2,
		UVec2,
		Double
	};

	inline VkFormat FormatTypeToVulkan( FormatType Type )
	{
		switch( Type )
		{
			case FormatType::Float:
				return VK_FORMAT_R32_SFLOAT;
				break;
			case FormatType::Vector2:
				return VK_FORMAT_R32G32_SFLOAT;
				break;
			case FormatType::Vector3:
				return VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case FormatType::Vector4:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			case FormatType::IVec2:
				return VK_FORMAT_R32G32_SINT;
				break;
			case FormatType::UVec2:
				return VK_FORMAT_R32G32_UINT;
				break;
			case FormatType::Double:
				return VK_FORMAT_R64_SFLOAT;
				break;
			default:
				break;
		}
	}

	inline FormatType VulkanToFormatType( VkFormat Type )
	{
		switch( Type )
		{
			case VK_FORMAT_R32_SFLOAT:
				return FormatType::Float;
				break;
			case VK_FORMAT_R32G32_SFLOAT:
				return FormatType::Vector2;
				break;
			case VK_FORMAT_R32G32B32_SFLOAT:
				return FormatType::Vector3;
				break;
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return FormatType::Vector4;
				break;
			case VK_FORMAT_R32G32_SINT:
				return FormatType::IVec2;
				break;
			case VK_FORMAT_R32G32_UINT:
				return FormatType::UVec2;
				break;
			case VK_FORMAT_R64_SFLOAT:
				return FormatType::Double;
				break;
			default:
				break;
		}
	}

	inline std::string FormatTypeToGLSLType( FormatType Type )
	{
		switch( Type )
		{
			case FormatType::Float:
				return "float";
				break;
			case FormatType::Vector2:
				return "vec2";
				break;
			case FormatType::Vector3:
				return "vec3";
				break;
			case FormatType::Vector4:
				return "vec4";
				break;
			case FormatType::IVec2:
				return "ivec2";
				break;
			case FormatType::UVec2:
				return "uvec2";
				break;
			case FormatType::Double:
				return "double";
				break;
			default:
				break;
		}
	}



	struct VertexLayoutType
	{
		FormatType Format;
		std::string NameInShader;
	};

	// A vertex layout represents a one or more vertex layouts in the shader.
	// So for example I could create one VertexLayout struct and have it contain all the data needed for a mesh.
	struct VertexLayout
	{
		VertexLayout( std::initializer_list< VertexLayoutType >& Layouts ) : Types( Layouts ) { }

		std::vector< VertexLayoutType > Types;
	};

	// A vulkan vertex buffer.
	class VertexBuffer
	{
	public:
		VertexBuffer() : m_pData( nullptr ) { }

		VertexBuffer( void* pData, VkDeviceSize Size, VkBufferUsageFlags Usage = 0 ) : m_pData( pData ) { }

		VertexBuffer( const std::vector< Vertex >& Vertices );

		VertexBuffer( const VertexBuffer& ) = delete;

		~VertexBuffer();
		void Terminate();

		void SetLayouts();

		void Bind( VkCommandBuffer CommandBuffer );
		void Draw( VkCommandBuffer CommandBuffer );
		void BindAndDraw( VkCommandBuffer CommandBuffer );

		void CreateBuffer();

		static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

		std::vector<Vertex> m_Vertices;

	private:

		void* m_pData = nullptr;
	
		Buffer m_Buffer;
	};
}