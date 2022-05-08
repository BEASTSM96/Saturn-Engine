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

#include "sppch.h"
#include "ShaderReflector.h"

namespace Saturn {

	ShaderReflector::ShaderReflector()
	{
	}

	ShaderReflector::~ShaderReflector()
	{
	}

	ReflectOutput ShaderReflector::ReflectShader( const Ref< Shader >& pShader )
	{
		return ReflectShader( pShader );
	}

	ReflectOutput ShaderReflector::ReflectShader( Shader* pShader )
	{
		ReflectOutput Out;

		for( auto&& [key, src] : pShader->GetSpvCode() )
		{
			std::vector< char > buffer;
			buffer.resize( src.size() * sizeof( uint32_t ) );
			memcpy( buffer.data(), ( char* )src.data(), src.size() );

			//spv_reflect::ShaderModule Module( buffer.size(), src.data() );
			spv_reflect::ShaderModule Module( src );

			SpvReflectShaderModule SPVShaderModule = Module.GetShaderModule();

			Out.EntryPoint = Module.GetEntryPointName();
			Out.SourceLanguage = spvReflectSourceLanguage( SPVShaderModule.source_language );
			Out.SourceVersion = SPVShaderModule.source_language_version;
			
			// Reflect over descriptors

			std::vector< SpvReflectDescriptorBinding* > Bindings;

			uint32_t Count = 0;
			SPV_REFLECT_CHECK( Module.EnumerateDescriptorBindings( &Count, nullptr ) );

			Bindings.resize( Count );

			SPV_REFLECT_CHECK( Module.EnumerateDescriptorBindings( &Count, Bindings.data() ) );

			std::sort( std::begin( Bindings ), std::end( Bindings ), []( SpvReflectDescriptorBinding* pA, SpvReflectDescriptorBinding* pB ) -> bool
				{
					if( pA->set != pB->set )
					{
						return pA->set < pB->set;
					}
					return pA->binding < pB->binding;
				} );

			for( int i = 0; i < Bindings.size(); i++ )
			{
				Out.Descriptors.push_back( ReflectDescriptor( *Bindings[ i ], Module ) );
			}
		}

		return Out;
	}

	ReflectionDescriptor ShaderReflector::ReflectDescriptor( SpvReflectDescriptorBinding& rBinding, spv_reflect::ShaderModule& Module )
	{
		ReflectionDescriptor Out;

		Out.Set = rBinding.set;
		Out.Binding = rBinding.binding;
		Out.Count = rBinding.count;
		Out.Accessed = ( bool )rBinding.accessed;
		Out.Name = std::move( rBinding.name == nullptr ? rBinding.type_description->type_name : rBinding.name );
		Out.Type = DescriptorTypeToVulkan( rBinding.descriptor_type );

		// Test if descriptor is a uniform buffer or a storage buffer.
		//if( rBinding.decoration_flags == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER || rBinding.decoration_flags == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER )
		{
			auto& rBlockValue = rBinding.block;

			for( int i = 0; i < rBlockValue.member_count; i++ )
			{
				ReflectionDescriptorMember Member;

				SpvReflectBlockVariable& rSPVMember = rBlockValue.members[ i ];

				Member.Name = rSPVMember.name;
				Member.Offset = rSPVMember.offset;
				Member.Size = rSPVMember.size;
				Member.RawType = ComponentTypeToString( *rSPVMember.type_description, rSPVMember.decoration_flags );
				Member.Type = ComponentTypeToShaderDataType( *rSPVMember.type_description, rSPVMember.decoration_flags );
				
				if( std::find( std::begin( Out.Members ), std::end( Out.Members ), Member ); == std::end( Out.Members ) )
				{
					Out.Members.push_back( Member );
				}
			}
		}

		return Out;
	}

	std::string ShaderReflector::ComponentTypeToString( const SpvReflectTypeDescription& rType, uint32_t Flags )
	{
		uint32_t MaskedType = rType.type_flags & 0xFFF;

		std::stringstream ss;

		if ( rType.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX )
		{
			if( Flags & SPV_REFLECT_DECORATION_COLUMN_MAJOR )
			{
				ss << "colum_major";
			}
			else if( Flags & SPV_REFLECT_DECORATION_ROW_MAJOR )
			{
				ss << "row_major";
			}
		}

		switch( MaskedType )
		{
			case SPV_REFLECT_TYPE_FLAG_BOOL: ss << "bool"; break;
			case SPV_REFLECT_TYPE_FLAG_FLOAT: ss << "float"; break;
			case SPV_REFLECT_TYPE_FLAG_INT: ss << ( rType.traits.numeric.scalar.signedness ? "int" : "uint" ); break;
		}

		if ( rType.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX )
		{
			// Could make matrix4x4.
			ss << rType.traits.numeric.matrix.row_count;
			ss << "x";
			ss << rType.traits.numeric.matrix.column_count;
		}
		else if( rType.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR ) 
		{
			// Where component_count could equal 2, 3, 4, i.e. vec3
			ss << rType.traits.numeric.vector.component_count;
		}

		return ss.str();
	}

	ShaderDataType ShaderReflector::ComponentTypeToShaderDataType( const SpvReflectTypeDescription& rType, uint32_t Flags )
	{
		uint32_t MaskedType = rType.type_flags & 0xFFF;

		ShaderDataType Type = ShaderDataType::None;

		switch( MaskedType )
		{
			case SPV_REFLECT_TYPE_FLAG_BOOL: Type = ShaderDataType::Bool; break;
			case SPV_REFLECT_TYPE_FLAG_FLOAT: Type = ShaderDataType::Float; break;
			case SPV_REFLECT_TYPE_FLAG_INT: Type = ShaderDataType::Int; break;
			case SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE: Type = ShaderDataType::Sampler2D; break;
		}

		if( rType.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX )
		{
			if ( rType.traits.numeric.matrix.row_count == 4 && rType.traits.numeric.matrix.column_count == 4 )
			{
				Type = ShaderDataType::Mat4;
			}
			else if( rType.traits.numeric.matrix.row_count == 3 && rType.traits.numeric.matrix.column_count == 3 )
			{
				Type = ShaderDataType::Mat3;
			}
		}
		else if( rType.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR )
		{
			// Where component_count could equal 2, 3, 4, i.e. vec3

			if ( rType.traits.numeric.vector.component_count == 2 )
			{
				Type = ShaderDataType::Float2;
			}
			else if( rType.traits.numeric.vector.component_count == 3 ) 
			{
				Type = ShaderDataType::Float3;
			}
			else if( rType.traits.numeric.vector.component_count == 4 )
			{
				Type = ShaderDataType::Float4;
			}
		}

		return Type;
	}

	std::string ShaderReflector::DescriptorTypeToString( SpvReflectDescriptorType Value )
	{
		switch( Value ) 
		{
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return "VK_DESCRIPTOR_TYPE_SAMPLER";
			case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
			case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
			case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR";
		}

		return "VK_DESCRIPTOR_TYPE_UNKNOWN";
	}

	VkDescriptorType ShaderReflector::DescriptorTypeToVulkan( SpvReflectDescriptorType Value )
	{
		switch( Value )
		{
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
			case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		}

		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

}