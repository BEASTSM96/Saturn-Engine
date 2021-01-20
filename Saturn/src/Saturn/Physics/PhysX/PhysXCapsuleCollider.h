#pragma once

#include "Saturn/Core/Base.h"

#ifdef USE_NVIDIA

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <physx/PxPhysicsAPI.h>
#include "PhysXCollider.h"
#include "PhysXMaterial.h"
#include "PhysXHelpers.h"

namespace Saturn {

	class PhysXCapsuleCollider : public PhysXCollider, public RefCounted
	{
	public:
		PhysXCapsuleCollider( PhysXScene* scene, PhysXRigidbody* body, PhysXMaterial* material, float Radius, float Height );
		~PhysXCapsuleCollider();

		void SetCenter( const glm::vec3& center );
		glm::vec3 GetCenter();

		void SetRadius( float radius );
		float& GetRadius();

		void Scale( const glm::vec3& scale );

	private:
		float Radius;
		float Height;
	};
}

#endif