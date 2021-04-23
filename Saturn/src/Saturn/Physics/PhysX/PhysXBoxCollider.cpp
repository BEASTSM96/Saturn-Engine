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
#include "PhysXBoxCollider.h"

namespace Saturn {

	PhysXBoxCollider::PhysXBoxCollider( PhysXScene* scene, PhysXRigidbody* body, PhysXMaterial* material, glm::vec3 Extents ) 
		: PhysXCollider( 
			body, 
			std::vector<physx::PxShape*>(
			1, scene->GetPhysics().createShape(
			physx::PxBoxGeometry( physx::PxVec3( Extents.x, Extents.y, Extents.z ) ),
			*material->m_Material, 
			true
		))
		)
	{
		m_Body = body;
		m_Scene = scene;
		SetCenter( glm::vec3() );
	}

	PhysXBoxCollider::~PhysXBoxCollider()
	{
	
	}

	void PhysXBoxCollider::SetCenter( const glm::vec3& center )
	{
		GetShapes()[ 0 ]->setLocalPose( physx::PxTransform( center.x, center.y, center.z ) );
	}

	glm::vec3 PhysXBoxCollider::GetCenter()
	{
		const physx::PxVec3 center = GetShapes()[ 0 ]->getLocalPose().p;
		return glm::vec3( center.x, center.y, center.z );
	}

	void PhysXBoxCollider::SetSize( const glm::vec3& size )
	{
		GetShapes()[ 0 ]->getGeometry().box().halfExtents = physx::PxVec3( size.x / 2.0f, size.y / 2.0f, size.z / 2.0f );
	}

	glm::vec3 PhysXBoxCollider::GetSize()
	{
		const physx::PxVec3 size = 2.0f * GetShapes()[ 0 ]->getGeometry().box().halfExtents;
		return glm::vec3( size.x, size.y, size.z );
	}

	void PhysXBoxCollider::Scale( const glm::vec3& scale )
	{
		physx::PxShape* shape = GetShapes()[ 0 ];
		const physx::PxVec3 realscale( scale.x, scale.y, scale.z );
		physx::PxTransform pose = shape->getLocalPose();
		const physx::PxMat33 PxMatrix = physx::PxMat33::createDiagonal( realscale ) * physx::PxMat33( pose.q );
		physx::PxBoxGeometry& box = shape->getGeometry().box();
		box.halfExtents.x *= PxMatrix.column0.magnitude();
		box.halfExtents.y *= PxMatrix.column1.magnitude();
		box.halfExtents.z *= PxMatrix.column2.magnitude();
		shape->setGeometry( box );
		pose.p = pose.p.multiply( realscale );
		shape->setLocalPose( pose );
	}

}
