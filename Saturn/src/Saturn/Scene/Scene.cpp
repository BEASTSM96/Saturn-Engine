#include "sppch.h"
#include "Scene.h"

#include "Components.h"

#include <glm/glm.hpp>
#include "Entity.h"
#include "Saturn/Core/Math/Math.h"
#include "Saturn/GameBase/GameObject.h"
#include "Saturn/Core/World/Level.h"

namespace Saturn {

	static void OnTransformConstruct(entt::registry& registry, entt::entity entity)
	{

	}

	Scene::Scene()
	{
		SAT_PROFILE_FUNCTION();

		m_data.name = m_data.name.empty() ? "Scene" : m_data.name;

		/*******************************************************************************************************************/
		m_CurrentLevel = new Level();
		m_CurrentLevel->CreateGameLayer();

		/*******************************************************************************************************************/
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		SAT_PROFILE_FUNCTION();

		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned Entity" : name;

		auto& ID = entity.AddComponent<IdComponent>();

		return entity;
	}

	GameObject Scene::CreateEntityGameObject(const std::string& name)
	{
		SAT_PROFILE_FUNCTION();

		GameObject entity = { m_Registry.create(), this };

		entity.AddComponent<TransformComponent>();
		
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned GameObject" : name;

		auto& ID = entity.AddComponent<IdComponent>();

		return entity;
	}

	GameObject * Scene::CreateEntityGameObjectprt(const std::string& name, const std::vector<std::string> ShaderPaths, std::string ObjectPath)
	{
		SAT_PROFILE_FUNCTION();

		GameObject * entity = new GameObject( m_Registry.create(), this );

		entity->AddComponent<TransformComponent>();

		auto& tag = entity->AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned GameObject" : name;

		auto& ID = entity->AddComponent<IdComponent>();

		if (ObjectPath.empty()) {
			std::string name = "assets/meshes/CUBE.fbx";
			ObjectPath = name;
		}

		entity->Init();

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity.m_EntityHandle);
	}

	void Scene::DestroyGameObject(GameObject entity)
	{
		m_Registry.destroy(entity.m_EntityHandle);
	}

	void Scene::DestroyGameObject(GameObject* entity)
	{
		m_Registry.destroy(entity->m_EntityHandle);
	}

	void Scene::OnUpdate(Timestep ts)
	{
		SAT_PROFILE_FUNCTION();

		SAT_CORE_ASSERT(m_CurrentLevel->GetGameLayer(), "Error GameLayer is null!");
	}


}