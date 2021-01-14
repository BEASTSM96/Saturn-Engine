#pragma once

#include "Saturn/Core/Base.h"

#ifdef USE_NVIDIA

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <physx/PxPhysicsAPI.h>
#include "PhysXScene.h"

namespace Saturn {

	class PhysXRigidbody
	{
	public:
		PhysXRigidbody( PhysXScene* scene, glm::vec3 pos, glm::quat rot );
		~PhysXRigidbody();

		glm::mat4 GetTransform()
		{
			auto xpos = m_Body->getGlobalPose().p.x;
			auto ypos = m_Body->getGlobalPose().p.y;
			auto zpos = m_Body->getGlobalPose().p.z;

			auto xq = m_Body->getGlobalPose().q.x;
			auto yq = m_Body->getGlobalPose().q.y;
			auto zq = m_Body->getGlobalPose().q.z;

			auto pos =	glm::mat4( xq * ypos * zpos );
			auto rot =	glm::mat4( xpos * yq * zq );

			return glm::mat4( pos * rot );

		}

		glm::vec3 GetPos();

		glm::quat GetRot();

		void SetKinematic( bool kinematic );

		bool IsKinematic();

	protected:
		PhysXScene* m_Scene;
		physx::PxRigidDynamic* m_Body = NULL;
	private:
		bool m_Kinematic = false;
	};
}

#endif // #ifdef USE_NVIDIA
