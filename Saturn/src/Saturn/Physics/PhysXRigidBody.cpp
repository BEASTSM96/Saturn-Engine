#include "sppch.h"
#include "PhysXRigidBody.h"

#include "physx/PxPhysicsAPI.h"

namespace Saturn::Physics {
	namespace {

		PhysXRigidBody::PhysXRigidBody(PhysXWorld* world)
		{
			if (GetData()->data == nullptr)
			{
				GetData()->data = static_cast<PhysXPhysics*>(world->GetPhysics())->getPhysics()->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
				physx::PxRigidDynamic* body = GetDataAs<physx::PxRigidDynamic>();
				world->GetScene()->addActor(*body);
				body->userData = this;
				body->setActorFlag(physx::PxActorFlag::eVISUALIZATION, true);
			}
			else
			{
				static_cast<PhysXCollider*>(GetDataAs<physx::PxRigidDynamic>()->userData)->rigidBody = this;
			}
			SetAsKinematic(IsKinematicByDefault());
			SetAngularDrag(GetDefaultAngularDrag());
			SetAngularVelocity(GetDefaultAngularVelocity());
			SetCenterOfMass(GetDefaultCenterOfMass());
			SetLinearDrag(GetDefaultLinearDrag());
			SetLinearVelocity(GetDefaultLinearVelocity());
			SetMass(GetDefaultMass());
			SetDiagonalInertiaTensor(GetDefaultDiagonalInertiaTensor());
			SetMaxAngularVelocity(GetDefaultMaxAngularVelocity());
			SetMaxDepenetrationVelocity(GetDefaultMaxDepenetrationVelocity());
			AffectByGravity(IsAffectedByGravityByDefault());
			SetCollisionDetectionMode(GetDefaultCollisionDetectionMode());
		}

		PhysXRigidBody::~PhysXRigidBody()
		{

		}

		void PhysXRigidBody::SetAngularDrag(float angularDrag)
		{
			GetDataAs<physx::PxRigidDynamic>()->setAngularDamping(angularDrag);
		}

		float PhysXRigidBody::GetAngularDrag(void) const
		{
			return GetDataAs<physx::PxRigidDynamic>()->getAngularDamping();
		}

		void PhysXRigidBody::SetAngularVelocity(const glm::vec3& angularVelocity)
		{
			GetDataAs<physx::PxRigidDynamic>()->setAngularVelocity(physx::PxVec3(angularVelocity.x, angularVelocity.y, angularVelocity.z));
		}

		glm::vec3 PhysXRigidBody::GetAngularVelocity(void) const
		{
			const physx::PxVec3 angularVelocity = GetDataAs<physx::PxRigidDynamic>()->getAngularVelocity();
			return glm::vec3(angularVelocity.x, angularVelocity.y, angularVelocity.z);
		}

		void PhysXRigidBody::SetCenterOfMass(const glm::vec3& centerOfMass)
		{
			GetDataAs<physx::PxRigidDynamic>()->setCMassLocalPose(physx::PxTransform(centerOfMass.x, centerOfMass.y, centerOfMass.z));
		}

		glm::vec3 PhysXRigidBody::GetCenterOfMass(void) const
		{
			const physx::PxVec3 centerOfMass = GetDataAs<physx::PxRigidDynamic>()->getCMassLocalPose().p;
			return glm::vec3(centerOfMass.x, centerOfMass.y, centerOfMass.z);
		}

		void PhysXRigidBody::SetLinearDrag(float linearDrag)
		{
			GetDataAs<physx::PxRigidDynamic>()->setLinearDamping(linearDrag);
		}

		float PhysXRigidBody::HetLinearDrag(void) const
		{
			return GetDataAs<physx::PxRigidDynamic>()->getLinearDamping();
		}

		void PhysXRigidBody::SetLinearVelocity(const glm::vec3& linearVelocity)
		{
			GetDataAs<physx::PxRigidDynamic>()->setLinearVelocity(physx::PxVec3(linearVelocity.x, linearVelocity.y, linearVelocity.z));
		}

		glm::vec3 PhysXRigidBody::GetLinearVelocity(void) const
		{
			const physx::PxVec3 linearVelocity = GetDataAs<physx::PxRigidDynamic>()->getLinearVelocity();
			return glm::vec3(linearVelocity.x, linearVelocity.y, linearVelocity.z);
		}

		void PhysXRigidBody::SetMass(float mass)
		{
			GetDataAs<physx::PxRigidDynamic>()->setMass(mass);
		}

		float PhysXRigidBody::GetMass(void) const
		{
			return GetDataAs<physx::PxRigidDynamic>()->getMass();
		}

		void PhysXRigidBody::SetDiagonalInertiaTensor(const glm::vec3& diagonalInertiaTensor)
		{
			GetDataAs<physx::PxRigidDynamic>()->setMassSpaceInertiaTensor(physx::PxVec3(diagonalInertiaTensor.x, diagonalInertiaTensor.y, diagonalInertiaTensor.z));
		}

		glm::vec3 PhysXRigidBody::GetDiagonalInertiaTensor(void) const
		{
			const physx::PxVec3 diagonalInertiaTensor = GetDataAs<physx::PxRigidDynamic>()->getMassSpaceInertiaTensor();
			return glm::vec3(diagonalInertiaTensor.x, diagonalInertiaTensor.y, diagonalInertiaTensor.z);
		}

		void PhysXRigidBody::SetMaxAngularVelocity(float maxAngularVelocity)
		{
			GetDataAs<physx::PxRigidDynamic>()->setMaxAngularVelocity(maxAngularVelocity);
		}

		float PhysXRigidBody::GetMaxAngularVelocity(void) const
		{
			return GetDataAs<physx::PxRigidDynamic>()->getMaxAngularVelocity();
		}

		void PhysXRigidBody::SetMaxDepenetrationVelocity(float maxDepenetrationVelocity)
		{
			GetDataAs<physx::PxRigidDynamic>()->setMaxDepenetrationVelocity(maxDepenetrationVelocity);
		}

		float PhysXRigidBody::GetMaxDepenetrationVelocity(void) const
		{
			return GetDataAs<physx::PxRigidDynamic>()->getMaxDepenetrationVelocity();
		}

		void PhysXRigidBody::SetPosition(const glm::vec3& position)
		{
			physx::PxRigidDynamic* actor = GetDataAs<physx::PxRigidDynamic>();
			physx::PxTransform transform = actor->getGlobalPose();
			transform.p.x = position.x;
			transform.p.y = position.y;
			transform.p.z = position.z;
			actor->setGlobalPose(transform);
			if (IsKinematic())
			{
				actor->setKinematicTarget(transform);
			}
		}

		glm::vec3 PhysXRigidBody::GetPosition(void) const
		{
			const physx::PxVec3 position = GetDataAs<physx::PxRigidDynamic>()->getGlobalPose().p;
			return glm::vec3(position.x, position.y, position.z);
		}

		void PhysXRigidBody::SetRotation(const glm::quat& rotation)
		{
			physx::PxRigidDynamic* actor = GetDataAs<physx::PxRigidDynamic>();
			physx::PxTransform transform = actor->getGlobalPose();
			transform.q.x = rotation.x;
			transform.q.y = rotation.y;
			transform.q.z = rotation.z;
			transform.q.w = rotation.w;
			actor->setGlobalPose(transform);
			if (IsKinematic())
			{
				actor->setKinematicTarget(transform);
			}
		}

		glm::quat PhysXRigidBody::GetRotation(void) const
		{
			const physx::PxQuat rotation = GetDataAs<physx::PxRigidDynamic>()->getGlobalPose().q;
			return glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
		}

		void PhysXRigidBody::AffectByGravity(bool mustBeAffectedByGravity)
		{
			GetDataAs<physx::PxRigidDynamic>()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !mustBeAffectedByGravity);
		}

		bool PhysXRigidBody::IsAffectedByGravity(void) const
		{
			return !GetDataAs<physx::PxRigidDynamic>()->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
		}

		void PhysXRigidBody::SetAsKinematic(bool mustBeKinematic)
		{
			physx::PxRigidDynamic* actor = GetDataAs<physx::PxRigidDynamic>();
			actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, mustBeKinematic);
			if (mustBeKinematic)
			{
				const glm::vec3 position = GetPosition();
				const glm::quat rotation = GetRotation();
				actor->setKinematicTarget(physx::PxTransform(position.x, position.y, position.z, physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)));
			}
		}

		bool PhysXRigidBody::IsKinematic(void) const
		{
			return GetDataAs<physx::PxRigidDynamic>()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);
		}

		void PhysXRigidBody::SetCollisionDetectionMode(CollisionDetectionMode collisionDetectionMode)
		{
			GetDataAs<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, collisionDetectionMode == CollisionDetectionMode::Continuous);
		}

		CollisionDetectionMode PhysXRigidBody::GetCollisionDetectionMode(void) const
		{
			return GetDataAs<physx::PxRigidDynamic>()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eENABLE_CCD) ? CollisionDetectionMode::Continuous : CollisionDetectionMode::Discrete;
		}

		void PhysXRigidBody::AddForce(const glm::vec3& force, ForceMode forceMode)
		{
			switch (forceMode)
			{
			case ForceMode::Acceleration:
				GetDataAs<physx::PxRigidDynamic>()->addForce(physx::PxVec3(force.x, force.y, force.z), physx::PxForceMode::eACCELERATION);
				break;
			case ForceMode::Force:
				GetDataAs<physx::PxRigidDynamic>()->addForce(physx::PxVec3(force.x, force.y, force.z), physx::PxForceMode::eFORCE);
				break;
			case ForceMode::Impulse:
				GetDataAs<physx::PxRigidDynamic>()->addForce(physx::PxVec3(force.x, force.y, force.z), physx::PxForceMode::eIMPULSE);
				break;
			case ForceMode::VelocityChange:
				GetDataAs<physx::PxRigidDynamic>()->addForce(physx::PxVec3(force.x, force.y, force.z), physx::PxForceMode::eVELOCITY_CHANGE);
				break;
			default:
				break;
			}
		}

		void PhysXRigidBody::AddForceAtWorldPosition(const glm::vec3& force, const glm::vec3& position, ForceMode forceMode)
		{
			switch (forceMode)
			{
			case ForceMode::Acceleration:
				physx::PxRigidBodyExt::addForceAtPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(force.x, force.y, force.z), physx::PxVec3(position.x, position.y, position.z), physx::PxForceMode::eACCELERATION);
				break;
			case ForceMode::Force:
				physx::PxRigidBodyExt::addForceAtPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(force.x, force.y, force.z), physx::PxVec3(position.x, position.y, position.z), physx::PxForceMode::eFORCE);
				break;
			case ForceMode::Impulse:
				physx::PxRigidBodyExt::addForceAtPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(force.x, force.y, force.z), physx::PxVec3(position.x, position.y, position.z), physx::PxForceMode::eIMPULSE);
				break;
			case ForceMode::VelocityChange:
				physx::PxRigidBodyExt::addForceAtPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(force.x, force.y, force.z), physx::PxVec3(position.x, position.y, position.z), physx::PxForceMode::eVELOCITY_CHANGE);
				break;
			default:
				break;
			}
		}

		void PhysXRigidBody::AddForceAtLocalPosition(const glm::vec3& force, const glm::vec3& position, ForceMode forceMode)
		{

			switch (forceMode)
			{
			case ForceMode::Acceleration:
				physx::PxRigidBodyExt::addForceAtLocalPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(force.x, force.y, force.z), physx::PxVec3(position.x, position.y, position.z), physx::PxForceMode::eACCELERATION);
				break;
			case ForceMode::Force:
				physx::PxRigidBodyExt::addForceAtLocalPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(force.x, force.y, force.z), physx::PxVec3(position.x, position.y, position.z), physx::PxForceMode::eFORCE);
				break;
			case ForceMode::Impulse:
				physx::PxRigidBodyExt::addForceAtLocalPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(force.x, force.y, force.z), physx::PxVec3(position.x, position.y, position.z), physx::PxForceMode::eIMPULSE);
				break;
			case ForceMode::VelocityChange:
				physx::PxRigidBodyExt::addForceAtLocalPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(force.x, force.y, force.z), physx::PxVec3(position.x, position.y, position.z), physx::PxForceMode::eVELOCITY_CHANGE);
				break;
			default:
				break;
			}
		}

		void PhysXRigidBody::AddAbsoluteTorque(const glm::vec3& torque, ForceMode forceMode)
		{
			switch (forceMode)
			{
			case ForceMode::Acceleration:
				GetDataAs<physx::PxRigidDynamic>()->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), physx::PxForceMode::eACCELERATION);
				break;
			case ForceMode::Force:
				GetDataAs<physx::PxRigidDynamic>()->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), physx::PxForceMode::eFORCE);
				break;
			case ForceMode::Impulse:
				GetDataAs<physx::PxRigidDynamic>()->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), physx::PxForceMode::eIMPULSE);
				break;
			case ForceMode::VelocityChange:
				GetDataAs<physx::PxRigidDynamic>()->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), physx::PxForceMode::eVELOCITY_CHANGE);
				break;
			default:
				break;
			}
		}

		void PhysXRigidBody::AddRelativeTorque(const glm::vec3& torque, ForceMode forceMode)
		{
			const physx::PxVec3 absoluteTorque = GetDataAs<physx::PxRigidDynamic>()->getGlobalPose().transform(physx::PxVec3(torque.x, torque.y, torque.z));
			AddAbsoluteTorque(glm::vec3(absoluteTorque.x, absoluteTorque.y, absoluteTorque.z), forceMode);
		}

		glm::vec3 PhysXRigidBody::GetVelocityAtWorldPosition(const glm::vec3& position) const
		{
			const physx::PxVec3 velocity = physx::PxRigidBodyExt::getVelocityAtPos(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(position.x, position.y, position.z));
			return glm::vec3(velocity.x, velocity.y, velocity.z);
		}

		glm::vec3 PhysXRigidBody::GetVelocityAtLocalPosition(const glm::vec3& position) const
		{
			const physx::PxVec3 velocity = physx::PxRigidBodyExt::getVelocityAtOffset(*GetDataAs<physx::PxRigidDynamic>(), physx::PxVec3(position.x, position.y, position.z));
			return glm::vec3(velocity.x, velocity.y, velocity.z);
		}

	}
}