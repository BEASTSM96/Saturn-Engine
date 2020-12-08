#pragma once

#include "Entity.h"

namespace Saturn {

	class ScriptableEntity
	{
	public:

		template<typename T>
		T& GetComponent() 
		{
			return m_Entity->GetComponent<T>();
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Entity->HasComponent<T>();
		}

		template<typename T>
		void RemoveComponent()
		{
			m_Entity->RemoveComponent<T>();
		}
		
		template<typename T, typename... Args>
		T& AddComponent( Args&&... args )
		{
			return m_Entity->AddComponent<T>();
		}

	protected:
		Entity* m_Entity;
	private:
		friend class Entity;
	};

}