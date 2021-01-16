#pragma once

#include "Saturn/Core/Base.h"

#ifdef USE_NVIDIA

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <physx/PxPhysicsAPI.h>
#include "PhysXRigidBody.h"

namespace Saturn {

	class PhysXCollider : public RefCounted
	{
	public:
		PhysXCollider( PhysXRigidbody* body, std::vector<physx::PxShape*> shapes );
		~PhysXCollider();

		void SetPosition( glm::vec3 position );

		std::vector<physx::PxShape*>& GetShapes();
		const std::vector<physx::PxShape*>& GetShapes() const;
	protected:
		std::vector<physx::PxShape*> m_Shapes;
		PhysXRigidbody* m_Body;
		PhysXScene* m_Scene;

	private:

	};
}


#endif