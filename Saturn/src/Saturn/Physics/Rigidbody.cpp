#include "sppch.h"
#include "Rigidbody.h"

#include "PhysicsScene.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace Saturn {

	Rigidbody::Rigidbody(PhysicsScene* scene, bool useGravity, const glm::vec3& position, const glm::vec3& rotation)
		: m_scene(scene) 
	{
		reactphysics3d::Vector3 vec(position.x, position.y, position.z);

		reactphysics3d::Quaternion quat = reactphysics3d::Quaternion::identity();
		quat.x = rotation.x; 
		quat.y = rotation.y; 
		quat.z = rotation.z;

		reactphysics3d::Transform transform(vec, quat);

		m_body = m_scene->m_world->createRigidBody(transform);

		m_body->setType(reactphysics3d::BodyType::DYNAMIC);
		m_isKinematic = false;

		if (useGravity)
		{
			UseGravity(useGravity);
		}
		else
		{
			UseGravity(false);
		}

	}

	Rigidbody::~Rigidbody() 
	{
		m_scene->m_world->destroyRigidBody(m_body);
	}

	glm::vec3 Rigidbody::GetPosition()
	{
		return glm::vec3(
			m_body->getTransform().getPosition().x,
			m_body->getTransform().getPosition().y,
			m_body->getTransform().getPosition().z);
	}

	glm::quat Rigidbody::GetRotation() 
	{
		return glm::quat(
			m_body->getTransform().getOrientation().w,
			m_body->getTransform().getOrientation().x,
			m_body->getTransform().getOrientation().y,
			m_body->getTransform().getOrientation().z
		);
	}

	void Rigidbody::AddBoxCollider(const glm::vec3& halfExtents) 
	{
		const reactphysics3d::Vector3 e(halfExtents.x, halfExtents.y, halfExtents.z);

		reactphysics3d::BoxShape* shape = m_scene->m_common.createBoxShape(e);

		reactphysics3d::Transform transform = reactphysics3d::Transform::identity();
		reactphysics3d::Collider* col = m_body->addCollider(shape, transform);
		m_colliders.push_back(col);
	}

	void Rigidbody::AddSphereCollider(float radius) 
	{
		reactphysics3d::SphereShape* shape = m_scene->m_common.createSphereShape(radius);

		reactphysics3d::Transform transform = reactphysics3d::Transform::identity();
		reactphysics3d::Collider* col = m_body->addCollider(shape, transform);
		m_colliders.push_back(col);
	}

	void Rigidbody::SetKinematic(bool set) 
	{
		if (set) {
			m_body->setType(reactphysics3d::BodyType::KINEMATIC);
		}
		else {
			m_body->setType(reactphysics3d::BodyType::DYNAMIC);
		}

		m_isKinematic = set;
	}

	float Rigidbody::GetMass() const 
	{
		return m_body->getMass();
	}

	void Rigidbody::SetMass(float mass) 
	{
		m_body->setMass(mass);
	}

	void Rigidbody::ApplyForce(glm::vec3 force)
	{
		m_body->applyForceToCenterOfMass(reactphysics3d::Vector3(force.x, force.y, force.z));
	}

	void Rigidbody::ApplyForceAtLocation(glm::vec3 force, glm::vec3 location)
	{
		m_body->applyForceAtLocalPosition(reactphysics3d::Vector3(force.x, force.y, force.z), reactphysics3d::Vector3(location.x, location.y, location.z));
	}

	void Rigidbody::ApplyTorque(glm::vec3 torque)
	{
		m_body->applyTorque(reactphysics3d::Vector3(torque.x, torque.y, torque.z));
	}

	void Rigidbody::SetCOG(glm::vec3 cog)
	{
		m_body->setLocalCenterOfMass(reactphysics3d::Vector3(cog.x, cog.y, cog.z));
	}

	void Rigidbody::UseGravity(bool use)
	{
		m_body->enableGravity(use);
	}
}
