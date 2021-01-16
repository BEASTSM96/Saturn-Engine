#pragma once

#include "Saturn/Core/Base.h"

#ifdef USE_NVIDIA

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <physx/PxPhysicsAPI.h>

namespace Saturn {

	static glm::vec3 PxVec3ToGLM( physx::PxVec3 vec ) 
	{
		glm::vec3 glmvec3;
		glmvec3.x = vec.x;
		glmvec3.y = vec.y;
		glmvec3.z = vec.z;
		return glmvec3;
	}

	static physx::PxVec3 glmVec3ToPx( glm::vec3 vec )
	{
		physx::PxVec3 pxvec3;
		pxvec3.x = vec.x;
		pxvec3.y = vec.y;
		pxvec3.z = vec.z;
		return pxvec3;
	}

}


#endif