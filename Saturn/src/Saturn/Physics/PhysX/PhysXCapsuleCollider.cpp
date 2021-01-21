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
