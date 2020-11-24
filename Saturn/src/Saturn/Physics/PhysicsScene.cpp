#include "sppch.h"
#include "PhysicsScene.h"

#include "Saturn/Scene/Scene.h"

#include <GLFW/glfw3.h>

#include "Saturn/Renderer/Renderer.h"

#include "Saturn/Scene/Components.h"

namespace Saturn {

	PhysicsScene::PhysicsScene(Scene* scene) : m_scene(scene), m_eventListener(this) {
		m_world = m_common.createPhysicsWorld();
		m_world->setEventListener(&m_eventListener);
	}

	PhysicsScene::~PhysicsScene() {

	}

	void PhysicsScene::Update(float delta) {
		const float timeStep = 1.0f / 60.0f;

		m_accumulator += delta;

		while (m_accumulator >= timeStep) {
			m_world->update(timeStep);
			m_scene->PhysicsUpdate(timeStep);

			m_accumulator -= delta;
		}
	}

	void PhysicsScene::Contact(rp3d::CollisionBody* body) {
		m_scene->Contact(body);
	}

	glm::vec3 Vec3FromReactVec3(const reactphysics3d::Vector3& matrix)
	{
		glm::vec3 result;
		result.x = matrix.x;
		result.y = matrix.y;
		result.z = matrix.z;
		return result;
	}

	glm::vec3 Vec3FromReactQuaternion(const reactphysics3d::Quaternion& matrix)
	{
		glm::vec3 result;
		result.x = matrix.x;
		result.y = matrix.y;
		result.z = matrix.z;
		return result;
	}

	glm::mat4 Mat4FromReactMat4(const reactphysics3d::Transform& matrix)
	{
		glm::mat4 result;
		result = glm::translate(result, Vec3FromReactVec3(matrix.getPosition()));
		//result = glm::rotate(Vec3FromReactQuaternion(matrix.getOrientation()));
		return result;
	}
}