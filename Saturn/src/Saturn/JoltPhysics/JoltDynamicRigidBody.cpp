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
#include "JoltDynamicRigidBody.h"

#include "Saturn/Core/OptickProfiler.h"

#include "JoltPhysicsFoundation.h"
#include "JoltConversions.h"

namespace Saturn {

	JoltDynamicRigidBody::JoltDynamicRigidBody( Entity entity )
		: JoltPhysicsBodyBase()
	{
		m_Entity = entity;
	}

	void JoltDynamicRigidBody::Create( const glm::vec3& Position, const glm::vec3& Rotation )
	{
		JPH::Body* newBody = nullptr;

		switch( PendingShapeInfo.ShapeType )
		{
			case PhysicsShape::BOX:
			{
				JPH::Body* newBody = JoltPhysicsFoundation::Get().CreateBoxCollider( Position, PendingShapeInfo.Scale, m_Kinematic );
				SetBody( newBody );
			} break;

			case PhysicsShape::CAPSULE:
			{

			} break;

			case PhysicsShape::SPHERE:
			{

			} break;
		}
	}

	JoltDynamicRigidBody::~JoltDynamicRigidBody()
	{
		DestoryBody();
	}

	void JoltDynamicRigidBody::DestoryBody()
	{
		if( m_Body )
		{
			JoltPhysicsFoundation::Get().DestoryBody( m_Body );
			m_Body = nullptr;
		}
	}

	void JoltDynamicRigidBody::SetBody( JPH::Body* body )
	{
		SAT_CORE_WARN( "Physics body changing!" );
		m_Body = body;
	}

	void JoltDynamicRigidBody::SetKinematic( bool kinematic )
	{
		m_Kinematic = kinematic;
	}

	void JoltDynamicRigidBody::ApplyForce( glm::vec3 ForceAmount, ForceMode Type )
	{
		m_Body->AddForce( Auxiliary::GLMToJPH( ForceAmount ) );
	}

	void JoltDynamicRigidBody::SetUserData( Entity& rEntity )
	{
	}

	void JoltDynamicRigidBody::UseCCD( bool ccd )
	{
	}

	void JoltDynamicRigidBody::SetMass( float mass )
	{
	}

	void JoltDynamicRigidBody::Rotate( const glm::vec3& rRotation )
	{
	}

	void JoltDynamicRigidBody::SetLinearVelocity( glm::vec3 linearVelocity )
	{

	}

	void JoltDynamicRigidBody::SetLinearDrag( float value )
	{
	}

	void JoltDynamicRigidBody::AttachShape( PhysicsShape shapeType, const glm::vec3& Scale /*= glm::vec3(0.0f) */ )
	{
		PendingShapeInfo.Scale = Scale;
		PendingShapeInfo.ShapeType = shapeType;
	}

	void JoltDynamicRigidBody::SyncTransform()
	{
		SAT_PF_EVENT();

		auto& tc = m_Entity.GetComponent<TransformComponent>();
		tc.Position = Auxiliary::JPHToGLM( m_Body->GetCenterOfMassPosition() );
	}

}