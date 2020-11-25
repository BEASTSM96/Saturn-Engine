#include "sppch.h"
#include "EventListener.h"

#include "PhysicsScene.h"

#include <stddef.h>

namespace Saturn {

	EventListener::EventListener(PhysicsScene* physicsScene) : m_physicsScene(physicsScene) {

	}

	void EventListener::onContact(const rp3d::CollisionCallback::CallbackData& callbackData) {
		// For each contact pair
		for (unsigned int p = 0; p < callbackData.getNbContactPairs(); p++) {

			// Get the contact pair
			rp3d::CollisionCallback::ContactPair contactPair = callbackData.getContactPair(p);

			// For each contact point of the contact pair
			for (unsigned int c = 0; c < contactPair.getNbContactPoints(); c++) {
				rp3d::CollisionCallback::ContactPoint contactPoint = contactPair.getContactPoint(c);
				m_physicsScene->Contact(contactPair.getCollider1()->getBody());
				m_physicsScene->Contact(contactPair.getCollider2()->getBody());
			}
		}
	}
}