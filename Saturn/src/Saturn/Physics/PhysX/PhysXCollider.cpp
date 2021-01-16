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
