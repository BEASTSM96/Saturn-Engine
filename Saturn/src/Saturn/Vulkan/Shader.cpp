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
#include "Shader.h"

#include "DescriptorSet.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

#include <istream>
#include <fstream>
#include <iostream>

#include <shaderc/shaderc.hpp>
#include <shaderc/shaderc.h>

#include "ShaderReflector.h"
#include <spirv/spirv_glsl.hpp>

#include <cassert>

namespace Saturn {
	
	VkShaderStageFlags ShaderTypeToVulkan( ShaderType type ) 
	{
		switch( type )
		{
			case Saturn::ShaderType::None:
				return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			case Saturn::ShaderType::Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case Saturn::ShaderType::Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			case Saturn::ShaderType::Geometry:
				return VK_SHADER_STAGE_COMPUTE_BIT;
			case Saturn::ShaderType::Compute:
				return VK_SHADER_STAGE_COMPUTE_BIT;
			case Saturn::ShaderType::All:
				return VK_SHADER_STAGE_ALL;
		}

		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}

	ShaderDataType SpvToSaturn( spirv_cross::SPIRType type )
	{
		switch( type.basetype )
		{
			case spirv_cross::SPIRType::Boolean: return ShaderDataType::Bool;
			case spirv_cross::SPIRType::Int: return ShaderDataType::Int;

			case spirv_cross::SPIRType::Float: 
			{
				if( type.vecsize == 1 ) return ShaderDataType::Int;
				if( type.vecsize == 2 ) return ShaderDataType::Int2;
				if( type.vecsize == 3 ) return ShaderDataType::Int3;
				if( type.vecsize == 4 ) return ShaderDataType::Int4;

				if( type.columns == 3 ) return ShaderDataType::Mat3;
				if( type.columns == 4 ) return ShaderDataType::Mat4;
			} break;

			case spirv_cross::SPIRType::SampledImage: return ShaderDataType::Sampler2D;
		}

		return ShaderDataType::None;
	}

	ShaderLibrary::ShaderLibrary()
	{
	}

	ShaderLibrary::~ShaderLibrary()
	{
	}

	void ShaderLibrary::Add( const Ref<Shader>& shader )
	{
		auto& rName = shader->GetName();
		
		SAT_CORE_ASSERT( m_Shaders.find( rName ) == m_Shaders.end() );

		m_Shaders[ rName ] = shader;
	}

	void ShaderLibrary::Load( const std::string& path )
	{
		Ref<Shader> shader = Ref<Shader>::Create( path );
	
		Add( shader );
	}

	void ShaderLibrary::Load( const std::string& name, const std::string& path )
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) == m_Shaders.end() );

		m_Shaders[ name ] = Ref<Shader>::Create( path );
	}

	void ShaderLibrary::Remove( const Ref<Shader>& shader )
	{
		m_Shaders[ shader->GetName() ] = nullptr;
		m_Shaders.erase( shader->GetName() );
	}

	void ShaderLibrary::Shutdown()
	{
		for ( auto& [name, shader] : m_Shaders )
		{
			if( shader )
				shader = nullptr;
		}

		m_Shaders.clear();
	}

	const Ref<Shader>& ShaderLibrary::Find( const std::string& name ) const
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) != m_Shaders.end() );

		return m_Shaders.at( name );
	}

	//////////////////////////////////////////////////////////////////////////
	
	ShaderType ShaderTypeFromString( const std::string& Str )
	{
		if( Str == "vertex" )
		{
			return ShaderType::Vertex;
		}
		else if( Str == "fragment" )
		{
			return ShaderType::Fragment;
		}
		else if( Str == "compute" )
		{
			return ShaderType::Compute;
		}
		else if( Str == "geometry" )
		{
			return ShaderType::Geometry;
		}
		else
		{
			return ShaderType::Vertex;
		}
	}

	std::string ShaderTypeToString( ShaderType Type )
	{
		switch( Type )
		{
			case Saturn::ShaderType::Vertex:
				return "Vertex";
				break;
			case Saturn::ShaderType::Fragment:
				return "Fragment";
				break;
			case Saturn::ShaderType::Geometry:
				return "Geometry";
				break;
			case Saturn::ShaderType::Compute:
				return "Compute";
				break;
			default:
				break;
		}
	}
	
	//////////////////////////////////////////////////////////////////////////

	Shader::Shader( std::filesystem::path Filepath )
	{
		m_Filepath = std::move( Filepath );
		m_Name = m_Filepath.filename().string();

		// Remove the extension from the name
		m_Name.erase( m_Name.find_last_of( '.' ) );

		ReadFile();
		DetermineShaderTypes();

		CompileGlslToSpvAssembly();

		for ( const auto& [k, data] : m_SpvCode )
		{
			Reflect( k.Type, data );
		}

		CreateDescriptors();

		GetAvailableUniform();
	}

	Shader::~Shader()
	{
		m_SpvCode.clear();
		m_ShaderSources.clear();
		
		for ( auto& uniform : m_Uniforms )
		{
			uniform.Terminate();
		}
		
		m_Uniforms.clear();

		for ( auto& [ set, descriptorSet ] : m_DescriptorSets )
		{
			vkDestroyDescriptorSetLayout( VulkanContext::Get().GetDevice(), descriptorSet.SetLayout, nullptr );
		}

		m_SetLayouts.clear();

		m_SetPool = nullptr;
	}

	void Shader::WriteDescriptor( const std::string& rName, VkDescriptorImageInfo& rImageInfo, VkDescriptorSet desSet )
	{
		for( auto& [set, descriptorSet] : m_DescriptorSets )
		{
			for( auto& texture : descriptorSet.SampledImages )
			{
				if( texture.Name == rName )
				{
					descriptorSet.WriteDescriptorSets[ texture.Binding ].pImageInfo = &rImageInfo;
					descriptorSet.WriteDescriptorSets[ texture.Binding ].dstSet = desSet;
					
					vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &descriptorSet.WriteDescriptorSets[ texture.Binding ], 0, nullptr );
				}
			}
		}
	}

	void Shader::WriteDescriptor( const std::string& rName, VkDescriptorBufferInfo& rBufferInfo, VkDescriptorSet desSet )
	{
		for( auto& [set, descriptorSet] : m_DescriptorSets )
		{
			for( auto& [ binding, ub ] : descriptorSet.UniformBuffers )
			{
				if( ub.Name == rName )
				{
					descriptorSet.WriteDescriptorSets[ binding ].pBufferInfo = &rBufferInfo;
					descriptorSet.WriteDescriptorSets[ binding ].dstSet = desSet;

					vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &descriptorSet.WriteDescriptorSets[ binding ], 0, nullptr );
				}
			}
		}
	}

	void Shader::WriteAllUBs( const Ref< DescriptorSet >& rSet )
	{
		if( !rSet )
			SAT_CORE_ASSERT( false, "DescriptorSet is null!" );

		// Iterate over uniform buffers
		for( auto& [set, descriptorSet] : m_DescriptorSets )
		{
			for( auto& [binding, ub] : descriptorSet.UniformBuffers )
			{
				VkDescriptorBufferInfo BufferInfo = {};
				BufferInfo.buffer = ub.Buffer;
				BufferInfo.offset = 0;
				BufferInfo.range = ub.Size;
				
				descriptorSet.WriteDescriptorSets[ binding ].pBufferInfo = &BufferInfo;
				descriptorSet.WriteDescriptorSets[ binding ].dstSet = rSet->GetVulkanSet();

				vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &descriptorSet.WriteDescriptorSets[ binding ], 0, nullptr );
			}
		}
	}

	void* Shader::MapUB( ShaderType Type, uint32_t Set, uint32_t Binding )
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		auto bufferAloc = pAllocator->GetAllocationFromBuffer( m_DescriptorSets[ Set ].UniformBuffers[ Binding ].Buffer );
		
		return pAllocator->MapMemory< void >( bufferAloc );
	}

	void Shader::UnmapUB( ShaderType Type, uint32_t Set, uint32_t Binding )
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();
		
		auto bufferAloc = pAllocator->GetAllocationFromBuffer( m_DescriptorSets[ Set ].UniformBuffers[ Binding ].Buffer );
		
		pAllocator->UnmapMemory( bufferAloc );
	}
	
	Ref<DescriptorSet> Shader::CreateDescriptorSet( uint32_t set )
	{
		DescriptorSetSpecification Specification;
		Specification.Layout = m_DescriptorSets[ set ].SetLayout;
		Specification.Pool = m_SetPool;
		Specification.SetIndex = set;

		return Ref<DescriptorSet>::Create( Specification );
	}

	void Shader::ReadFile()
	{
		std::ifstream f( m_Filepath, std::ios::ate | std::ios::binary );

		if( !std::filesystem::exists( m_Filepath ) )
			return;

		m_FileSize = static_cast< size_t >( f.tellg() );
		std::vector<char> Buffer( m_FileSize );

		f.seekg( 0 );
		f.read( Buffer.data(), m_FileSize );

		f.close();

		m_FileContents = std::string( Buffer.begin(), Buffer.end() );
	}

	void Shader::GetAvailableUniform()
	{

	}

	void Shader::DetermineShaderTypes()
	{
		int VertexShaders = -1;
		int FragmentShaders = -1;
		int ComputeShaders = -1;

		const char* TypeToken = "#type";
		size_t TypeTokenLength = strlen( TypeToken );
		size_t TypeTokenPosition = m_FileContents.find( TypeToken, 0 );
		
		while ( TypeTokenPosition != std::string::npos )
		{
			std::string FileCopy;
			
			std::copy( m_FileContents.begin(), m_FileContents.end(), std::back_inserter( FileCopy ) );
			
			size_t TypeTokenEnd = FileCopy.find( "\r\n", TypeTokenPosition );

			size_t Begin = TypeTokenPosition + TypeTokenLength + 1;

			std::string Type = FileCopy.substr( Begin, TypeTokenEnd - Begin );
			
			size_t NextLinePos = FileCopy.find_first_not_of( "\r\n", TypeTokenEnd );
			TypeTokenPosition = FileCopy.find( TypeToken, NextLinePos );
			
			auto RawShaderCode = FileCopy.substr( NextLinePos, TypeTokenPosition - ( NextLinePos == std::string::npos ? FileCopy.size() - 1 : NextLinePos ) );

			auto Shader_Type = ShaderTypeFromString( Type );

			if( Shader_Type == ShaderType::Fragment )
				FragmentShaders++;
			else if( Shader_Type == ShaderType::Vertex )
				VertexShaders++;
			else if( Shader_Type == ShaderType::Compute )
				ComputeShaders++;
			
			int Index = Shader_Type == ShaderType::Vertex ? VertexShaders : ( Shader_Type == ShaderType::Fragment ? FragmentShaders : ComputeShaders );

			ShaderSource src = ShaderSource( RawShaderCode, Shader_Type, Index );
			m_ShaderSources[ ShaderSourceKey( Shader_Type, Index ) ] = src;
		}
	}

	void Shader::Reflect( ShaderType shaderType, const std::vector<uint32_t>& rShaderData )
	{
		spirv_cross::Compiler Compiler( rShaderData );
		auto Resources = Compiler.get_shader_resources();

		// Sort the descriptors by set and binding.
		auto Fn = [&]( const auto& a, const auto& b ) -> bool
		{
			auto aSet = Compiler.get_decoration( a.id, spv::DecorationDescriptorSet );
			auto bSet = Compiler.get_decoration( b.id, spv::DecorationDescriptorSet );

			if( aSet == bSet )
			{
				uint32_t aBinding = Compiler.get_decoration( a.id, spv::DecorationBinding );
				uint32_t bBinding = Compiler.get_decoration( b.id, spv::DecorationBinding );

				return aBinding < bBinding;
			}

			return aSet < bSet;
		};

		std::sort( Resources.uniform_buffers.begin(), Resources.uniform_buffers.end(), Fn );

		for ( const auto& ub : Resources.uniform_buffers )
		{
			const auto& Name = ub.name;
			auto& BufferType = Compiler.get_type( ub.base_type_id );
			int MemberCount = BufferType.member_types.size();
			uint32_t Binding = Compiler.get_decoration( ub.id, spv::DecorationBinding );
			uint32_t Set = Compiler.get_decoration( ub.id, spv::DecorationDescriptorSet );

			uint32_t Size = Compiler.get_declared_struct_size( BufferType );

			SAT_CORE_INFO( "Uniform Buffer: {0}", Name );
			SAT_CORE_INFO( " Size: {0}", Size );
			SAT_CORE_INFO( " Binding: {0}", Binding );
			SAT_CORE_INFO( " Set: {0}", Set);

			if( m_DescriptorSets[ Set ].Set == -1 )
				m_DescriptorSets[ Set ] = { .Set = Set };

			ShaderUniformBuffer Uniform;
			Uniform.Binding = Binding;
			Uniform.Size = Size;
			Uniform.Location = shaderType;
			Uniform.Name = Name;

			auto& UniformBuffers = m_DescriptorSets[ Set ].UniformBuffers;

			// Check if the same element already exists if so the stage "All"
			// is used to represent all stages.
			auto It = std::find_if( UniformBuffers.begin(), UniformBuffers.end(), [&]( const auto& a )
			{
				auto&& [k, v] = a;

				return v == Uniform;
			} );
			
			if( It != std::end( UniformBuffers ) )
			{
				auto&& [Binding, ub] = ( *It );

				ub.Location = ShaderType::All;

				continue;
			}

			m_DescriptorSets[ Set ].UniformBuffers[ Binding ] = Uniform;

			for( int i = 0; i < MemberCount; i++ )
			{
				auto type = Compiler.get_type( BufferType.member_types[i] );
				const auto& memberName = Compiler.get_member_name( BufferType.self, i );
				size_t size = Compiler.get_declared_struct_member_size( BufferType, i );
				auto offset = Compiler.type_struct_member_offset( BufferType, i );

				std::string MemberName = Name + "." + memberName;

				SAT_CORE_INFO( "  {0}", memberName );
				SAT_CORE_INFO( "   Size: {0}", size );
				SAT_CORE_INFO( "   Offset: {0}", offset );

				// Use Binding as location it does not matter.
				m_Uniforms.push_back( { MemberName, (int)Binding, SpvToSaturn( type ), size, offset } );
			}
		}

		for ( const auto& pc : Resources.push_constant_buffers )
		{
			const auto& Name = pc.name;
			auto& BufferType = Compiler.get_type( pc.base_type_id );
			int MemberCount = BufferType.member_types.size();
			uint32_t set = Compiler.get_decoration( pc.id, spv::DecorationDescriptorSet );

			uint32_t Size = Compiler.get_declared_struct_size( BufferType );
			uint32_t Offset = 0;

			// Make sure the buffer offset is size + offset for because that's what vulkan needs when we render.
			if( m_PushConstantRanges.size() )
				Offset = m_PushConstantRanges.back().offset + m_PushConstantRanges.back().size;

			m_PushConstantRanges.push_back( { .stageFlags = ShaderTypeToVulkan( shaderType ), .offset = Offset , .size = ( uint32_t )Size } );

			SAT_CORE_INFO( "Push constant buffer: {0}", Name );
			SAT_CORE_INFO( " Size: {0}", Size );
			SAT_CORE_INFO( " Offset: {0}", Offset );
			SAT_CORE_INFO( " Set: {0}", ( uint32_t )set );
			SAT_CORE_INFO( " Stage: {0}", ( uint32_t )shaderType );

			for( int i = 0; i < MemberCount; i++ )
			{
				auto type = Compiler.get_type( BufferType.member_types[ i ] );
				const auto& memberName = Compiler.get_member_name( BufferType.self, i );
				size_t size = Compiler.get_declared_struct_member_size( BufferType, i );
				auto offset = Compiler.type_struct_member_offset( BufferType, i );
				
				std::string MemberName;

				if( Name.empty() )
					MemberName = memberName;
				else
					MemberName = Name + "." + memberName;

				bool PushConstantData = false;

				if( shaderType == ShaderType::Fragment )
					PushConstantData = true;

				SAT_CORE_INFO( "  {0}", memberName );
				SAT_CORE_INFO( "  Size: {0}", size );
				SAT_CORE_INFO( "  Offset: {0}", offset );

				// Use Binding as location it does not matter.
				m_Uniforms.push_back( { MemberName, ( int ) offset, SpvToSaturn( type ), size, offset - Offset, PushConstantData } );
			}
		}

		for( const auto& Resource : Resources.sampled_images )
		{
			const auto& Name = Resource.name;
			auto& BufferType = Compiler.get_type( Resource.base_type_id );
			uint32_t binding = Compiler.get_decoration( Resource.id, spv::DecorationBinding );
			uint32_t set = Compiler.get_decoration( Resource.id, spv::DecorationDescriptorSet );

			SAT_CORE_INFO( "Sampled image: {0}", Name );
			SAT_CORE_INFO( " Binding: {0}", binding );
			SAT_CORE_INFO( " Set: {0}", set );

			m_DescriptorSets[ set ].SampledImages.push_back( { Name, shaderType, set, binding } );
		}
	}

	void Shader::CreateDescriptors()
	{
		// Create the descriptor set layout.

		std::vector< VkDescriptorPoolSize > PoolSizes;

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		// Iterate over descriptor sets
		for( auto& [ set, descriptorSet ] : m_DescriptorSets )
		{
			std::vector< VkDescriptorSetLayoutBinding > Bindings;

			// Iterate over uniform buffers
			for( auto& [ Binding, ub ] : descriptorSet.UniformBuffers )
			{
				VkDescriptorSetLayoutBinding Binding = {};
				Binding.binding = ub.Binding;
				Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				Binding.descriptorCount = 1;
				Binding.stageFlags = ub.Location == ShaderType::Vertex ? VK_SHADER_STAGE_VERTEX_BIT : ub.Location == ShaderType::All ? VK_SHADER_STAGE_ALL : VK_SHADER_STAGE_FRAGMENT_BIT;
				Binding.pImmutableSamplers = nullptr;

				VkBufferCreateInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				BufferInfo.size = ub.Size;
				BufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				pAllocator->AllocateBuffer( BufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, &ub.Buffer );

				PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 250 } );

				Bindings.push_back( Binding );

				descriptorSet.WriteDescriptorSets[ ub.Binding ] =
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = nullptr,
					.dstBinding = ub.Binding,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pImageInfo = nullptr,
					.pBufferInfo = nullptr,
					.pTexelBufferView = nullptr
				};
			}

			// Iterate over textures
			for( auto& texture : descriptorSet.SampledImages )
			{
				VkDescriptorSetLayoutBinding Binding = {};
				Binding.binding = texture.Binding;
				Binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				Binding.descriptorCount = 1;
				Binding.stageFlags = texture.Stage == ShaderType::Vertex ? VK_SHADER_STAGE_VERTEX_BIT : texture.Stage == ShaderType::All ? VK_SHADER_STAGE_ALL : VK_SHADER_STAGE_FRAGMENT_BIT;
				Binding.pImmutableSamplers = nullptr;

				PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 250 } );

				Bindings.push_back( Binding );

				descriptorSet.WriteDescriptorSets[ texture.Binding ] =
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = nullptr,
					.dstBinding = texture.Binding,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = nullptr,
					.pBufferInfo = nullptr,
					.pTexelBufferView = nullptr
				};
			}

			VkDescriptorSetLayoutCreateInfo LayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			LayoutInfo.bindingCount = Bindings.size();
			LayoutInfo.pBindings = Bindings.data();

			VK_CHECK( vkCreateDescriptorSetLayout( VulkanContext::Get().GetDevice(), &LayoutInfo, nullptr, &descriptorSet.SetLayout ) );

			m_SetLayouts.push_back( descriptorSet.SetLayout );
		}

		m_SetPool = Ref< DescriptorPool >::Create( PoolSizes, 10000 );
	}

	void Shader::CompileGlslToSpvAssembly()
	{
		shaderc::Compiler       Compiler;
		shaderc::CompileOptions CompilerOptions;

		// We don't use shaderc_optimization_level_zero, as will remove the uniform names, it took me 6 hours to figure out.
		CompilerOptions.SetOptimizationLevel( shaderc_optimization_level_zero );

		CompilerOptions.SetWarningsAsErrors();

		CompilerOptions.SetTargetEnvironment( shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2 );
		CompilerOptions.SetTargetSpirv( shaderc_spirv_version_1_5 );

		Timer CompileTimer;
		
		for ( auto&& [key, src] : m_ShaderSources )
		{
			const std::string& rShaderSrcCode = src.Source;

			const auto Res = Compiler.CompileGlslToSpv(
				rShaderSrcCode,
				src.Type == ShaderType::Vertex ? shaderc_shader_kind::shaderc_glsl_default_vertex_shader : shaderc_shader_kind::shaderc_glsl_default_fragment_shader,
				m_Filepath.string().c_str(),
				CompilerOptions );

			if( Res.GetCompilationStatus() != shaderc_compilation_status_success ) 
			{
				SAT_CORE_INFO( "Shader Error status {0}", Res.GetCompilationStatus() );
				SAT_CORE_INFO( "Shader Error messages {0}", Res.GetErrorMessage() );
				SAT_ASSERT( false, "Shader Compilation Failed" );
			}

			SAT_CORE_INFO( "Shader Warings {0}", Res.GetNumWarnings() );

			std::vector< uint32_t > SpvBinary( Res.begin(), Res.end() );

			m_SpvCode[ key ] = SpvBinary;
		}

		SAT_CORE_INFO( "Shader Compilation took {0} ms", CompileTimer.ElapsedMilliseconds() );
	}
}