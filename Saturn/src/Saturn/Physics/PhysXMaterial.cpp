#include "sppch.h"
#include "PhysXMaterial.h"
#include "PhysXWorld.h"

namespace Saturn::Physics {

	PhysXMaterial::PhysXMaterial(PhysXWorld* world, const std::string& name) : 
		m_PxMaterial(static_cast<PhysXPhysics*>(world->GetPhysics())->GetPhysics()->CreateMaterial(GetDefaultStaticFriction(), GetDefaultDynamicFriction(), GetDefaultRestitution()))
	{
		SAT_CORE_ASSERT(m_PxMaterial, "Error! PxMaterial null!");
	}

	PhysXMaterial::~PhysXMaterial()
	{
		if (m_PxMaterial != nullptr)
		{
			m_PxMaterial->release();
			m_PxMaterial = nullptr;
		}
	}


	physx::PxMaterial* PhysXMaterial::GetMaterial()
	{
		return m_PxMaterial;
	}

	const physx::PxMaterial* PhysXMaterial::GetMaterial() const
	{
		return m_PxMaterial;
	}

	void PhysXMaterial::SetStaticFriction(float staticFriction)
	{
		m_PxMaterial->setStaticFriction(staticFriction);
	}

	float PhysXMaterial::GetStaticFriction() const
	{
		return m_PxMaterial->getStaticFriction();
	}

	void PhysXMaterial::SetDynamicFriction(float dynamicFriction)
	{
		m_PxMaterial->setDynamicFriction(dynamicFriction);
	}

	float PhysXMaterial::GetDynamicFriction() const
	{
		return m_PxMaterial->getStaticFriction();
	}

	void PhysXMaterial::SetRestitution(float restitution)
	{
		m_PxMaterial->setRestitution(restitution);
	}

	float PhysXMaterial::GetRestitution(void) const
	{
		return m_PxMaterial->getStaticFriction();
	}

	float PhysXMaterial::GetDefaultStaticFriction()
	{
		return 0.5f;
	}

	float PhysXMaterial::GetDefaultDynamicFriction()
	{
		return 0.5f;
	}

	float PhysXMaterial::GetDefaultRestitution()
	{
		return 0.1f;
	}

}