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
#include "PhysicsRigidBody.h"

#include "PhysicsFoundation.h"
#include "PhysicsAuxiliary.h"

namespace Saturn {

	PhysicsRigidBody::PhysicsRigidBody( Entity entity )
		: m_Entity( entity )
	{
		TransformComponent& tc = entity.GetComponent<TransformComponent>();
		RigidbodyComponent& rb = entity.GetComponent<RigidbodyComponent>();

		physx::PxRigidDynamic* pBody = PhysicsFoundation::Get().GetPhysics().createRigidDynamic( Auxiliary::GLMTransformToPx( tc.GetTransform() ) );
		m_Actor = pBody;

		SetKinematic( rb.IsKinematic );
		SetMass( rb.Mass );

		physx::PxRigidBodyExt::updateMassAndInertia( *pBody, (physx::PxReal)rb.Mass );
	}

	PhysicsRigidBody::~PhysicsRigidBody()
	{
		Destroy();
	}

	void PhysicsRigidBody::CreateShape()
	{
		RigidbodyComponent& rb = m_Entity.GetComponent<RigidbodyComponent>();

		// Handle shapes
		if( m_Entity.HasComponent<BoxColliderComponent>() )
			AttachPhysicsShape( ShapeType::Box );

		if( m_Entity.HasComponent<SphereColliderComponent>() )
			AttachPhysicsShape( ShapeType::Sphere );

		if( m_Entity.HasComponent<CapsuleColliderComponent>() )
			AttachPhysicsShape( ShapeType::Capusle );

		m_Actor->userData = this;

		// The settings might of changed.
		SetKinematic( rb.IsKinematic );
		SetMass( rb.Mass );
	}

	void PhysicsRigidBody::AttachPhysicsShape( ShapeType type )
	{
		switch( type )
		{
			case Saturn::ShapeType::ConvexMesh:
				break;
			case Saturn::ShapeType::TriangleMesh:
				break;

			case Saturn::ShapeType::Box: 
			{
				m_Shape = Ref<BoxShape>::Create( m_Entity );
			} break;

			case Saturn::ShapeType::Sphere:
			{
				m_Shape = Ref<SphereShape>::Create( m_Entity );
			} break;

			case Saturn::ShapeType::Capusle:
			{
				m_Shape = Ref<CapusleShape>::Create( m_Entity );
			} break;

			case Saturn::ShapeType::Unknown:
			default:
				break;
		}

		m_Shape->Create( *m_Actor );
	}

	void PhysicsRigidBody::Destroy()
	{
		if( m_Shape )
		{
			m_Shape->Detach( *m_Actor );
		}

		m_Shape = nullptr;

		PHYSX_TERMINATE_ITEM( m_Actor );
	}

	void PhysicsRigidBody::SetKinematic( bool val )
	{
		physx::PxRigidDynamic* pBody = ( physx::PxRigidDynamic* ) m_Actor;
		pBody->setRigidBodyFlag( physx::PxRigidBodyFlag::eKINEMATIC, val );

		m_Kinematic = val;
	}

	void PhysicsRigidBody::SetMass( float val )
	{
		physx::PxRigidDynamic* pBody = ( physx::PxRigidDynamic* ) m_Actor;
		pBody->setMass( val );
	}

	glm::vec3 PhysicsRigidBody::GetPosition()
	{
		float xpos = m_Actor->getGlobalPose().p.x;
		float ypos = m_Actor->getGlobalPose().p.y;
		float zpos = m_Actor->getGlobalPose().p.z;

		glm::vec3 pos{};

		pos.x = xpos;
		pos.y = ypos;
		pos.z = zpos;

		return  pos;
	}

	glm::vec3 PhysicsRigidBody::GetRotation()
	{
		auto xq = m_Actor->getGlobalPose().q.x;
		auto yq = m_Actor->getGlobalPose().q.y;
		auto zq = m_Actor->getGlobalPose().q.z;
		auto wq = m_Actor->getGlobalPose().q.w;

		glm::vec3 q = {};
		q.x = xq;
		q.y = yq;
		q.z = zq;

		return q;
	}

	glm::mat4 PhysicsRigidBody::GetTransform()
	{
		auto xpos = m_Actor->getGlobalPose().p.x;
		auto ypos = m_Actor->getGlobalPose().p.y;
		auto zpos = m_Actor->getGlobalPose().p.z;

		auto xq = m_Actor->getGlobalPose().q.x;
		auto yq = m_Actor->getGlobalPose().q.y;
		auto zq = m_Actor->getGlobalPose().q.z;

		auto pos = glm::mat4( xq * ypos * zpos );
		auto rot = glm::mat4( xpos * yq * zq );

		return glm::mat4( pos * rot );
	}

	void PhysicsRigidBody::SyncTransfrom()
	{
		TransformComponent& tc = m_Entity.GetComponent<TransformComponent>();

		physx::PxRigidDynamic* pBody = ( physx::PxRigidDynamic* ) m_Actor;
		physx::PxTransform actorPose = m_Actor->getGlobalPose();

		tc.Position = Auxiliary::PxToGLM( actorPose.p );
		tc.Rotation = glm::eulerAngles( Auxiliary::QPxToGLM( actorPose.q ) );
	}

}