#include "sppch.h"
#include "PhysXPhysics.h"

namespace Saturn::Physics {

	physx::PxFoundation*	PhysXPhysics::m_Foundation = nullptr;
	physx::PxPhysics*		PhysXPhysics::m_Physics = nullptr;
	bool					PhysXPhysics::m_Extensions = false;
	std::size_t				PhysXPhysics::m_Counter = 0;


	physx::PxFoundation* PhysXPhysics::GetFoundation()
	{
		return m_Foundation;
	}

	physx::PxCooking* PhysXPhysics::GetCooking()
	{
		return m_Cooking;
	}

	physx::PxPhysics* PhysXPhysics::GetPhysics()
	{
		return m_Physics;
	}

	bool PhysXPhysics::Initialize()
	{
		if (m_Foundation == nullptr)
		{
			m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_DefaultAllocator, m_DefaultErrorCallback);
			if (m_Foundation == nullptr)
			{
				m_Foundation = &PxGetFoundation();
				if (m_Foundation == nullptr)
				{
					SAT_CORE_ASSERT(m_Foundation, "Error!, Foundation is null!");
					return false;
				}
			}
		}
		m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, physx::PxCookingParams(m_TolerancesScale));
		if (m_Cooking == nullptr)
		{
			SAT_CORE_ASSERT(m_Cooking, "Error!, Cooking is null!");
			return false;
		}
		if (m_Physics == nullptr)
		{
			m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, m_TolerancesScale);
			if (m_Physics == nullptr)
			{
				SAT_CORE_ASSERT(m_Physics, "Error!, Physics is null!");
				return false;
			}
		}
		if (!m_Extensions)
		{
			m_Extensions = PxInitExtensions(*m_Physics, nullptr);
			if (!m_Extensions)
			{
				SAT_CORE_ASSERT(m_Extensions, "Error!, Extensions is null!");
				return false;
			}
		}
		++m_Counter;
		return true;
	}

	void PhysXPhysics::Finalize()
	{
		if (!--m_Counter)
		{
			if (m_Extensions)
			{
				PxCloseExtensions();
				m_Extensions = false;
			}
			if (m_Physics != nullptr)
			{
				m_Physics->release();
				m_Physics = nullptr;
			}
			if (m_Cooking != nullptr)
			{
				m_Cooking->release();
				m_Cooking = nullptr;
			}
			if (m_Foundation != nullptr)
			{
				m_Foundation->release();
				m_Foundation = nullptr;
			}
		}
	}

}