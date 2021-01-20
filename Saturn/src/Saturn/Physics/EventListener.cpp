/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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
#include "EventListener.h"

#include "PhysicsScene.h"

#include <stddef.h>

namespace Saturn {

	EventListener::EventListener(PhysicsScene* physicsScene) : m_physicsScene(physicsScene) 
	{

	}

	void EventListener::onContact(const rp3d::CollisionCallback::CallbackData& callbackData) 
	{
		// For each contact pair
		for( unsigned int p = 0; p < callbackData.getNbContactPairs(); p++ )
		{
			// Get the contact pair
			rp3d::CollisionCallback::ContactPair contactPair = callbackData.getContactPair( p );

			// For each contact point of the contact pair
			for( unsigned int c = 0; c < contactPair.getNbContactPoints(); c++ )
			{
				if( contactPair.getEventType() == rp3d::CollisionCallback::ContactPair::EventType::ContactExit )
				{
					m_physicsScene->ContactExit( contactPair.getCollider1()->getBody(), contactPair.getCollider2()->getBody() );
					SAT_CORE_INFO( "Contact" );
				}
				else if( contactPair.getEventType() == rp3d::CollisionCallback::ContactPair::EventType::ContactStart )
				{
					m_physicsScene->ContactEnter( contactPair.getCollider1()->getBody(), contactPair.getCollider2()->getBody() );
				}
				else if( contactPair.getEventType() == rp3d::CollisionCallback::ContactPair::EventType::ContactStay )
				{
					m_physicsScene->ContactStay( contactPair.getCollider1()->getBody(), contactPair.getCollider2()->getBody() );
				}
			}
		}
	}
}