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

	class PhysXBoxCollider : public PhysXCollider, public RefCounted
	{
	public:
		PhysXBoxCollider( PhysXScene* scene, PhysXRigidbody* body, PhysXMaterial* material, glm::vec3 Extents );
		~PhysXBoxCollider();

		void SetCenter( const glm::vec3& center );
		glm::vec3 GetCenter();

		void SetSize( const glm::vec3& size );
		glm::vec3 GetSize();

		void Scale( const glm::vec3& scale );

	private:
		PhysXRigidbody* m_Body;
		PhysXScene* m_Scene;
	};

}

#else
#pragma message( "Use USE_NVIDIA if you to use PhysX!" )

#endif