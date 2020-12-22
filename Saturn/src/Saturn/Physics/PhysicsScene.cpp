/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

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

		m_world->setIsDebugRenderingEnabled(true);

		reactphysics3d::DebugRenderer& debugRenderer = m_world->getDebugRenderer();
		debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_POINT, true);
		debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_NORMAL, true);
		debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLIDER_AABB, true);
		debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB, true);
		debugRenderer.setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);


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