/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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

#include "PhysXCore.h"

#include "PxPhysicsAPI.h"

#include <stdint.h>
	
namespace Saturn {

	class PhysXContact : public physx::PxSimulationEventCallback, public CountedObj
	{
	public:
		void onConstraintBreak( physx::PxConstraintInfo* pConstraints, physx::PxU32 Count ) override;
		void onWake( physx::PxActor** ppActors, physx::PxU32 Count ) override;
		void onSleep( physx::PxActor** ppActors, physx::PxU32 Count ) override;
		void onContact( const physx::PxContactPairHeader& rPairHeader, const physx::PxContactPair* pPairs, physx::PxU32 Pairs ) override;
		void onTrigger( physx::PxTriggerPair* pPairs, physx::PxU32 Count ) override;
		void onAdvance( const physx::PxRigidBody* const* pBodyBuffer, const physx::PxTransform* PoseBuffer, const physx::PxU32 Count ) override;
	};

	class PhysXAssertCallback : public physx::PxAssertHandler, public CountedObj
	{
	public:
		virtual void operator()( const char* pError, const char* pFile, int Line, bool& rIngore ) override;
	};

	class PhysXErrorCallback : public physx::PxErrorCallback, public CountedObj 
	{
	public:
		void reportError( physx::PxErrorCode::Enum ErrorCode, const char* pErrorString, const char* pFile, int Line ) override;
	};

	using PhysXShapeMap = std::vector<physx::PxShape*>;

	class PhysXFnd
	{
		SINGLETON( PhysXFnd );
	public:
		PhysXFnd() { Init(); }
		~PhysXFnd() { Terminate(); }

		void CreateBoxCollider( Entity& rEntity, physx::PxRigidActor& rActor );
		void CreateSphereCollider( Entity& rEntity, physx::PxRigidActor& rActor );
		void CreateCapsuleCollider( Entity& rEntity, physx::PxRigidActor& rActor );
		void CreateMeshCollider( Entity& rEntity, physx::PxRigidActor& rActor );
		
		void AddRigidBody( Entity& entity );

		physx::PxScene* CreateScene();

		bool Raycast( glm::vec3& Origin, glm::vec3& Direction, float Distance, RaycastResult* pResult );
		
		physx::PxPhysics* GetPhysics();
		physx::PxAllocatorCallback& GetAllocator();

	private:
		void Init();
		void Terminate();
	};
}