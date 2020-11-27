#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Log.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <unordered_set>

namespace Saturn {

	enum class MaterialFlag
	{
		None = BIT(0),
		DepthTest = BIT(1),
		Blend = BIT(2)
	};

	class Material : public RefCounted
	{
		friend class MaterialInstance;
	public:
		Material(const Ref<Shader>& shader);
		virtual ~Material();

		void Bind();

		uint32_t GetFlags() const { return m_MaterialFlags; }
		void SetFlag(MaterialFlag flag) { m_MaterialFlags |= (uint32_t)flag; }

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = FindUniformDeclaration(name);
			SAT_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = GetUniformBufferTarget(decl);
			buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());

			for (auto mi : m_MaterialInstances)
				mi->OnMaterialValueUpdated(decl);
		}

		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto decl = FindResourceDeclaration(name);
			uint32_t slot = decl->GetRegister();
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		template<typename T>
		T& Get(const std::string& name)
		{
			auto decl = FindUniformDeclaration(name);
			SAT_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = GetUniformBufferTarget(decl);
			return buffer.Read<T>(decl->GetOffset());
		}

		template<typename T>
		Ref<T> GetResource(const std::string& name)
		{
			auto decl = FindResourceDeclaration(name);
			uint32_t slot = decl->GetRegister();
			SAT_CORE_ASSERT(slot < m_Textures.size(), "Texture slot is invalid!");
			return m_Textures[slot];
		}
	public:
		static Ref<Material> Create(const Ref<Shader>& shader);
	private:
		void AllocateStorage();
		void OnShaderReloaded();
		void BindTextures();

		ShaderUniformDeclaration* FindUniformDeclaration(const std::string& name);
		ShaderResourceDeclaration* FindResourceDeclaration(const std::string& name);
		Buffer& GetUniformBufferTarget(ShaderUniformDeclaration* uniformDeclaration);
	private:
		Ref<Shader> m_Shader;
		std::unordered_set<MaterialInstance*> m_MaterialInstances;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::vector<Ref<Texture>> m_Textures;

		uint32_t m_MaterialFlags;
	};

	class MaterialInstance : public RefCounted
	{
		friend class Material;
	public:
		MaterialInstance(const Ref<Material>& material, const std::string& name = "");
		virtual ~MaterialInstance();

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = m_Material->FindUniformDeclaration(name);
			if (!decl)
				return;
			SAT_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = GetUniformBufferTarget(decl);
			buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());

			m_OverriddenValues.insert(name);
		}

		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto decl = m_Material->FindResourceDeclaration(name);
			if (!decl)
			{
				//SAT_CORE_WARN("Cannot find material property: ", name);
				return;
			}
			uint32_t slot = decl->GetRegister();
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		template<typename T>
		T& Get(const std::string& name)
		{
			auto decl = m_Material->FindUniformDeclaration(name);
			SAT_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = GetUniformBufferTarget(decl);
			return buffer.Read<T>(decl->GetOffset());
		}

		template<typename T>
		Ref<T> GetResource(const std::string& name)
		{
			auto decl = m_Material->FindResourceDeclaration(name);
			SAT_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			uint32_t slot = decl->GetRegister();
			SAT_CORE_ASSERT(slot < m_Textures.size(), "Texture slot is invalid!");
			return Ref<T>(m_Textures[slot]);
		}

		template<typename T>
		Ref<T> TryGetResource(const std::string& name)
		{
			auto decl = m_Material->FindResourceDeclaration(name);
			if (!decl)
				return nullptr;

			uint32_t slot = decl->GetRegister();
			if (slot >= m_Textures.size())
				return nullptr;

			return Ref<T>(m_Textures[slot]);
		}


		void Bind();

		uint32_t GetFlags() const { return m_Material->m_MaterialFlags; }
		bool GetFlag(MaterialFlag flag) const { return (uint32_t)flag & m_Material->m_MaterialFlags; }
		void SetFlag(MaterialFlag flag, bool value = true);

		Ref<Shader> GetShader() { return m_Material->m_Shader; }

		const std::string& GetName() const { return m_Name; }
	public:
		static Ref<MaterialInstance> Create(const Ref<Material>& material);
	private:
		void AllocateStorage();
		void OnShaderReloaded();
		Buffer& GetUniformBufferTarget(ShaderUniformDeclaration* uniformDeclaration);
		void OnMaterialValueUpdated(ShaderUniformDeclaration* decl);
	private:
		Ref<Material> m_Material;
		std::string m_Name;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::vector<Ref<Texture>> m_Textures;

		// TODO: This is temporary; come up with a proper system to track overrides
		std::unordered_set<std::string> m_OverriddenValues;
	};
}