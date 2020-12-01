#include "sppch.h"
#include "PhysXRigidbody.h"

namespace Saturn {

	PhysXRigidbody::PhysXRigidbody(PhysXScene* scene, glm::vec3 pos, glm::quat rot) : m_Scene(scene)
	{
		physx::PxVec3 PxPos(pos.x, pos.y, pos.z);
		physx::PxQuat PxQua(rot.x, rot.y, rot.z, rot.w);

		physx::PxTransform Transform( PxPos, PxQua );

		m_Scene->m_Physics->createRigidDynamic( Transform );
	}

	PhysXRigidbody::~PhysXRigidbody()
	{
		m_Scene->m_Physics->release();
	}

}