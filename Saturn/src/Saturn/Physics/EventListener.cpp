#include "sppch.h"
#include "EventListener.h"

#include "PhysicsScene.h"

namespace Saturn {

	EventListener::EventListener(PhysicsScene* physicsScene) : m_PhysicsScene(physicsScene) {

	}

	void EventListener::onContact(const reactphysics3d::CollisionCallback::CallbackData& callbackData) {
		// For each contact pair
		for (unsigned int p = 0; p < callbackData.getNbContactPairs(); p++) {

			// Get the contact pair
			reactphysics3d::CollisionCallback::ContactPair contactPair = callbackData.getContactPair(p);

			// For each contact point of the contact pair
			for (unsigned int c = 0; c < contactPair.getNbContactPoints(); c++) {
				if (contactPair.getEventType() == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactExit) {
					m_PhysicsScene->ContactExit(contactPair.getCollider1()->getBody(), contactPair.getCollider2()->getBody());
					SAT_CORE_INFO("HIT!!!");
				}
				else if (contactPair.getEventType() == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStart) {
					m_PhysicsScene->ContactEnter(contactPair.getCollider1()->getBody(), contactPair.getCollider2()->getBody());
				}
				else if (contactPair.getEventType() == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStay) {
					m_PhysicsScene->ContactStay(contactPair.getCollider1()->getBody(), contactPair.getCollider2()->getBody());
				}
			}
		}
	}

}