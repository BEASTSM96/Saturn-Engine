/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "JoltRuntime.h"

#include "Saturn/Core/OptickProfiler.h"

#include "JoltPhysicsFoundation.h"
#include "JoltDynamicRigidBody.h"

#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Components.h"

namespace Saturn {
	
	JoltRuntime::JoltRuntime( const Ref<Scene> scene )
		: m_Scene( scene )
	{
		auto view = m_Scene->GetRegistry().view<TransformComponent, RigidbodyComponent>();

		// Create all rigid bodies.
		for( const auto& entity : view )
		{
			auto [tc, rb] = view.get<TransformComponent, RigidbodyComponent>( entity );
			Entity e{ entity, m_Scene.Pointer() };

			rb.Rigidbody = new JoltDynamicRigidBody( e );
			rb.Rigidbody->SetKinematic( rb.IsKinematic );

			// I do not like this...
			if( e.HasComponent<BoxColliderComponent>() )
			{
				rb.Rigidbody->AttachBox( e.GetComponent<BoxColliderComponent>().Extents );
			}
			else if( e.HasComponent<SphereColliderComponent>() ) 
			{
				rb.Rigidbody->AttachSphere( e.GetComponent<SphereColliderComponent>().Radius );
			}
			else if( e.HasComponent<CapsuleColliderComponent>() )
			{
				rb.Rigidbody->AttachCapsule( e.GetComponent<CapsuleColliderComponent>().Radius, e.GetComponent<CapsuleColliderComponent>().Height );
			}
			else
			{
				rb.Rigidbody->AttachMesh( e.GetComponent<MeshColliderComponent>().AssetID );
			}

			rb.Rigidbody->Create( tc.Position, tc.Rotation );
		}
	}

	JoltRuntime::~JoltRuntime()
	{
		auto view = m_Scene->GetRegistry().view<TransformComponent, RigidbodyComponent>();

		// Destroy all rigid bodies.
		for( const auto& entity : view )
		{
			auto [tc, rb] = view.get<TransformComponent, RigidbodyComponent>( entity );
			rb.Rigidbody = nullptr;
		}
	}

	void JoltRuntime::OnUpdate( Timestep ts )
	{
		SAT_PF_EVENT();

		JoltPhysicsFoundation::Get().Update( ts );
	}

}