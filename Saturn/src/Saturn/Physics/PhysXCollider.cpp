#include "sppch.h"
#include "PhysXCollider.h"

namespace Saturn::Physics {

	PhysXCollider::~PhysXCollider()
	{

	}

	PhysXCollider::PhysXCollider(PhysXWorld * world, std::vector<physx::PxShape*> shapes)
	{
		physx::PxRigidDynamic* body = GetDataAs<physx::PxRigidDynamic>();
		static_cast<PhysXWorld*>(world)->GetScene()->addActor(*body);
		body->setActorFlag(physx::PxActorFlag::eVISUALIZATION, true);
		body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
		body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		body->setMass(0.0f);
	}

	std::vector<physx::PxShape*>& PhysXCollider::GetShapes()
	{
		return m_Shapes;
	}

	const std::vector<physx::PxShape*>& PhysXCollider::GetShapes() const
	{
		return m_Shapes;
	}

	void PhysXCollider::UpdateShape(std::vector<physx::PxShape*> newShapes)
	{
		SAT_CORE_ASSERT(newShapes.empty(), "Error! Invalid shape");
		physx::PxRigidDynamic* body = GetDataAs<physx::PxRigidDynamic>();
		for (physx::PxShape* shape : m_Shapes)
		{
			body->detachShape(*shape);
		}
		m_Shapes = std::move(newShapes);
		for (physx::PxShape* shape : m_Shapes)
		{
			body->attachShape(*shape);
			shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
		}
	}


	void PhysXCollider::SetAsTrigger(bool mustBeATrigger)
	{
		for (physx::PxShape* shape : m_Shapes)
		{
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !mustBeATrigger);
			shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, !mustBeATrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, mustBeATrigger);
		}
	}

	bool PhysXCollider::IsATrigger() const
	{
		return m_Shapes.empty() ? true : m_Shapes[0]->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE);
	}

	void PhysXCollider::SetFilterGroup(FilterGroup group)
	{
		physx::PxFilterData filterData;
		filterData.word0 = static_cast<physx::PxU32>(group);
		for (physx::PxShape* shape : m_Shapes)
		{
			shape->setSimulationFilterData(filterData);
			shape->setQueryFilterData(filterData);
		}
	}

	FilterGroup PhysXCollider::GetFilterGroup() const
	{
		return m_Shapes.empty() ? FilterGroup::Default : static_cast<FilterGroup>(m_Shapes[0]->getSimulationFilterData().word0);
	}

	void PhysXCollider::SetMaterial(const std::string& name)
	{
		PhysXMaterial* newMaterial = GetWorld()->CreateMaterial(name);
		static_cast<PhysXCollider*>(this)->SetMaterial(newMaterial);
		physx::PxMaterial* concreteMaterial = static_cast<PhysXMaterial*>(newMaterial)->GetMaterial();
		for (physx::PxShape* shape : m_Shapes)
		{
			shape->setMaterials(&concreteMaterial, 1);
		}
	}

	void PhysXCollider::SetPosition(const glm::vec3& position)
	{
		physx::PxRigidDynamic* actor = GetDataAs<physx::PxRigidDynamic>();
		physx::PxTransform transform = actor->getGlobalPose();
		transform.p.x = position.x;
		transform.p.y = position.y;
		transform.p.z = position.z;
		actor->setGlobalPose(transform);
		if (actor->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
		{
			actor->setKinematicTarget(transform);
		}
	}

	void PhysXCollider::SetRotation(const glm::quat& rotation)
	{
		physx::PxRigidDynamic* actor = GetDataAs<physx::PxRigidDynamic>();
		physx::PxTransform transform = actor->getGlobalPose();
		transform.q.x = rotation.x;
		transform.q.y = rotation.y;
		transform.q.z = rotation.z;
		transform.q.w = rotation.w;
		actor->setGlobalPose(transform);
		if (actor->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
		{
			actor->setKinematicTarget(transform);
		}
	}

}