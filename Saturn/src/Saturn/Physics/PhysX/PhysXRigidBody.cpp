/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#include "sppch.h"
#include "PhysXRigidBody.h"

#include "Saturn/Scene/Entity.h"
#include "PhysXFnd.h"
#include "PhysXRuntime.h"
#include "PhysXHelpers.h"

namespace Saturn {

	PhysXRigidbody::PhysXRigidbody( Entity entity, glm::vec3& pos, glm::quat& rot )
		: m_Entity( entity )
	{

		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();

		auto& trans = entity.GetComponent<TransformComponent>();

		if( trans.Position.y <= -0 )
			trans.Position.y = 0;

		physx::PxVec3 PxPos;
		PxPos.x = trans.Position.x;
		PxPos.y = trans.Position.y;
		PxPos.z = trans.Position.z;

		physx::PxQuat PxQua;
		PxQua.x = rot.x;
		PxQua.y = rot.y;
		PxQua.z = rot.z;
		PxQua.w = rot.w;

		physx::PxTransform PhysXTransform( PxPos, PxQua );

		physx::PxRigidDynamic* actor = PhysXFnd::GetPhysics().createRigidDynamic( glmTransformToPx( trans.GetTransform() ) );

		if( !rb.isKinematic )
			actor->setRigidBodyFlag( physx::PxRigidBodyFlag::eENABLE_CCD, rb.UseCCD );

		physx::PxRigidBodyExt::setMassAndUpdateInertia( *actor, rb.Mass );
		m_Body = actor;
	}

	PhysXRigidbody::~PhysXRigidbody()
	{
		PhysXFnd::GetPhysics().release();
	}

	void PhysXRigidbody::Init()
	{
		auto& rb = m_Entity.GetComponent<PhysXRigidbodyComponent>();

		if( m_Entity.HasComponent<PhysXBoxColliderComponent>() )
			PhysXFnd::CreateBoxCollider( m_Entity, *m_Body );
		if( m_Entity.HasComponent<PhysXSphereColliderComponent>() )
			PhysXFnd::CreateSphereCollider( m_Entity, *m_Body );
		if( m_Entity.HasComponent<PhysXCapsuleColliderComponent>() )
			PhysXFnd::CreateCapsuleCollider( m_Entity, *m_Body );
		if( m_Entity.HasComponent<PhysXMeshColliderComponent>() )
			PhysXFnd::CreateMeshCollider( m_Entity, *m_Body );

		physx::PxAllocatorCallback& allocator = PhysXFnd::GetAllocator();
		physx::PxFilterData filterData;
		filterData.word0 = BIT( 0 );
		filterData.word1 = BIT( 0 );
		const physx::PxU32 numShapes = m_Body->getNbShapes();
		physx::PxShape** shapes = ( physx::PxShape** )allocator.allocate( sizeof( physx::PxShape* ) * numShapes, "", "", 0 );
		m_Body->getShapes( shapes, numShapes );
		for( physx::PxU32 i = 0; i < numShapes; i++ )
			shapes[ i ]->setSimulationFilterData( filterData );
		allocator.deallocate( shapes );
		m_Body->userData = &m_Entity;
		SetKinematic( rb.isKinematic );
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
		physx::PxRigidDynamic* actor = ( physx::PxRigidDynamic* )m_Body;

		actor->setRigidBodyFlag( physx::PxRigidBodyFlag::eKINEMATIC, kinematic );
		m_Kinematic = kinematic;
	}

	bool PhysXRigidbody::IsKinematic()
	{
		physx::PxRigidDynamic* actor = ( physx::PxRigidDynamic* )m_Body;

		return actor->getRigidBodyFlags().isSet( physx::PxRigidBodyFlag::eKINEMATIC );
	}

	void PhysXRigidbody::ApplyForce( glm::vec3 force, ForceType type )
	{
		physx::PxRigidDynamic* actor = ( physx::PxRigidDynamic* )m_Body;

		switch( type )
		{
			case Saturn::ForceType::Force:
				actor->addForce( physx::PxVec3( force.x, force.y, force.z ), physx::PxForceMode::eFORCE );
				SAT_CORE_INFO( "Added Force type: {0}, Force Location X = {1}, Y = {2}, Z = {3}", ForceType::Force, force.x, force.y, force.z );
				break;
			case Saturn::ForceType::Acceleration:
				actor->addForce( physx::PxVec3( force.x, force.y, force.z ), physx::PxForceMode::eACCELERATION );
				SAT_CORE_INFO( "Added Force type: {0}, Force Location X = {1}, Y = {2}, Z = {3}", ForceType::Acceleration, force.x, force.y, force.z );
				break;
			case Saturn::ForceType::Impulse:
				actor->addForce( physx::PxVec3( force.x, force.y, force.z ), physx::PxForceMode::eIMPULSE );
				SAT_CORE_INFO( "Added Force type: {0}, Force Location X = {1}, Y = {2}, Z = {3}", ForceType::Impulse, force.x, force.y, force.z );
				break;
			case Saturn::ForceType::VelocityChange:
				actor->addForce( physx::PxVec3( force.x, force.y, force.z ), physx::PxForceMode::eVELOCITY_CHANGE );
				SAT_CORE_INFO( "Added Force type: {0}, Force Location X = {1}, Y = {2}, Z = {3}", ForceType::VelocityChange, force.x, force.y, force.z );
				break;
			default:
				break;
		}
	}

	bool PhysXRigidbody::AttachShape( physx::PxShape& shape )
	{
		return m_Body->attachShape( shape );
	}

	void PhysXRigidbody::AddActorToScene()
	{
		PhysXRuntime::GetPhysXScene().addActor( *m_Body );
	}

	void PhysXRigidbody::SetUserData( Entity& e )
	{
		m_Body->userData = &e;
	}

	void PhysXRigidbody::UseCCD( bool use )
	{	
	}

	void PhysXRigidbody::SetMass( int mass )
	{
	}

}