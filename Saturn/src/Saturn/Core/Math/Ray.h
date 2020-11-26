#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Application.h"
#include <glm/glm.hpp>

class GameObject;

namespace Saturn {
	class SATURN_API Raycast
	{
	public:
		//Returns the distance of A and B
		static glm::vec3 GetDistanceFromGameObjectToGameObject(GameObject * From, GameObject* To) {
			return (glm::vec3)glm::distance(From->GetComponent<TransformComponent>().Transform[0], To->GetComponent<TransformComponent>().Transform[0]);
		}

	};

}