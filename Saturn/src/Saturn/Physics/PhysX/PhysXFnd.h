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

#pragma once

#include "Saturn/Scene/Entity.h"

#include "Saturn/Scene/Components.h"

#include <stdint.h>

namespace Saturn {

	//The Main PhysX class for Saturn.

	enum class ForceMode : uint16_t
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class BroadphaseType
	{
		SweepAndPrune,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType
	{
		Patch,
		OneDirectional,
		TwoDirectional
	};

	//TODO: Settings

	class PhysXContact : public physx::PxSimulationEventCallback, public RefCounted
	{
	public:
		virtual void onConstraintBreak( physx::PxConstraintInfo* constraints, physx::PxU32 count ) override;
		virtual void onWake( physx::PxActor** actors, physx::PxU32 count ) override;
		virtual void onSleep( physx::PxActor** actors, physx::PxU32 count ) override;
		virtual void onContact( const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs ) override;
		virtual void onTrigger( physx::PxTriggerPair* pairs, physx::PxU32 count ) override;
		virtual void onAdvance( const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count ) override;
	};

	class PhysXAssertCallback : public physx::PxAssertHandler, public RefCounted
	{
	public:
		virtual void operator()( const char* exp, const char* file, int line, bool& ignore ) override;
	};

	class PhysXFnd : public RefCounted
	{
	public:
		static void Init();
		static void Clear();
		static physx::PxScene* CreateScene();

		static void CreateBoxCollider( Entity& entity, physx::PxRigidActor& actor );
		static void CreateSphereCollider( Entity& entity, physx::PxRigidActor& actor );
		static void CreateCapsuleCollider( Entity& entity, physx::PxRigidActor& actor );
		static void CreateMeshCollider( Entity& entity, physx::PxRigidActor& actor );

		static std::vector<physx::PxShape*> CreateConvexMesh( PhysXMeshColliderComponent& collider, const glm::vec3& size, bool invalidateOld = false );
		static std::vector<physx::PxShape*> CreateTriangleMesh( PhysXMeshColliderComponent& collider, const glm::vec3& scale = glm::vec3( 1.0f ), bool invalidateOld = false );

		static void AddRigidBody( Entity& entity );
	public:
		static physx::PxPhysics& GetPhysics();
		static physx::PxScene& GetPhysXScene();

		static physx::PxAllocatorCallback& GetAllocator();
		static PhysXContact& GetPhysXContact();
	protected:
	private:
	};

	class PhysXErrorCallback : public physx::PxErrorCallback, public RefCounted
	{
	public:
		void reportError( physx::PxErrorCode::Enum code, const char* message, const char* file, int line ) override;
	};

}