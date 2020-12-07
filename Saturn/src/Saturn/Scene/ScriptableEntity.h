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

	protected:
		Entity* m_Entity;
	private:
		friend class Entity;
	};

}