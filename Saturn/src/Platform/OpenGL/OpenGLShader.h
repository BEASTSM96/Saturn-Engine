#pragma once

#include "Saturn/Renderer/Shader.h"
#include <glad/glad.h>
#include "OpenGLShaderUniform.h"

namespace Saturn {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader() = default;
		OpenGLShader(const std::string& filepath);
		static Ref<OpenGLShader> CreateFromString(const std::string& source);

		virtual void Reload() override;
		virtual void AddShaderReloadedCallback(const ShaderReloadedCallback& callback) override;

		virtual void Bind() override;
		virtual RendererID GetRendererID() const override { return m_RendererID; }

		virtual void UploadUniformBuffer(const UniformBufferBase& uniformBuffer) override;

		virtual void SetVSMaterialUniformBuffer(Buffer buffer) override;
		virtual void SetPSMaterialUniformBuffer(Buffer buffer) override;

		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;
		virtual void SetMat4FromRenderThread(const std::string& name, const glm::mat4& value, bool bind = true) override;

		virtual void SetIntArray(const std::string& name, int* values, u32 size) override;

		virtual const std::string& GetName() const override { return m_Name; }
	private:
		void Load(const std::string& source);

		std::string ReadShaderFromFile(const std::string& filepath) const;
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Parse();
		void ParseUniform(const std::string& statement, ShaderDomain domain);
		void ParseUniformStruct(const std::string& block, ShaderDomain domain);
		ShaderStruct* FindStruct(const std::string& name);

		int32_t GetUniformLocation(const std::string& name) const;

		void ResolveUniforms();
		void ValidateUniforms();
		void CompileAndUploadShader();
		static GLenum ShaderTypeFromString(const std::string& type);

		void ResolveAndSetUniforms(const Ref<OpenGLShaderUniformBufferDeclaration>& decl, Buffer buffer);
		void ResolveAndSetUniform(OpenGLShaderUniformDeclaration* uniform, Buffer buffer);
		void ResolveAndSetUniformArray(OpenGLShaderUniformDeclaration* uniform, Buffer buffer);
		void ResolveAndSetUniformField(const OpenGLShaderUniformDeclaration& field, byte* data, int32_t offset);

		void UploadUniformInt(u32 location, int32_t value);
		void UploadUniformIntArray(u32 location, int32_t* values, int32_t count);
		void UploadUniformFloat(u32 location, float value);
		void UploadUniformFloat2(u32 location, const glm::vec2& value);
		void UploadUniformFloat3(u32 location, const glm::vec3& value);
		void UploadUniformFloat4(u32 location, const glm::vec4& value);
		void UploadUniformMat3(u32 location, const glm::mat3& values);
		void UploadUniformMat4(u32 location, const glm::mat4& values);
		void UploadUniformMat4Array(u32 location, const glm::mat4& values, u32 count);

		void UploadUniformStruct(OpenGLShaderUniformDeclaration* uniform, byte* buffer, u32 offset);

		void UploadUniformInt(const std::string& name, int32_t value);
		void UploadUniformIntArray(const std::string& name, int32_t* values, u32 count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void UploadUniformMat4(const std::string& name, const glm::mat4& value);

		virtual const ShaderUniformBufferList& GetVSRendererUniforms() const override { return m_VSRendererUniformBuffers; }
		virtual const ShaderUniformBufferList& GetPSRendererUniforms() const override { return m_PSRendererUniformBuffers; }
		virtual bool HasVSMaterialUniformBuffer() const override { return (bool)m_VSMaterialUniformBuffer; }
		virtual bool HasPSMaterialUniformBuffer() const override { return (bool)m_PSMaterialUniformBuffer; }
		virtual const ShaderUniformBufferDeclaration& GetVSMaterialUniformBuffer() const override { return *m_VSMaterialUniformBuffer; }
		virtual const ShaderUniformBufferDeclaration& GetPSMaterialUniformBuffer() const override { return *m_PSMaterialUniformBuffer; }
		virtual const ShaderResourceList& GetResources() const override { return m_Resources; }
	private:
		RendererID m_RendererID = 0;
		bool m_Loaded = false;
		bool m_IsCompute = false;

		std::string m_Name, m_AssetPath;
		std::unordered_map<GLenum, std::string> m_ShaderSource;

		std::vector<ShaderReloadedCallback> m_ShaderReloadedCallbacks;

		ShaderUniformBufferList m_VSRendererUniformBuffers;
		ShaderUniformBufferList m_PSRendererUniformBuffers;
		Ref<OpenGLShaderUniformBufferDeclaration> m_VSMaterialUniformBuffer;
		Ref<OpenGLShaderUniformBufferDeclaration> m_PSMaterialUniformBuffer;
		ShaderResourceList m_Resources;
		ShaderStructList m_Structs;

	};
}