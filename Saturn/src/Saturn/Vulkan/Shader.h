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

#include "Saturn/Core/UUID.h"

#include "ShaderUniform.h"
#include "DescriptorSet.h"

#include "Base.h"
#include <vector>
#include <string>
#include <filesystem>
#include <vulkan.h>
#include <unordered_map>

namespace Saturn {
	
	enum class ShaderType : uint32_t
	{
		None = 0,
		Vertex = 1,
		Fragment = 2,
		Geometry = 3,
		Compute = 4,
		All = 5
	};
	
	// A shader uniform buffer represents a uniform buffer block in a shader.
	struct ShaderUniformBuffer
	{
		std::string Name;
		uint32_t Binding;
		size_t Size;
		ShaderType Location;
		
		VkBuffer Buffer;

		bool operator==( const ShaderUniformBuffer& rOther ) 
		{
			return Binding == rOther.Binding && Name == rOther.Name && Size == rOther.Size;
		}

		auto operator<=>( const ShaderUniformBuffer& rOther ) const = default;

		std::vector< ShaderUniform > Members;
	};

	struct ShaderSampledImage
	{
		std::string Name;
		ShaderType Stage;
		uint32_t Set;
		uint32_t Binding;
	};

	struct ShaderDescriptorSet
	{
		uint32_t Set = -1;

		VkDescriptorSetLayout SetLayout;
		
		std::unordered_map< uint32_t, VkWriteDescriptorSet > WriteDescriptorSets;
		std::vector< ShaderSampledImage > SampledImages;
		std::unordered_map< uint32_t, ShaderUniformBuffer > UniformBuffers;
	};

	struct ShaderSource
	{
		ShaderSource() {}
		~ShaderSource() {}
		
		ShaderSource( const std::string& rSrc, ShaderType Type, int Index )
			: Source( rSrc ), Type( Type ), Index( Index )
		{
		}

		std::string Source = "";
		ShaderType Type = ShaderType::Vertex;
		int Index = -1;
	};

	struct ShaderSourceKey
	{
		ShaderSourceKey() {}
		ShaderSourceKey( ShaderType _Type, int _Index ) : Type( _Type ), Index( _Index ) {}
		~ShaderSourceKey() {}

		ShaderSourceKey operator=( const ShaderSourceKey& rKey )
		{
			Type = rKey.Type;
			Index = rKey.Index;
			return *this;
		}
		
		bool operator==( const ShaderSourceKey& rKey )
		{
			return ( Type == rKey.Type && Index == rKey.Index );
		}
		
		bool operator==( const ShaderSourceKey& rKey ) const
		{
			return ( Type == rKey.Type && Index == rKey.Index );
		}

		ShaderType Type = ShaderType::Vertex;
		int Index = -1;
	};
}

namespace std {

	template<>
	struct hash< Saturn::ShaderSourceKey >
	{
		size_t operator()( const Saturn::ShaderSourceKey& rKey ) const
		{
			return ( size_t ) rKey.Index;
		}
	};

	template<>
	struct hash< Saturn::ShaderSource >
	{
		size_t operator()( const Saturn::ShaderSource& rKey )
		{
			return hash< std::string >()( rKey.Source ) ^ ( ( size_t )rKey.Type << 32 );
		}
	};

}

namespace Saturn {

	static ShaderType VulkanStageToSaturn( VkShaderStageFlags Flags )
	{
		switch( Flags )
		{
			case VK_SHADER_STAGE_VERTEX_BIT:
				return ShaderType::Vertex;
			case VK_SHADER_STAGE_FRAGMENT_BIT:
				return ShaderType::Fragment;
			case VK_SHADER_STAGE_GEOMETRY_BIT:
				return ShaderType::Geometry;
			case VK_SHADER_STAGE_COMPUTE_BIT:
				return ShaderType::Compute;
			default:
				return ShaderType::All;
		}
	}
	
	extern ShaderType ShaderTypeFromString( const std::string& Str );
	extern std::string ShaderTypeToString( ShaderType Type );

	class Shader : public CountedObj
	{
		using ShaderSourceMap = std::unordered_map< ShaderSourceKey, ShaderSource >;
		using SpvSourceMap = std::unordered_map< ShaderSourceKey, std::vector< uint32_t > >;
		using ShaderUBMap = std::unordered_map< ShaderType, std::unordered_map< uint32_t, ShaderUniformBuffer > >;
		//                                      ShaderType,                     Binding,   UniformBuffer
		
		using ShaderWriteMap = std::unordered_map< ShaderType, std::unordered_map< std::string, VkWriteDescriptorSet > >;

	public:
		Shader( std::filesystem::path Filepath );

		~Shader();

		
		std::string& GetName() { return m_Name; }
		const std::string& GetName() const { return m_Name; }

		ShaderSourceMap& GetShaderSources() { return m_ShaderSources; }
		const ShaderSourceMap& GetShaderSources() const { return m_ShaderSources; }

		const SpvSourceMap& GetSpvCode() const { return m_SpvCode; }
		SpvSourceMap& GetSpvCode() { return m_SpvCode; }
		
		const std::vector< ShaderUniform > GetUniforms() const { return m_Uniforms; }
		
		std::vector< ShaderSampledImage >& GetTextures() { return m_Textures; }
		
		Ref< DescriptorPool >& GetDescriptorPool() { return m_SetPool; }
		const Ref< DescriptorPool >& GetDescriptorPool() const { return m_SetPool; }
		
		ShaderWriteMap& GetWriteDescriptors() { return m_DescriptorWrites; }
		const ShaderWriteMap& GetWriteDescriptors() const { return m_DescriptorWrites; }
		
		std::vector< VkPushConstantRange >& GetPushConstantRanges() { return m_PushConstantRanges; }
		const std::vector< VkPushConstantRange >& GetPushConstantRanges() const { return m_PushConstantRanges; }

		void WriteDescriptor( const std::string& rName, VkDescriptorImageInfo& rImageInfo, VkDescriptorSet desSet );
		void WriteDescriptor( const std::string& rName, VkDescriptorBufferInfo& rBufferInfo, VkDescriptorSet desSet );

		// Make sure all the buffers have data mapped to them!
		void WriteAllUBs( const Ref< DescriptorSet >& rSet );

		void* MapUB( ShaderType Type, uint32_t Set, uint32_t Binding );
		void UnmapUB( ShaderType Type, uint32_t Set, uint32_t Binding );
		
		uint32_t GetDescriptorSetCount() { return m_DescriptorSetCount; }

		Ref<DescriptorSet> CreateDescriptorSet( uint32_t set );

		std::vector< VkDescriptorSetLayout >& GetSetLayouts() { return m_SetLayouts; }
		VkDescriptorSetLayout GetSetLayout( uint32_t set = 0 ) { return m_SetLayouts[ set ]; }

	private:

		void ReadFile();

		void GetAvailableUniform();

		void DetermineShaderTypes();
		
		void Reflect( ShaderType shaderType, const std::vector<uint32_t>& rShaderData );
		
		void CreateDescriptors();

		void CompileGlslToSpvAssembly();

	private:
		ShaderSourceMap m_ShaderSources;
		SpvSourceMap m_SpvCode;

		std::string m_FileContents = "";
		size_t m_FileSize;

		std::string m_Name = "";

		std::filesystem::path m_Filepath = "";
		
		std::vector< ShaderUniform > m_Uniforms;
		std::vector< ShaderUniform > m_PushConstantUniforms;
		
		ShaderUBMap m_UniformBuffers;

		std::vector< ShaderSampledImage > m_Textures;
		
		std::vector< VkPushConstantRange > m_PushConstantRanges;

		// Set -> Binding -> ShaderDescriptorSet
		std::unordered_map< uint32_t, ShaderDescriptorSet > m_DescriptorSets;

		ShaderWriteMap m_DescriptorWrites;

		uint32_t m_DescriptorSetCount = -1;

		std::vector< VkDescriptorSetLayout > m_SetLayouts;

		Ref< DescriptorPool > m_SetPool;
	};

	class ShaderLibrary : public CountedObj
	{
		SINGLETON( ShaderLibrary );
	public:
		ShaderLibrary();
		~ShaderLibrary();

		void Add( const Ref<Shader>& shader );
		void Load( const std::string& path );
		void Load( const std::string& name, const std::string& path );

		const Ref<Shader>& Find( const std::string& name ) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}
