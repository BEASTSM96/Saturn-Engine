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
#include "PhysXCollider.h"

namespace Saturn {

	PhysXCollider::PhysXCollider( PhysXRigidbody* body, std::vector<physx::PxShape*> shapes ) : m_Shapes( std::move( shapes ) )
	{
		m_Body = body;

		if( m_Body->m_Scene )
		{
			m_Scene = m_Body->m_Scene;
		}

		for( physx::PxShape* shape : m_Shapes )
		{
			m_Body->m_Body->attachShape( *shape );
			shape->setFlag( physx::PxShapeFlag::eVISUALIZATION, true );
			shape->setFlag( physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true );
			shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, true );
		}

	}

	PhysXCollider::~PhysXCollider()
	{
		for( physx::PxShape* shape : m_Shapes )
		{
			m_Body->m_Body->detachShape( *shape );
		}

		m_Scene->m_PhysXScene->removeActor( *m_Body->m_Body );
		m_Body->m_Body->release();

		m_Shapes.clear();
	}

	std::vector<physx::PxShape*>& PhysXCollider::GetShapes()
	{
		return m_Shapes;
	}

	const std::vector<physx::PxShape*>& PhysXCollider::GetShapes() const
	{
		return m_Shapes;
	}

	void PhysXCollider::SetPosition( glm::vec3 position )
	{

	}

}
