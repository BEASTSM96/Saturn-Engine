#pragma once

#include "PhysXWorld.h"
#include "physx/PxPhysicsAPI.h"
#include <glm/glm.hpp>

namespace Saturn::Physics {

	namespace {

		class PhysXRigidBody
		{
		private:
			template <typename T>
			T* GetDataAs();

			template <typename T>
			const T* GetDataAs() const;

			template <typename T>
			inline T* PhysXRigidBody::GetDataAs()
			{
				return static_cast<T*>(getData()->data);
			}

			template <typename T>
			inline const T* PhysXRigidBody::GetDataAs() const
			{
				return static_cast<T*>(getData()->data);
			}
		private:
			inline float GetDefaultAngularDrag()
			{
				return 0.05f;
			}

			inline glm::vec3 GetDefaultAngularVelocity()
			{
				return glm::vec3();
			}

			inline glm::vec3 GetDefaultCenterOfMass()
			{
				return glm::vec3();
			}

			inline float GetDefaultLinearDrag()
			{
				return 0.0f;
			}

			inline glm::vec3 GetDefaultLinearVelocity()
			{
				return glm::vec3();
			}

			inline float GetDefaultMass()
			{
				return 1.0f;
			}

			inline glm::vec3 GetDefaultDiagonalInertiaTensor()
			{
				return glm::vec3(1.0f, 1.0f, 1.0f);
			}

			inline float GetDefaultMaxAngularVelocity()
			{
				return 1000000.0f;
			}

			inline float GetDefaultMaxDepenetrationVelocity()
			{
				return 1000000.0f;
			}

			inline bool IsAffectedByGravityByDefault()
			{
				return true;
			}

			inline bool IsKinematicByDefault()
			{
				return false;
			}

			inline CollisionDetectionMode GetDefaultCollisionDetectionMode()
			{
				return CollisionDetectionMode::Continuous;
			}


		public:
			PhysXRigidBody() = delete;
			PhysXRigidBody(PhysXWorld* world);
			PhysXRigidBody(PhysXRigidBody&) = default;
			~PhysXRigidBody();

			PhysXRigidBody& operator=(const PhysXRigidBody&) = delete;

		private:

			void SetAngularDrag(float angularDrag);

			float GetAngularDrag(void) const;

			void SetAngularVelocity(const glm::vec3& angularVelocity);

			glm::vec3 GetAngularVelocity(void) const;

			void SetCenterOfMass(const glm::vec3& centerOfMass);

			glm::vec3 GetCenterOfMass(void) const;

			void SetLinearDrag(float linearDrag);

			float HetLinearDrag(void) const;

			void SetLinearVelocity(const glm::vec3& linearVelocity);

			glm::vec3 GetLinearVelocity(void) const;

			void SetMass(float mass);

			float GetMass(void) const;

			void SetDiagonalInertiaTensor(const glm::vec3& diagonalInertiaTensor);

			glm::vec3 GetDiagonalInertiaTensor(void) const;

			void SetMaxAngularVelocity(float maxAngularVelocity);

			float GetMaxAngularVelocity(void) const;

			void SetMaxDepenetrationVelocity(float maxDepenetrationVelocity);

			float GetMaxDepenetrationVelocity(void) const;

			void SetPosition(const glm::vec3& position);

			glm::vec3 GetPosition(void) const;

			void SetRotation(const glm::quat& rotation);

			glm::quat GetRotation(void) const;

			void AffectByGravity(bool mustBeAffectedByGravity);

			bool IsAffectedByGravity(void) const;

			void SetAsKinematic(bool mustBeKinematic);

			bool IsKinematic(void) const;

			void SetCollisionDetectionMode(CollisionDetectionMode collisionDetectionMode);

			CollisionDetectionMode GetCollisionDetectionMode(void) const;

			void AddForce(const glm::vec3& force, ForceMode forceMode);

			void AddForceAtWorldPosition(const glm::vec3& force, const glm::vec3& position, ForceMode forceMode);

			void AddForceAtLocalPosition(const glm::vec3& force, const glm::vec3& position, ForceMode forceMode);

			void AddAbsoluteTorque(const glm::vec3& torque, ForceMode forceMode);

			void AddRelativeTorque(const glm::vec3& torque, ForceMode forceMode);

			glm::vec3 GetVelocityAtWorldPosition(const glm::vec3& position) const;

			glm::vec3 GetVelocityAtLocalPosition(const glm::vec3& position) const;

		};

	}

}