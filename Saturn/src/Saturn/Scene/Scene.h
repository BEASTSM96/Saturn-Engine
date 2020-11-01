#pragma once

#include "entt.hpp"

#include "Saturn/Core.h"
#include "Saturn/Log.h"
#include "Saturn/Core/Timestep.h"
#include "Saturn/Core.h"
#include "Saturn/Debug/Instrumentor.h"

namespace Saturn {


	struct SceneData {

		std::string name;
		float ID;

	};

	class Level;
	class Entity;
	class GameObject;

	class SATURN_API Scene
	{
	public:
		template<class T>
		T* CreateEntityGameObjectprt(std::string name, std::vector<std::string> ShaderPaths, std::string ObjectPath = std::string());

		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());

		GameObject CreateEntityGameObject(const std::string& name = std::string());

		SceneData& GetData() { return m_data; }
		Level& GetLevel() { return *m_CurrentLevel; }

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		SceneData m_data;

		Level* m_CurrentLevel;

		friend class  Entity;

		friend class  GameObject;

		friend class  SceneHierarchyPanel;

	};

	template<class T>
	T* Scene::CreateEntityGameObjectprt(std::string name, std::vector<std::string> ShaderPaths, std::string ObjectPath)
	{
		SAT_PROFILE_FUNCTION();

		T* entity = new T(m_Registry.create(), this);

		entity->AddComponent<TransformComponent>();

		auto& tag = entity->AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned GameObject" : name;

		auto& ID = entity->AddComponent<IdComponent>();

		if (ObjectPath.empty()) {
			std::string name = "assets/meshes/CUBE.fbx";
			ObjectPath = name;
		}

		entity->ourModel = new Model(ObjectPath, ShaderPaths.at(0), ShaderPaths.at(1));
		entity->AddComponent<MeshComponent>(entity->ourModel);

		entity->Init();

		return entity;
	}
}