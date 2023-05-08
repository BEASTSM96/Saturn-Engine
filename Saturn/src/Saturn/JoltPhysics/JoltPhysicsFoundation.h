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

#pragma once

#include "SingletonStorage.h"

#include "JoltBase.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>

namespace Saturn {

	class JoltPhysicsContactListener : public JPH::ContactListener
	{
	public:
		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate( const  JPH::Body& inBody1, const  JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult ) override
		{
			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void OnContactAdded( const JPH::Body& A, const JPH::Body& B, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings ) override
		{
		}

		virtual void OnContactPersisted( const JPH::Body& A, const JPH::Body& B, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings ) override
		{
		}

		virtual void OnContactRemoved( const JPH::SubShapeIDPair& inSubShapePair ) override
		{
		}
	};

	class JoltDynamicRigidBody;
	
	class JoltPhysicsFoundation
	{
	public:
		static inline JoltPhysicsFoundation& Get() { return *SingletonStorage::Get().GetOrCreateSingleton<JoltPhysicsFoundation>(); }
	public:
		JoltPhysicsFoundation();
		~JoltPhysicsFoundation();

		void Init();

		void Update( Timestep ts );

		// Creates a rigid body with a box collider.
		JPH::Body* CreateBoxCollider( const glm::vec3& Position, const glm::vec3& Extents, bool Kinematic = false );
		void DestoryBody( JPH::Body* pBody );

		JPH::PhysicsSystem* GetPhysicsSystem() { return m_PhysicsSystem; }
		const JPH::PhysicsSystem* GetPhysicsSystem() const { return m_PhysicsSystem; }

	private:
		JPH::PhysicsSystem*       m_PhysicsSystem = nullptr;
		JPH::JobSystemThreadPool* m_JobSystem = nullptr;
		JPH::TempAllocatorImpl*   m_Allocator = nullptr;

		JoltPhysicsContactListener m_ContactListener;
	};
}