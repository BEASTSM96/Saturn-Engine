#include "sppch.h"
#include "PhysXRigidBody.h"

namespace Saturn {

	PhysXRigidbody::PhysXRigidbody( PhysXScene* scene, glm::vec3 pos, glm::quat rot ) : m_Scene( scene )
	{
		physx::PxVec3 PxPos;
		PxPos.x = pos.x;
		PxPos.y = pos.y;
		PxPos.z = pos.z;

		physx::PxQuat PxQua;
		PxQua.x = rot.x;
		PxQua.y = rot.y;
		PxQua.z = rot.z;
		PxQua.w = rot.w;

		physx::PxTransform PhysXTransform( PxPos, PxQua );

		m_Body = m_Scene->m_Physics->createRigidDynamic( PhysXTransform );
		m_Body->userData = this;
		m_Body->setActorFlag( physx::PxActorFlag::eVISUALIZATION, true );
		m_Scene->m_PhysXScene->addActor( *m_Body );

	}

	PhysXRigidbody::~PhysXRigidbody()
	{
		m_Scene->m_Physics->release();
	}

	glm::vec3 PhysXRigidbody::GetPos()
	{
		float xpos = m_Body->getGlobalPose().p.x;
		float ypos = m_Body->getGlobalPose().p.y;
		float zpos = m_Body->getGlobalPose().p.z;

		glm::vec3 pos;

		pos.x = xpos;
		pos.y = ypos;
		pos.z = zpos;

		return  pos;
	}

	glm::quat PhysXRigidbody::GetRot()
	{
		auto xq = m_Body->getGlobalPose().q.x;
		auto yq = m_Body->getGlobalPose().q.y;
		auto zq = m_Body->getGlobalPose().q.z;
		auto wq = m_Body->getGlobalPose().q.w;

		glm::quat q;
		q.x = xq;
		q.y = yq;
		q.z = zq;
		q.w = wq;

		return q;
	}

	void PhysXRigidbody::SetKinematic( bool kinematic )
	{
		m_Body->setRigidBodyFlag( physx::PxRigidBodyFlag::eKINEMATIC, kinematic );
		m_Kinematic = kinematic;

		if( kinematic )
		{
			const glm::vec3 position = GetPos();
			const glm::quat rotation = GetRot();

			m_Body->setKinematicTarget( physx::PxTransform( position.x, position.y, position.z, physx::PxQuat( rotation.x, rotation.y, rotation.z, rotation.w ) ) );
		}
	}

	bool PhysXRigidbody::IsKinematic()
	{
		return m_Body->getRigidBodyFlags().isSet( physx::PxRigidBodyFlag::eKINEMATIC );
	}

	void PhysXRigidbody::ApplyForce( glm::vec3 force, ForceType type )
	{
		switch( type )
		{
			case Saturn::ForceType::Force:
				m_Body->addForce( physx::PxVec3( force.x, force.y, force.z ), physx::PxForceMode::eFORCE );
				SAT_CORE_INFO( "Added Force type: {0}, Force Location X = {1}, Y = {2}, Z = {3}", ForceType::Force, force.x, force.y, force.z );
				break;
			case Saturn::ForceType::Acceleration:
				m_Body->addForce( physx::PxVec3( force.x, force.y, force.z ), physx::PxForceMode::eACCELERATION );
				SAT_CORE_INFO( "Added Force type: {0}, Force Location X = {1}, Y = {2}, Z = {3}", ForceType::Acceleration, force.x, force.y, force.z );
				break;
			case Saturn::ForceType::Impulse:
				m_Body->addForce( physx::PxVec3( force.x, force.y, force.z ), physx::PxForceMode::eIMPULSE );
				SAT_CORE_INFO( "Added Force type: {0}, Force Location X = {1}, Y = {2}, Z = {3}", ForceType::Impulse, force.x, force.y, force.z );
				break;
			case Saturn::ForceType::VelocityChange:
				m_Body->addForce( physx::PxVec3( force.x, force.y, force.z ), physx::PxForceMode::eVELOCITY_CHANGE );
				SAT_CORE_INFO( "Added Force type: {0}, Force Location X = {1}, Y = {2}, Z = {3}", ForceType::VelocityChange, force.x, force.y, force.z );
				break;
			default:
				break;
		}
	}

	void PhysXRigidbody::AttachShape( physx::PxShape& shape )
	{
		m_Body->attachShape( shape );
	}

}