#pragma once

#include "Core.h"

#include <glm/glm.hpp>

namespace PHYSICNAMESPACE {

	struct Object
	{
		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec3 Force;
		Decimal Mass;
	};
}