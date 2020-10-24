#pragma once

#include <string>
#include <physx/PxPhysicsAPI.h>
#include "PhysXWorld.h"

namespace Saturn::Physics {

	class PhysXMaterial
	{
	public:

		PhysXMaterial(void) = delete;
		PhysXMaterial(PhysXWorld* world, const std::string& name);
		PhysXMaterial(const PhysXMaterial&) = delete;
		~PhysXMaterial();

		PhysXMaterial& operator=(const PhysXMaterial&) = delete;

		physx::PxMaterial* GetMaterial();

		const physx::PxMaterial* GetMaterial() const;

	protected:
		static float GetDefaultStaticFriction();

		static float GetDefaultDynamicFriction();

		static float GetDefaultRestitution();
	private:

		physx::PxMaterial* m_PxMaterial = nullptr;

		void SetStaticFriction(float staticFriction);

		float GetStaticFriction(void) const;

		void SetDynamicFriction(float dynamicFriction);

		float GetDynamicFriction(void) const;

		void SetRestitution(float restitution);

		float GetRestitution(void) const;

	};

}