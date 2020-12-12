#pragma once

#include "Entity.h"

namespace Saturn {

	class ScriptableEntity
	{
	public:
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
		virtual void OnCreate() { }
		virtual void OnDestroy() { }
		virtual void OnUpdate( Timestep ts ) { }
		virtual void BeginPlay() { }
	private:
		Entity m_Entity;
		friend class Scene;
	};
}