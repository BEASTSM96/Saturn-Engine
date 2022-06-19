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

#include <cassert>

namespace Saturn {
	
	ShaderLibrary::ShaderLibrary()
	{
	}

	ShaderLibrary::~ShaderLibrary()
	{
		m_Shaders.clear();
	}

	void ShaderLibrary::Add( const Ref<Shader>& shader )
	{
		auto& rName = shader->GetName();
		
		SAT_CORE_ASSERT( m_Shaders.find( rName ) == m_Shaders.end() );

		m_Shaders[ rName ] = shader;
	}

	void ShaderLibrary::Load( const std::string& path )
	{
		auto shader = Ref<Shader>::Create( path );
	
		Add( shader );
	}

	void ShaderLibrary::Load( const std::string& name, const std::string& path )
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) == m_Shaders.end() );

		m_Shaders[ name ] = Ref<Shader>::Create( path );
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

	Shader::Shader( std::string Name, std::filesystem::path Filepath )
	{
		m_Filepath = std::move( Filepath );
		m_Name = std::move( Name );

		ReadFile();
		DetermineShaderTypes();
		
		CompileGlslToSpvAssembly();
				
		Reflect( {} );

		GetAvailableUniform();
	}

	Shader::Shader( std::filesystem::path Filepath )
	{
		m_Filepath = std::move( Filepath );
		m_Name = std::move( Filepath.filename().string() );

		ReadFile();
		DetermineShaderTypes();

		CompileGlslToSpvAssembly();

		Reflect( {} );

		GetAvailableUniform();
	}

	Shader::Shader( std::string Filepath )
	{
		m_Filepath = std::move( Filepath );
		
		// Strip the extension from the filename
		auto name = m_Filepath.filename().string();
		auto pos = name.find_last_of( '.' );
		if ( pos != std::string::npos )
		{
			name = name.substr( 0, pos );
		}

		m_Name = std::move( name );

		ReadFile();
		DetermineShaderTypes();

		CompileGlslToSpvAssembly();

		Reflect( {} );

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
		
		for( auto& uniform : m_Uniforms )
		{
			uniform.Terminate();
		}

		m_Uniforms.clear();

		vkDestroyDescriptorSetLayout( VulkanContext::Get().GetDevice(), m_SetLayout, nullptr );
		
		m_SetPool = nullptr;
	}

	void Shader::WriteDescriptor( ShaderType Type, const std::string& rName, VkWriteDescriptorSet& rWriteDescriptor )
	{
		m_DescriptorWrites[ Type ][ rName ] = rWriteDescriptor;

		vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &rWriteDescriptor, 0, nullptr );
	}

	void Shader::WriteAllUBs( const Ref< DescriptorSet >& rSet )
	{
		// Iterate over uniform buffers
		for( auto& [stage, bindingsMap] : m_UniformBuffers )
		{
			for( auto& [binding, buffer] : bindingsMap )
			{
				if( m_DescriptorWrites[ stage ][ buffer.Name ].descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER )
					continue;
				
				VkDescriptorBufferInfo BufferInfo = {};
				BufferInfo.buffer = buffer.Buffer;
				BufferInfo.offset = 0;
				BufferInfo.range = buffer.Size;

				m_DescriptorWrites[ stage ][ buffer.Name ].dstSet = rSet->GetVulkanSet();
				m_DescriptorWrites[ stage ][ buffer.Name ].pBufferInfo = &BufferInfo;
				
				vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &m_DescriptorWrites[ stage ][ buffer.Name ], 0, nullptr );
			}
		}
	}

	void* Shader::MapUB( ShaderType Type, uint32_t Binding )
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		auto bufferAloc = pAllocator->GetAllocationFromBuffer( m_UniformBuffers[ Type ][ Binding ].Buffer );
		
		return pAllocator->MapMemory< void >( bufferAloc );
	}

	void Shader::UnmapUB( ShaderType Type, uint32_t Binding )
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();
		
		auto bufferAloc = pAllocator->GetAllocationFromBuffer( m_UniformBuffers[ Type ][ Binding ].Buffer );
		
		pAllocator->UnmapMemory( bufferAloc );
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

	void Shader::Reflect( const std::vector<uint32_t>& rShaderData )
	{
		Timer t;

		SAT_CORE_INFO( "Shader Reflecting, {0}", m_Name );
		
		ReflectOutput Output = ShaderReflector::Get().ReflectShader( this );

		SAT_CORE_INFO( "Reflecting took: {0}", t.ElapsedMilliseconds() );
		
		SAT_CORE_INFO( " {0} Descriptors", Output.Descriptors.size() );

		for( auto& rDescriptor : Output.Descriptors )
		{
			// I know the for loop has a boolean condition, but I am still going to do this.
			if( rDescriptor.Members.size() > 0 )
			{
				SAT_CORE_INFO( " {0} Is a uniform block.", rDescriptor.Name );
				
				int i = 0;
				
				std::vector< ShaderUniform > UBMembers;

				for( auto& rMember : rDescriptor.Members )
				{
					SAT_CORE_INFO( "  {0}", rMember.Name );
					SAT_CORE_INFO( "   Offset {0}", rMember.Offset );
					SAT_CORE_INFO( "   Size {0}", rMember.Size );
					SAT_CORE_INFO( "   Type {0}", ShaderDataTypeToString( rMember.Type ) );

					// Check if the uniform already exists in list, if not add it.
					auto result = std::find_if( UBMembers.begin(), UBMembers.end(), [&]( const ShaderUniform& rUniform )
					{
						return rUniform.GetName() == rMember.Name;
					} );
					
					if( result == UBMembers.end() )
					{
						ShaderUniform Uniform = ShaderUniform( rDescriptor.Name + "." + rMember.Name, i, rMember.Type, rMember.Size, ( uint32_t ) rMember.Offset );

						UBMembers.push_back( Uniform );

						Uniform.Terminate();
					}

					i++;
				}
			
				m_UniformBuffers[ VulkanStageToSaturn( rDescriptor.StageFlags ) ][ rDescriptor.Binding ] = ShaderUniformBuffer( rDescriptor.Name, rDescriptor.Binding, rDescriptor.Size, VulkanStageToSaturn( rDescriptor.StageFlags ), nullptr, UBMembers );
			}
			else
			{
				SAT_CORE_INFO( " {0} Is a not uniform block.", rDescriptor.Name );
				
				// Add textures.

				if( rDescriptor.Type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER )
				{
					m_Textures.push_back( { VulkanStageToSaturn( rDescriptor.StageFlags ), rDescriptor.Binding, rDescriptor.Name } );

					SAT_CORE_INFO( "  At ({0}, {1}) texture is present.", ( int ) VulkanStageToSaturn( rDescriptor.StageFlags ), rDescriptor.Binding );
				}

				// Check if the uniform already exists in list, if not add it.
				auto result = std::find_if( m_Uniforms.begin(), m_Uniforms.end(), [&]( const ShaderUniform& rUniform )
				{
					return rUniform.GetName() == rDescriptor.Name;
				} );

				if( result == m_Uniforms.end() )
				{
					ShaderUniform Uniform = ShaderUniform( rDescriptor.Name, rDescriptor.Binding, VulkanDescriptorToShaderDataType( rDescriptor.Type ), sizeof( rDescriptor ), ( uint32_t ) rDescriptor.Offset );

					m_Uniforms.push_back( Uniform );
				}
			}
		}
		
		SAT_CORE_INFO( "Push constants:" );
		for ( auto& PushConstant : Output.PushConstants )
		{
			SAT_CORE_INFO( " {0}:", PushConstant.Name );
			SAT_CORE_INFO( "  Offset: {0}", PushConstant.Offset );
			SAT_CORE_INFO( "  Size: {0}", PushConstant.Size );

			uint32_t Offset = 0;
			
			if( m_PushConstantRanges.size() )
				Offset = m_PushConstantRanges.back().offset + m_PushConstantRanges.back().size;

			m_PushConstantRanges.push_back( { .stageFlags = PushConstant.StageFlags, .offset = Offset, .size = (uint32_t)PushConstant.Size } );

			for( auto& rPCMember : PushConstant.Members )
			{
				SAT_CORE_INFO( "   {0}", rPCMember.Name );
				SAT_CORE_INFO( "    Offset {0}", rPCMember.Offset );
				SAT_CORE_INFO( "    Size {0}", rPCMember.Size );
				SAT_CORE_INFO( "    Type {0}", ShaderDataTypeToString( rPCMember.Type ) );

				auto result = std::find_if( m_Uniforms.begin(), m_Uniforms.end(), [&]( const ShaderUniform& rUniform )
					{
						return rUniform.GetName() == rPCMember.Name;
					} );

				if( result == m_Uniforms.end() )
				{
					// Although this is still push constant data we only want push constant data in the fragment because push constant data in the vertex shader is not important.
					bool PushConstantData = false;
					
					if( PushConstant.StageFlags == VK_SHADER_STAGE_FRAGMENT_BIT )
						PushConstantData = true;

					ShaderUniform Uniform = ShaderUniform( PushConstant.Name + "." + rPCMember.Name, ( int ) rPCMember.Offset, rPCMember.Type, rPCMember.Size, ( uint32_t ) rPCMember.Offset, PushConstantData );
						

					// Reason why we pass in the offset twice is because that is kind of the location in the push constant buffer.
					m_Uniforms.push_back( Uniform );
				}
			}
		}

		// Create the descriptor set layout.
		
		std::vector< VkDescriptorSetLayoutBinding > Bindings;
		std::vector< VkDescriptorPoolSize > PoolSizes;

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		// Iterate over uniform buffers
		for( auto& [ stage, bindingsMap ] : m_UniformBuffers )
		{
			for ( auto& [ binding, buffer ] : bindingsMap )
			{
				VkDescriptorSetLayoutBinding Binding = {};
				Binding.binding = binding;
				Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				Binding.descriptorCount = 1;
				Binding.stageFlags = stage == ShaderType::Vertex ? VK_SHADER_STAGE_VERTEX_BIT : stage == ShaderType::All ? VK_SHADER_STAGE_ALL : VK_SHADER_STAGE_FRAGMENT_BIT;
				Binding.pImmutableSamplers = nullptr;

				VkBufferCreateInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				BufferInfo.size = buffer.Size;
				BufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				pAllocator->AllocateBuffer( BufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, &buffer.Buffer );

				PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000 } );

				Bindings.push_back( Binding );

				m_DescriptorWrites[ stage ][ buffer.Name ]  = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = nullptr,
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pImageInfo = nullptr,
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr };
			}
		}

		// Iterate over textures
		for ( auto& [ stage, binding, name ] : m_Textures )
		{
			VkDescriptorSetLayoutBinding Binding = {};
			Binding.binding = binding;
			Binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			Binding.descriptorCount = 1;
			Binding.stageFlags = stage == ShaderType::Vertex ? VK_SHADER_STAGE_VERTEX_BIT : stage == ShaderType::All ? VK_SHADER_STAGE_ALL : VK_SHADER_STAGE_FRAGMENT_BIT;
			Binding.pImmutableSamplers = nullptr;

			PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );

			Bindings.push_back( Binding );

			m_DescriptorWrites[ stage ][ name ] = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = nullptr,
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = nullptr,
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr };
		}

		VkDescriptorSetLayoutCreateInfo LayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		LayoutInfo.bindingCount = Bindings.size();
		LayoutInfo.pBindings = Bindings.data();
		
		VK_CHECK( vkCreateDescriptorSetLayout( VulkanContext::Get().GetDevice(), &LayoutInfo, nullptr, &m_SetLayout ) );

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