#pragma once

#include "EventListener.h"
#pragma warning(push)
#include <reactphysics3d/reactphysics3d.h>
#pragma  warning(pop)

namespace Saturn {

	class Scene;

	class PhysicsScene {
	public:
		PhysicsScene(Scene* scene);
		~PhysicsScene();

		void Update(float delta);

		void Contact(rp3d::CollisionBody* body);
	private:
		friend class Scene;

		rp3d::PhysicsCommon m_common;
		rp3d::PhysicsWorld* m_world;

		float m_accumulator{ 0.0f };

		EventListener m_eventListener;

		Scene* m_scene;
	private:
		friend class Rigidbody;
	};
}
