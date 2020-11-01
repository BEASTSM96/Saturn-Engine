#include "sppch.h"
#include "Scene.h"

#include "Components.h"

#include <glm/glm.hpp>
#include "Entity.h"
#include "Saturn/Core/Math/Math.h"
#include "Saturn/Core/World/Level.h"

namespace Saturn {
	Scene::Scene()
	{
		SAT_PROFILE_FUNCTION();

		m_data.ID = Random::Float();
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

	void Scene::OnUpdate(Timestep ts)
	{
		SAT_PROFILE_FUNCTION();

		SAT_CORE_ASSERT(m_CurrentLevel->GetGameLayer(), "Error GameLayer is null!");
	}

}