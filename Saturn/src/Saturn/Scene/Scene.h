#pragma once

#include "entt.hpp"

#include "Saturn/Core.h"
#include "Saturn/Log.h"
#include "Saturn/Core/Timestep.h"
#include "Saturn/Core.h"


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
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());

		GameObject CreateEntityGameObject(const std::string& name = std::string());

		GameObject * CreateEntityGameObjectprt(const std::string& name, const std::vector<std::string> ShaderPaths, std::string ObjectPath = std::string());
		SceneData& GetData() { return m_data; }
		Level& GetLevel() { return *m_CurrentLevel; }
		entt::registry& GetRegistry() { return m_Registry; }

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
}