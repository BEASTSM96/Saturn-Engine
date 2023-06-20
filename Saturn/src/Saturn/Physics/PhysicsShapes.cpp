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
#include "PhysicsShapes.h"

#include "PhysicsAuxiliary.h"
#include "PhysicsFoundation.h"

namespace Saturn {

	void PhysicsShape::Detach( physx::PxRigidActor& rActor )
	{
		rActor.detachShape( *m_Shape );
	}

	//////////////////////////////////////////////////////////////////////////

	BoxShape::BoxShape( Entity entity )
		: PhysicsShape( entity )
	{
		m_Type = ShapeType::Box;
	}

	BoxShape::~BoxShape()
	{
		DestroyShape();
	}

	void BoxShape::Create( physx::PxRigidActor& rActor )
	{
		BoxColliderComponent& bcc = m_Entity.GetComponent<BoxColliderComponent>();
		TransformComponent& transform = m_Entity.GetComponent<TransformComponent>();

		glm::vec3 size = bcc.Extents;
		glm::vec3 scale = transform.Scale;

		glm::vec3 halfSize = size / 2.0f;

		physx::PxMaterial* mat = PhysicsFoundation::Get().GetPhysics().createMaterial( 1, 1, 1 );

		physx::PxBoxGeometry BoxGeometry = physx::PxBoxGeometry( halfSize.x, halfSize.y, halfSize.z );
		physx::PxShape* pShape = physx::PxRigidActorExt::createExclusiveShape( rActor, BoxGeometry, *mat );

		pShape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !bcc.IsTrigger );
		pShape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, bcc.IsTrigger );

		pShape->setLocalPose( Auxiliary::GLMTransformToPx( glm::translate( glm::mat4( 1.0f ), bcc.Offset ) ) );

		m_Shape = pShape;

		rActor.attachShape( *m_Shape );
	}

	void BoxShape::DestroyShape()
	{
		PHYSX_TERMINATE_ITEM( m_Shape );
	}

	//////////////////////////////////////////////////////////////////////////

	SphereShape::SphereShape( Entity entity )
		: PhysicsShape( entity )
	{

	}

	SphereShape::~SphereShape()
	{

	}

	void SphereShape::Create( physx::PxRigidActor& rActor )
	{

	}

	void SphereShape::DestroyShape()
	{

	}

	//////////////////////////////////////////////////////////////////////////

	CapusleShape::CapusleShape( Entity entity )
		: PhysicsShape( entity )
	{

	}

	CapusleShape::~CapusleShape()
	{

	}

	void CapusleShape::Create( physx::PxRigidActor& rActor )
	{

	}

	void CapusleShape::DestroyShape()
	{

	}

}