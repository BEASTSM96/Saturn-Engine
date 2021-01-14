#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Core/Timestep.h"

#ifdef USE_NVIDIA

#include <physx/PxPhysicsAPI.h>
#include <physx/PxFoundation.h>
#include <physx/PxScene.h>

#include "ErrorCallback.h"

namespace Saturn {
	
	class Scene;

	class PhysXScene : public RefCounted
	{
	public:
		PhysXScene( Scene* scene );
		~PhysXScene();

		void Update( Timestep ts );

		PhysXErrorCallback m_DefaultErrorCallback;
		physx::PxDefaultAllocator m_DefaultAllocatorCallback;
		physx::PxFoundation* m_Foundation = NULL;
		physx::PxDefaultCpuDispatcher* m_Dispatcher = NULL;
		physx::PxCooking* m_Cooking = NULL;
		physx::PxPhysics* m_Physics = NULL;
		physx::PxScene* m_PhysXScene = NULL;

	protected:
		Scene* m_Scene;

	private:
		_declspec( align( 16 ) ) std::uint8_t MemoryBlock[65536];
	};
}

#else
#error You need to use nvidia for physx!
#endif // USE_NVIDIA
