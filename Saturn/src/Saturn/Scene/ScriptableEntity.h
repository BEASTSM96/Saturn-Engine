#pragma once

#include "Entity.h"

namespace Saturn {

	class ScriptableEntity
	{
	public:
		ScriptableEntity() { }
		virtual ~ScriptableEntity() { }

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Entity.HasComponent<T>();
		}

		template<typename T>
		T& AddComponent()
		{
			return m_Entity.AddComponent<T>();
		}

		template<typename T>
		void RemoveComponent()
		{
			m_Entity.RemoveComponent<T>();
		}

	protected:
		virtual void OnCreate() { SAT_INFO("Super::OnCreate"); }
		virtual void OnDestroy() { SAT_INFO("Super::OnDestroy"); }
		virtual void OnUpdate( Timestep ts ) { }
		virtual void BeginPlay() { SAT_INFO("Super::BeginPlay"); }

	private:
		Entity m_Entity;
		Scene* m_Scene;
		friend class Scene;
		friend class EditorLayer;
		friend class SceneHierarchyPanel;
	};
}