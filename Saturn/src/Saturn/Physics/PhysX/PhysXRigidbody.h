#pragma once

#include "Saturn/Core/Base.h"

#ifdef USE_NVIDIA

#include <physx/PxPhysicsAPI.h>
#include "PhysXScene.h"

namespace Saturn {

	class PhysXRigidbody
	{
	public:
		PhysXRigidbody(PhysXScene* scene, glm::vec3 pos, glm::quat rot);
		~PhysXRigidbody();

		glm::mat4 GetTransform() 
		{
			auto xpos = m_Body->getGlobalPose().p.x; 
			auto ypos = m_Body->getGlobalPose().p.y;
			auto zpos = m_Body->getGlobalPose().p.z;

			auto xq = m_Body->getGlobalPose().q.x;
			auto yq = m_Body->getGlobalPose().q.y;
			auto zq = m_Body->getGlobalPose().q.z;

			auto pos =	glm::mat4(xq * ypos * zpos);
			auto rot =	glm::mat4(xpos * yq * zq);

			return glm::mat4(pos * rot);

		}

		glm::vec3 GetPos() 
		{  
			auto xpos = m_Body->getGlobalPose().p.x;
			auto ypos = m_Body->getGlobalPose().p.y;
			auto zpos = m_Body->getGlobalPose().p.z;

			glm::vec3 pos(xpos * ypos * zpos);

			return  pos;
		}

		glm::quat GetRot() 
		{
			auto xq = m_Body->getGlobalPose().q.x;
			auto yq = m_Body->getGlobalPose().q.y;
			auto zq = m_Body->getGlobalPose().q.z;
			auto wq = m_Body->getGlobalPose().q.w;

			glm::quat q;
			q.x = (xq);
			q.y = (yq);
			q.z = (zq);
			q.w = (wq);

			return q;
		}

	protected:
		PhysXScene* m_Scene;
		physx::PxRigidDynamic* m_Body;
	private:

	};

}

#endif // #ifdef USE_NVIDIA