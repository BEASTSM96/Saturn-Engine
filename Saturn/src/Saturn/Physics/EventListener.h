#pragma once

#include <reactphysics3d/reactphysics3d.h>

namespace Saturn {

	class PhysicsScene;

	class EventListener : public reactphysics3d::EventListener
	{
	public:
		EventListener(PhysicsScene* physicsScene);
		void onContact(const reactphysics3d::CollisionCallback::CallbackData& callbackData) override;
	private:
		PhysicsScene* m_PhysicsScene;
	};
}