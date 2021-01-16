#pragma once

#include "Saturn/Core/Base.h"

#ifdef USE_NVIDIA

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <physx/PxPhysicsAPI.h>
#include <string>

#include "PhysXScene.h"

namespace Saturn {

	class PhysXMaterial : public RefCounted
	{
	public:
		PhysXMaterial( PhysXScene* scene, std::string name );
		~PhysXMaterial();

		physx::PxMaterial* m_Material = NULL;

		operator physx::PxMaterial* ( ) { return m_Material; }

	protected:
		PhysXScene* m_Scene;
	private:
	};

}

#endif