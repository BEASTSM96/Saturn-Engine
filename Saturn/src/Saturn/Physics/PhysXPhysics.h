#pragma once

#include <physx/PxPhysicsAPI.h>

namespace Saturn::Physics {

	class PhysXPhysics
	{
	public:
		PhysXPhysics() = default;
		PhysXPhysics(PhysXPhysics&) = delete;
		virtual ~PhysXPhysics() = default;

		PhysXPhysics& operator=(const PhysXPhysics&) = delete;

		physx::PxFoundation*	GetFoundation();
		physx::PxCooking*		GetCooking();
		physx::PxPhysics*		GetPhysics();

	private:
		physx::PxDefaultErrorCallback	m_DefaultErrorCallback;
		physx::PxDefaultAllocator		m_DefaultAllocator;
		physx::PxTolerancesScale		m_TolerancesScale;
		static physx::PxFoundation*		m_Foundation;
		physx::PxCooking*				m_Cooking = nullptr;
		static physx::PxPhysics*		m_Physics;
		static bool						m_Extensions;
		static size_t					m_Counter;

		bool Initialize();
		void Finalize();
	};

}