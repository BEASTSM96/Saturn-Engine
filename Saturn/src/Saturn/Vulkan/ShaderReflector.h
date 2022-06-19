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

#include "Shader.h"

#include "spirv_reflect/spirv_reflect.h"

#include <vulkan.h>

#define SPV_REFLECT_CHECK( x )  \
{\
	SAT_CORE_ASSERT( x == SPV_REFLECT_RESULT_SUCCESS ) \
}

namespace Saturn {
	
	struct ReflectionDescriptorMember 
	{
		std::string Name = "";

		int Offset = -1;
		size_t Size = -1;

		std::string RawType = "";

		ShaderDataType Type = ShaderDataType::None;

		bool operator == ( const ReflectionDescriptorMember& other ) const
		{
			return Name == other.Name && Offset == other.Offset && Size == other.Size && RawType == other.RawType && Type == other.Type;
		}
		
		bool operator != ( const ReflectionDescriptorMember& other ) const
		{
			return !( *this == other );
		}
	};

	struct ReflectionDescriptor
	{
		std::string Name = "";
		int Set = -1;
		int Binding = -1;
		int Count = -1;
		uint32_t Offset = -1;
		size_t Size = 0;
		bool Accessed = false;
		VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM; // i.e VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		
		VkShaderStageFlags StageFlags = 0;

		std::vector< ReflectionDescriptorMember > Members;

		bool operator == ( const ReflectionDescriptor& other ) const
		{
			return Name == other.Name && Set == other.Set && Binding == other.Binding && Count == other.Count && Accessed == other.Accessed && Type == other.Type && Members == other.Members;
		}
	};

	struct ReflectionPushConstant
	{
		std::string Name = "";
		size_t Size = 0;
		uint32_t Offset = 0;
		VkShaderStageFlags StageFlags = 0;
		
		std::vector< ReflectionDescriptorMember > Members;
	};

	struct ReflectOutput
	{
		std::string SourceLanguage = "";
		std::string EntryPoint = "";

		int SourceVersion = 0;
		
		std::vector<ReflectionPushConstant> PushConstants;

		std::vector< ReflectionDescriptor > Descriptors;
	};

	class ShaderReflector : public CountedObj
	{
		SINGLETON( ShaderReflector );
	public:
		 ShaderReflector();
		~ShaderReflector();

		ReflectOutput ReflectShader( const Ref< Shader >& pShader );
		ReflectOutput ReflectShader( Shader* pShader );

	private:
		ReflectionDescriptor ReflectDescriptor( SpvReflectDescriptorBinding& rBinding, spv_reflect::ShaderModule& Module );
		ReflectionPushConstant ReflectPushConstant( SpvReflectBlockVariable& rBinding, spv_reflect::ShaderModule& Module );

		std::string ComponentTypeToString( const SpvReflectTypeDescription& rType, uint32_t Flags );
		ShaderDataType ComponentTypeToShaderDataType( const SpvReflectTypeDescription& rType, uint32_t Flags );

		std::string DescriptorTypeToString( SpvReflectDescriptorType Value );
		VkDescriptorType DescriptorTypeToVulkan( SpvReflectDescriptorType Value );
	};
}