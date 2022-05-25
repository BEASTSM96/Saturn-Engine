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

#include "Base.h"
#include <vector>
#include <string>
#include <filesystem>
#include <vulkan.h>
#include <unordered_map>

namespace Saturn {
	
	enum class ShaderType : uint32_t
	{
		Vertex = 0,
		Fragment = 1,
		Geometry = 2,
		Compute = 3,
		All = 4
	};
	
	// A shader uniform buffer represents a uniform buffer variable in a shader.
	struct ShaderUniformBuffer
	{
		std::string Name;
		int Binding;
		size_t Size;
		ShaderType Location;
		
		VkBuffer Buffer;

		std::vector< ShaderUniform > Members;
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

	class Shader;
	class DescriptorPool;

	class ShaderLibrary
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

	extern ShaderType ShaderTypeFromString( std::string Str );
	extern std::string ShaderTypeToString( ShaderType Type );

	class Shader
	{
		using ShaderSourceMap = std::unordered_map< ShaderSourceKey, ShaderSource >;
		using SpvSourceMap = std::unordered_map< ShaderSourceKey, std::vector< uint32_t > >;
		using ShaderUBMap = std::unordered_map< ShaderType, std::unordered_map< uint32_t, ShaderUniformBuffer > >;
		//                                      ShaderType,                     Binding,   UniformBuffer
		
		using ShaderWriteMap = std::unordered_map< ShaderType, std::unordered_map< std::string, VkWriteDescriptorSet > >;

	public:
		Shader() { }
		Shader( std::string Name, std::filesystem::path Filepath );
		Shader( std::filesystem::path Filepath );
		Shader( std::string Filepath );

		~Shader();

		// Moves the uniform for the available uniforms to the uniforms list. So that we can use them. Also removes the uniform from the available uniforms.
		void UseUniform( const std::string& rName ) 
		{
			for( auto& rUniform : m_AvailableUniforms )
			{
				if( rUniform.Name == rName )
				{
					m_Uniforms.push_back( rUniform );
					m_AvailableUniforms.erase( std::remove( m_AvailableUniforms.begin(), m_AvailableUniforms.end(), rUniform ) );
					return;
				}
			}
		}

		// Moves the uniform for the uniforms to the available uniforms list. So that we can't use them. Also removes the uniform from the uniforms list.
		void FreeUniform( ShaderUniform& rUniform ) 
		{
			m_Uniforms.erase( std::remove( m_Uniforms.begin(), m_Uniforms.end(), rUniform ) );
			m_AvailableUniforms.push_back( rUniform );
		}

		// Moves the uniform for the uniforms to the available uniforms list. So that we can't use them. Also removes the uniform from the uniforms list.
		void FreeUniform( const std::string& rName )
		{
			for( auto& rUniform : m_Uniforms )
			{
				if( rUniform.Name == rName )
				{
					m_Uniforms.erase( std::remove( m_Uniforms.begin(), m_Uniforms.end(), rUniform ) );
					m_AvailableUniforms.push_back( rUniform );
					return;
				}
			}
		}
		
		ShaderUniform& FindUniform( const std::string& rName ) 
		{
			for ( auto& rUniform : m_Uniforms )
			{
				if( rUniform.Name == rName )
					return rUniform;
			}

			//return ShaderUniform( "", -1, ShaderDataType::None );
		}

		std::string& GetName() { return m_Name; }
		const std::string& GetName() const { return m_Name; }

		ShaderSourceMap& GetShaderSources() { return m_ShaderSources; }
		const ShaderSourceMap& GetShaderSources() const { return m_ShaderSources; }

		const SpvSourceMap& GetSpvCode() const { return m_SpvCode; }
		SpvSourceMap& GetSpvCode() { return m_SpvCode; }

		std::vector< ShaderUniform > GetUniforms() const { return m_Uniforms; }
		std::vector< ShaderUniform >& GetUniforms() { return m_Uniforms; }
		
		std::vector< ShaderUniform > GetAvailableUniforms() const { return m_AvailableUniforms; }
		std::vector< ShaderUniform >& GetAvailableUniforms() { return m_AvailableUniforms; }
		
		ShaderUBMap& GetUniformBuffers() { return m_UniformBuffers; }
		const ShaderUBMap& GetUniformBuffers() const { return m_UniformBuffers; }
		
		std::vector< std::tuple< ShaderType, uint32_t, std::string > >& GetTextures() { return m_Textures; }
		
		VkDescriptorSetLayout GetSetLayout() const { return m_SetLayout; }
		
		Ref< DescriptorPool >& GetDescriptorPool() { return m_SetPool; }
		const Ref< DescriptorPool >& GetDescriptorPool() const { return m_SetPool; }
		
		ShaderWriteMap& GetWriteDescriptors() { return m_DescriptorWrites; }
		const ShaderWriteMap& GetWriteDescriptors() const { return m_DescriptorWrites; }

		void WriteDescriptor( ShaderType Type, const std::string& rName, VkWriteDescriptorSet& rWriteDescriptor )
		{
			m_DescriptorWrites[ Type ][ rName ] = rWriteDescriptor;
		}

	private:

		void ReadFile();

		void GetAvailableUniform();

		void DetermineShaderTypes();
		
		void Reflect( const std::vector<uint32_t>& rShaderData );
		
		void CompileGlslToSpvAssembly();

	private:
		ShaderSourceMap m_ShaderSources;
		SpvSourceMap m_SpvCode;

		std::string m_FileContents = "";
		size_t m_FileSize;

		std::string m_Name = "";

		std::filesystem::path m_Filepath = "";
		
		std::vector< ShaderUniform > m_AvailableUniforms;
		std::vector< ShaderUniform > m_Uniforms;

		ShaderUBMap m_UniformBuffers;
		// So here we saying that the shader might X amount of textures at X index and X shader stage. It's the materials job to create the textures.
		std::vector< std::tuple< ShaderType, uint32_t, std::string > > m_Textures;
		
		ShaderWriteMap m_DescriptorWrites;

		VkDescriptorSetLayout m_SetLayout;
		Ref< DescriptorPool > m_SetPool;
	};
}
