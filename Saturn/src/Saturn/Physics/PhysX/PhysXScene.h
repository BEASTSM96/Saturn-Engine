#pragma once

#include "Saturn/Core/Base.h"

#ifdef USE_NVIDIA

#include <physx/PxPhysicsAPI.h>
#include <physx/PxScene.h>

namespace Saturn {
	
	class Scene;

	class PhysXScene {
	public:
		PhysXScene(Scene* scene);
		~PhysXScene();

	protected:
		physx::PxScene* m_PhysXScene;
		physx::
		Scene* m_Scene;

	private:
	};
}

#else
#error You need to use nvidia for physx!
#endif // USE_NVIDIA

