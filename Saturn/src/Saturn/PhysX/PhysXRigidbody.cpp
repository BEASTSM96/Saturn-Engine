/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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

#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Entity.h"

#include "PhysXFnd.h"
#include "PhysXRuntime.h"

#include "glm/glm.hpp"

namespace Saturn {

	PhysXRigidbody::PhysXRigidbody( Entity entity, glm::vec3& Position, glm::vec3& Rotation )
	{
		m_Entity = entity;

		auto& rb = m_Entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = m_Entity.GetComponent<TransformComponent>();

		physx::PxRigidDynamic* pActor = PhysXFnd::Get().GetPhysics()->createRigidDynamic( glmTransformToPx( trans.GetTransform() ) );

		if( !rb.IsKinematic )
			pActor->setRigidBodyFlag( physx::PxRigidBodyFlag::eENABLE_CCD, rb.UseCCD );

		physx::PxRigidBodyExt::setMassAndUpdateInertia( *pActor, (physx::PxReal)rb.Mass );
		m_Body = pActor;
	}

	PhysXRigidbody::~PhysXRigidbody()
	{
	}

	void PhysXRigidbody::Create()
	{
		auto& rb = m_Entity.GetComponent<PhysXRigidbodyComponent>();
		
		if( m_Entity.HasComponent<PhysXBoxColliderComponent>() )
			PhysXFnd::Get().CreateBoxCollider( m_Entity, *m_Body );

		if( m_Entity.HasComponent<PhysXSphereColliderComponent>() )
			PhysXFnd::Get().CreateSphereCollider( m_Entity, *m_Body );

		if( m_Entity.HasComponent<PhysXCapsuleColliderComponent>() )
			PhysXFnd::Get().CreateCapsuleCollider( m_Entity, *m_Body );

		physx::PxAllocatorCallback& rAllocator = PhysXFnd::Get().GetAllocator();

		// Set filter data
		physx::PxFilterData filterData;

		filterData.word0 = BIT( 0 );
		filterData.word1 = BIT( 0 );

		const physx::PxU32 numShapes = m_Body->getNbShapes();

		physx::PxShape** shapes = ( physx::PxShape** ) rAllocator.allocate( sizeof( physx::PxShape* ) * numShapes, "", "", 0 );

		m_Body->getShapes( shapes, numShapes );

		for( physx::PxU32 i = 0; i < numShapes; i++ )
			shapes[ i ]->setSimulationFilterData( filterData );

		rAllocator.deallocate( shapes );

		m_Body->userData = this;
		SetKinematic( rb.IsKinematic );

		m_LockFlags = rb.LockFlags;
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;
		pActor->setRigidDynamicLockFlags( ( physx::PxRigidDynamicLockFlags ) m_LockFlags );

		SetLinearDrag( rb.LinearDrag );
	}

	void PhysXRigidbody::SetKinematic( bool kinematic )
	{
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;

		pActor->setRigidBodyFlag( physx::PxRigidBodyFlag::eKINEMATIC, kinematic );
		m_Kinematic = kinematic;
	}

	void PhysXRigidbody::ApplyForce( glm::vec3 ForceAmount, ForceMode Type )
	{
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;

		pActor->addForce( GLMToPhysXVec( ForceAmount ), ( physx::PxForceMode::Enum ) Type );
	}

	void PhysXRigidbody::SetUserData( Entity& rEntity )
	{
	}

	void PhysXRigidbody::UseCCD( bool ccd )
	{
	}

	void PhysXRigidbody::SetMass( float mass )
	{
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;
		pActor->setMass( mass );	
	}

	void PhysXRigidbody::Rotate( const glm::vec3& rRotation )
	{
		physx::PxTransform trans = m_Body->getGlobalPose();

		trans.q *= ( physx::PxQuat( glm::radians( rRotation.x ), { 1.0f, 0.0f, 0.0f } )
			* physx::PxQuat( glm::radians( rRotation.y ), { 0.0f, 1.0f, 0.0f } )
			* physx::PxQuat( glm::radians( rRotation.z ), { 0.0f, 0.0f, 1.0f } ) );

		m_Body->setGlobalPose( trans );
	}

	void PhysXRigidbody::AddActorToScene()
	{
		physx::PxScene* pScene = static_cast< physx::PxScene* >( PhysXRuntime::GetScene() );
		pScene->addActor( *m_Body );
	}

	void PhysXRigidbody::SetLinearVelocity( glm::vec3 linearVelocity )
	{
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;
		pActor->setLinearVelocity(  GLMToPhysXVec( linearVelocity ) );
	}

	bool PhysXRigidbody::AttachShape( physx::PxShape& rShape )
	{
		return m_Body->attachShape( rShape );
	}

	void PhysXRigidbody::SetLinearDrag( float value )
	{
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;
		pActor->setLinearDamping( value );
	}

	float PhysXRigidbody::GetLinearDrag()
	{
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;
		return pActor->getLinearDamping();
	}

	glm::vec3 PhysXRigidbody::GetPosition()
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

	glm::vec3 PhysXRigidbody::GetRotation()
	{
		auto xq = m_Body->getGlobalPose().q.x;
		auto yq = m_Body->getGlobalPose().q.y;
		auto zq = m_Body->getGlobalPose().q.z;
		auto wq = m_Body->getGlobalPose().q.w;

		glm::vec3 q = {};
		q.x = xq;
		q.y = yq;
		q.z = zq;

		return q;
	}

	glm::mat4 PhysXRigidbody::GetTransform()
	{
		auto xpos = m_Body->getGlobalPose().p.x;
		auto ypos = m_Body->getGlobalPose().p.y;
		auto zpos = m_Body->getGlobalPose().p.z;

		auto xq = m_Body->getGlobalPose().q.x;
		auto yq = m_Body->getGlobalPose().q.y;
		auto zq = m_Body->getGlobalPose().q.z;

		auto pos = glm::mat4( xq * ypos * zpos );
		auto rot = glm::mat4( xpos * yq * zq );

		return glm::mat4( pos * rot );
	}

	glm::vec3 PhysXRigidbody::GetLinearVelocity()
	{
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;
		return PxVecToGLM( pActor->getLinearVelocity() );
	}

	void PhysXRigidbody::SetLockFlag( LockFlags flags, bool value )
	{
		value ? m_LockFlags |= ( uint32_t ) flags : m_LockFlags &= ~( uint32_t ) flags;

		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;
		pActor->setRigidDynamicLockFlags( ( physx::PxRigidDynamicLockFlags ) m_LockFlags );

		m_Entity.GetComponent<PhysXRigidbodyComponent>().LockFlags = m_LockFlags;
	}

	bool PhysXRigidbody::AllRotationLocked()
	{
		return m_LockFlags & LockFlags::LockRotationX && m_LockFlags & LockFlags::LockRotationY && m_LockFlags & LockFlags::LockRotationZ;
	}

	void PhysXRigidbody::SyncTransfrom()
	{
		TransformComponent& ts = m_Entity.GetComponent<TransformComponent>();
		physx::PxRigidDynamic* pActor = ( physx::PxRigidDynamic* ) m_Body;
		physx::PxTransform actorPose = pActor->getGlobalPose();

		ts.Position = PxVecToGLM( actorPose.p );

		if( !AllRotationLocked() )
			ts.Rotation = glm::eulerAngles( PxQuatToGLM( actorPose.q ) );
	}

}