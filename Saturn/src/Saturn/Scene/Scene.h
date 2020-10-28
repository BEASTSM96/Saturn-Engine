#pragma once

#include "entt.hpp"

#include "Saturn/Core.h"
#include "Saturn/Log.h"
#include "Saturn/Core/Timestep.h"
#include "Saturn/Core.h"
#include "Saturn/Renderer/Material.h"
#include "Saturn/Renderer/3D/Mesh/Mesh.h"

namespace Saturn {

	struct SceneData {
		UUID SceneID;
		std::string name;
		RefSR<FTexture> m_SkyboxTexture;
		RefSR<Material> m_SkyboxMaterial;
	};

	struct Light
	{
		glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };

		float Multiplier = 1.0f;
	};

	class Level;
	class Entity;
	class GameObject;

	class SATURN_API Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		GameObject CreateEntityGameObject(const std::string& name = std::string());
		GameObject * CreateEntityGameObjectprt(const std::string& name, const std::vector<std::string> ShaderPaths, std::string ObjectPath = std::string());
		void DestroyEntity(Entity entity);
		void DestroyGameObject(GameObject entity);
		void DestroyGameObject(GameObject * entity);

		template<typename T>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<T>();
		}

		SceneData& GetData() { return m_data; }
		Level& GetLevel() { return *m_CurrentLevel; }
		entt::registry& GetRegistry() { return m_Registry; }

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		Light m_Light;
		float m_LightMultiplier = 0.3f;

		SceneData m_data;

		Level* m_CurrentLevel;

		friend class  Entity;
		friend class  GameObject;
		friend class  SceneHierarchyPanel;

	};
}