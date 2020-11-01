#pragma once

#include "PhysicsCore.h"
#include "PhysXBoxCollider.h"
#include "PhysXCollider.h"
#include "PhysXMaterial.h"
#include "PhysXPhysics.h"
#include "PhysXMeshCollider.h"
#include "PhysXCapsuleCollider.h"
#include "PhysXSphereCollider.h"

namespace Saturn::Physics {

	class PhysXWorld : 
		public physx::PxSimulationEventCallback, 
		public physx::PxBroadPhaseCallback, 
		public PhysXRigidBody,
		public PhysXMaterial,
		public PhysXBoxCollider,
		public PhysXCapsuleCollider,
		public PhysXMeshCollider, 
		public PhysXSphereCollider
	{
	public:

		PhysXWorld() = delete;
		PhysXWorld(PhysXWorld&) = delete;
		PhysXWorld(PhysXPhysics * Physics);
		~PhysXWorld();

		PhysXWorld& operator=(const PhysXWorld&) = delete;

		physx::PxScene* GetScene();

		const physx::PxScene* GetScene() const;

		std::vector<physx::PxShape*> GetCollisionShapes(const std::string& mesh, bool isConvex);

	protected:
		//

	private:
		physx::PxScene* m_Scene = nullptr;

		physx::PxU64 m_WorldAddress[1];

		physx::PxU32 m_GroupCollisionFlags[32];

#ifdef SAT_PLATFORM_WINDOWS
		__declspec(align(16)) std::uint8_t  m_ScratchMemoryBlock[65536];
#endif // SAT_PLATFORM_WINDOWS

		physx::PxDefaultCpuDispatcher* m_CpuDispatcher = nullptr;

		physx::PxCudaContextManager* m_CudaContextManager = nullptr;

		std::unordered_map<PhysXCollider*, std::unordered_map<PhysXCollider*, std::size_t>>  m_Triggers;

		std::unordered_map<std::string, std::pair<void*, std::vector<physx::PxBase*>>> collisionShapes;

		static physx::PxFilterFlags FilterShader(physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxFilterObjectAttributes attributes2,
			physx::PxFilterData filterData2, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

		static physx::PxU32 GetIndexForFilterGroup(FilterGroup group);

		void NotifyTriggers();

		void SetGravity(const glm::vec3& gravity);

		glm::vec3 GetGravity() const;

		void EnableCollisionBetweenGroups(FilterGroup group1, FilterGroup group2);

		void DisableCollisionBetweenGroups(FilterGroup group1, FilterGroup group2);

		void Simulate(float stepSize);

		PhysXRaycaster* CreateRaycaster();

		PhysXRigidBody* CreateRigidBody(Private::GenericData* data);

		void DestroyRigidBody(PhysXRigidBody* rigidBody);

		PhysXMaterial* CreateCollider(ColliderType colliderType, const std::string& mesh, Private::GenericData* data);

		void DestroyCollider(PhysXMaterial* collider);

		PhysXMaterial* CreateMaterial(const std::string& name);

		void DestroyMaterial(PhysXMaterial* material);

		void EnableDebug();

		void DisableDebug();

		void OnConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 numberOfConstraints);

		void OnWake(physx::PxActor** actors, physx::PxU32 numberOfActors);

		void OnSleep(physx::PxActor** actors, physx::PxU32 numberOfActors);

		void OnContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 numberOfPairs);

		void OnTrigger(physx::PxTriggerPair* pairs, physx::PxU32 numberOfPairs);

		void OnObjectOutOfBounds(physx::PxShape& shape, physx::PxActor& actor);

		void OnObjectOutOfBounds(physx::PxAggregate& aggregate);

	};

}