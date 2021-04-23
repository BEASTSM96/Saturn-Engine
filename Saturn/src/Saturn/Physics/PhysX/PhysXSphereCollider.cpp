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
#include "PhysXSphereCollider.h"

namespace Saturn {

	PhysXSphereCollider::PhysXSphereCollider( PhysXScene* scene, PhysXRigidbody* body, PhysXMaterial* material, float Radius )
		: PhysXCollider( body, std::vector< physx::PxShape* >(1, scene->GetPhysics().createShape( physx::PxSphereGeometry( Radius ), *material->m_Material, true ) ) )
	{
		m_Body = body;
		m_Scene = scene;
		SetCenter( glm::vec3() );
		SetRadius( Radius );
	}

	PhysXSphereCollider::~PhysXSphereCollider()
	{

	}

	void PhysXSphereCollider::SetCenter( const glm::vec3& center )
	{
		GetShapes()[ 0 ]->setLocalPose( physx::PxTransform( center.x, center.y, center.z ) );
	}

	glm::vec3 PhysXSphereCollider::GetCenter()
	{
		const physx::PxVec3 center = GetShapes()[ 0 ]->getLocalPose().p;
		return glm::vec3( center.x, center.y, center.z );
	}

	void PhysXSphereCollider::SetRadius( float radius )
	{
		GetShapes()[ 0 ]->getGeometry().sphere().radius = radius;
	}

	float& PhysXSphereCollider::GetRadius()
	{
		return GetShapes()[ 0 ]->getGeometry().sphere().radius;
	}

	void PhysXSphereCollider::Scale( const glm::vec3& scale )
	{
		physx::PxShape* shape = GetShapes()[ 0 ];
		const physx::PxVec3 realscale( scale.x, scale.y, scale.z );
		physx::PxTransform pose = shape->getLocalPose();
		physx::PxSphereGeometry& sphere = shape->getGeometry().sphere();
		sphere.radius *= std::pow( std::abs( realscale.x * realscale.y * realscale.z ), 1.0f / 3.0f );
		shape->setGeometry( sphere );
		pose.p = pose.p.multiply( realscale );
		shape->setLocalPose( pose );
	}

}
