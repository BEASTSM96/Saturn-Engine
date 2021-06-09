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
#include "Saturn/Core/Ref.h"
#include "Saturn/Core/Buffer.h"

#include "Saturn/Renderer/RendererAPI.h"
#include "Saturn/Renderer/ShaderUniform.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	enum class UniformType
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Matrix3x3, Matrix4x4,
		Int32, Uint32
	};

	struct UniformDecl
	{
		UniformType Type;
		std::ptrdiff_t Offset;
		std::string Name;
	};

	struct UniformBuffer
	{
		byte* Buffer;
		std::vector<UniformDecl> Uniforms;
	};

	struct UniformBufferBase
	{
		virtual const byte* GetBuffer( void )  const = 0;
		virtual const UniformDecl* GetUniforms( void )  const = 0;
		virtual unsigned int GetUniformCount( void )  const = 0;
	};

	template<unsigned int N, unsigned int U>
	struct UniformBufferDeclaration : public UniformBufferBase
	{
		byte Buffer[ N ];
		UniformDecl Uniforms[ U ];
		std::ptrdiff_t Cursor = 0;
		int Index = 0;

		virtual const byte* GetBuffer( void )  const override { return Buffer; }
		virtual const UniformDecl* GetUniforms( void ) const override { return Uniforms; }
		virtual unsigned int GetUniformCount( void )  const { return U; }


		template<typename T>
		void Push( const std::string& name, const T& data ) { }

	#ifdef SAT_PLATFORM_WINDOWS
		template<>
		void Push( const std::string& name, const float& data )
		{
			Uniforms[ Index++ ] ={ UniformType::Float, Cursor, name };
			memcpy( Buffer + Cursor, &data, sizeof( float ) );
			Cursor += sizeof( float );
		}

		template<>
		void Push( const std::string& name, const glm::vec3& data )
		{
			Uniforms[ Index++ ] ={ UniformType::Float3, Cursor, name };
			memcpy( Buffer + Cursor, glm::value_ptr( data ), sizeof( glm::vec3 ) );
			Cursor += sizeof( glm::vec3 );
		}

		template<>
		void Push( const std::string& name, const glm::vec4& data )
		{
			Uniforms[ Index++ ] ={ UniformType::Float4, Cursor, name };
			memcpy( Buffer + Cursor, glm::value_ptr( data ), sizeof( glm::vec4 ) );
			Cursor += sizeof( glm::vec4 );
		}

		template<>
		void Push( const std::string& name, const glm::mat4& data )
		{
			Uniforms[ Index++ ] ={ UniformType::Matrix4x4, Cursor, name };
			memcpy( Buffer + Cursor, glm::value_ptr( data ), sizeof( glm::mat4 ) );
			Cursor += sizeof( glm::mat4 );
		}
	#endif

	#ifdef SAT_PLATFORM_LINUX

		void Push( const std::string& name, const float& data )
		{
			Uniforms[ Index++ ] ={ UniformType::Float, Cursor, name };
			memcpy( Buffer + Cursor, &data, sizeof( float ) );
			Cursor += sizeof( float );
		}

		void Push( const std::string& name, const glm::vec3& data )
		{
			Uniforms[ Index++ ] ={ UniformType::Float3, Cursor, name };
			memcpy( Buffer + Cursor, glm::value_ptr( data ), sizeof( glm::vec3 ) );
			Cursor += sizeof( glm::vec3 );
		}

		void Push( const std::string& name, const glm::vec4& data )
		{
			Uniforms[ Index++ ] ={ UniformType::Float4, Cursor, name };
			memcpy( Buffer + Cursor, glm::value_ptr( data ), sizeof( glm::vec4 ) );
			Cursor += sizeof( glm::vec4 );
		}

		void Push( const std::string& name, const glm::mat4& data )
		{
			Uniforms[ Index++ ] ={ UniformType::Matrix4x4, Cursor, name };
			memcpy( Buffer + Cursor, glm::value_ptr( data ), sizeof( glm::mat4 ) );
			Cursor += sizeof( glm::mat4 );
		}

	#endif
	};


	class SATURN_API Shader : public RefCounted
	{
	public:
		using ShaderReloadedCallback = std::function<void()>;

		virtual void Reload( void )  = 0;

		virtual void Bind( void )  = 0;
		virtual RendererID GetRendererID() const = 0;
		virtual void UploadUniformBuffer( const UniformBufferBase& uniformBuffer ) = 0;

		// Temporary while we don't have materials
		virtual void SetFloat( const std::string& name, float value ) = 0;
		virtual void SetInt( const std::string& name, int value ) = 0;
		virtual void SetFloat3( const std::string& name, const glm::vec3& value ) = 0;
		virtual void SetMat4( const std::string& name, const glm::mat4& value ) = 0;
		virtual void SetMat4FromRenderThread( const std::string& name, const glm::mat4& value, bool bind = true ) = 0;

		virtual void SetIntArray( const std::string& name, int* values, uint32_t size ) = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create( const std::string& filepath );
		static Ref<Shader> CreateFromString( const std::string& source );

		virtual void SetVSMaterialUniformBuffer( Buffer buffer ) = 0;
		virtual void SetPSMaterialUniformBuffer( Buffer buffer ) = 0;

		virtual const ShaderUniformBufferList& GetVSRendererUniforms() const = 0;
		virtual const ShaderUniformBufferList& GetPSRendererUniforms() const = 0;
		virtual bool HasVSMaterialUniformBuffer() const = 0;
		virtual bool HasPSMaterialUniformBuffer() const = 0;
		virtual const ShaderUniformBufferDeclaration& GetVSMaterialUniformBuffer() const = 0;
		virtual const ShaderUniformBufferDeclaration& GetPSMaterialUniformBuffer() const = 0;

		virtual const ShaderResourceList& GetResources() const = 0;

		virtual void AddShaderReloadedCallback( const ShaderReloadedCallback& callback ) = 0;

		static std::vector<Ref<Shader>> s_AllShaders;
	};

	class ShaderLibrary : public RefCounted
	{
	public:
		ShaderLibrary();
		~ShaderLibrary();

		void Add( const Ref<Shader>& shader );
		void Load( const std::string& path );
		void Load( const std::string& name, const std::string& path );
		void Clear();

		const Ref<Shader>& Get( const std::string& name ) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}