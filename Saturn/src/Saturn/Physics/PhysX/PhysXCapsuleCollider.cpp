#include "sppch.h"
#include "PhysXCapsuleCollider.h"

namespace Saturn {

	PhysXCapsuleCollider::PhysXCapsuleCollider( PhysXScene* scene, PhysXRigidbody* body, PhysXMaterial* material, float Radius, float Height )
		: PhysXCollider( body, std::vector<physx::PxShape*>(1, scene->m_Physics->createShape( physx::PxCapsuleGeometry( Radius, Height / 2.0f ), *material->m_Material ) ) )
	{
		m_Body = body;
		m_Scene = scene;
		SetCenter( glm::vec3() );
		SetRadius( Radius );
	}

	PhysXCapsuleCollider::~PhysXCapsuleCollider()
	{

	}

	void PhysXCapsuleCollider::SetCenter( const glm::vec3& center )
	{
		GetShapes()[ 0 ]->setLocalPose( physx::PxTransform( center.x, center.y, center.z ) );
	}

	glm::vec3 PhysXCapsuleCollider::GetCenter()
	{
		const physx::PxVec3 center = GetShapes()[ 0 ]->getLocalPose().p;
		return glm::vec3( center.x, center.y, center.z );
	}

	void PhysXCapsuleCollider::SetRadius( float radius )
	{
		GetShapes()[ 0 ]->getGeometry().sphere().radius = radius;
	}

	float& PhysXCapsuleCollider::GetRadius()
	{
		return GetShapes()[ 0 ]->getGeometry().sphere().radius;
	}

	void PhysXCapsuleCollider::Scale( const glm::vec3& scale )
	{

	}

}
