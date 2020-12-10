#pragma once

#pragma warning(push)
#include <reactphysics3d/reactphysics3d.h>
#pragma  warning(pop)

#include "Saturn/Log.h"

namespace Saturn {

	class PhysicsScene;

	class EventListener : public rp3d::EventListener {
	private:
		PhysicsScene* m_physicsScene;
	public:
		EventListener(PhysicsScene* physicsScene);

		void onContact(const rp3d::CollisionCallback::CallbackData& callbackData) override;
	};
}