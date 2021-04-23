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

#include "Saturn/Core/Base.h"
#include "Saturn/Core/Timestep.h"

#ifdef USE_NVIDIA

#include <physx/PxPhysicsAPI.h>
#include <physx/PxFoundation.h>
#include <physx/PxScene.h>

#include "ErrorCallback.h"
#include "Saturn/Renderer/Camera.h"
#include "Saturn/Editor/EditorCamera.h"
#include "Saturn/Scene/SceneCamera.h"
#include "PhysXHelpers.h"

namespace Saturn {
	
	class Scene;
	class PhysXSimulationEventCallback;

	class PhysXScene : public RefCounted
	{
	public:
		PhysXScene( Scene* scene );
		~PhysXScene();

		static void Init();

		void Update( Timestep ts );

	public:
		physx::PxPhysics& GetPhysics();
		physx::PxScene& GetPhysXScene();

		physx::PxAllocatorCallback& GetAllocator();
		const physx::PxAllocatorCallback& GetAllocator() const;

		const physx::PxPhysics& GetPhysics() const;
		const physx::PxScene& GetPhysXScene() const;

	protected:
		Scene* m_Scene;

	private:
		_declspec( align( 16 ) ) std::uint8_t MemoryBlock[65536];
	};

	class PhysXContact : public physx::PxSimulationEventCallback
	{
	public:
		virtual void onConstraintBreak( physx::PxConstraintInfo* constraints, physx::PxU32 count ) override;
		virtual void onWake( physx::PxActor** actors, physx::PxU32 count ) override;
		virtual void onSleep( physx::PxActor** actors, physx::PxU32 count ) override;
		virtual void onContact( const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs ) override;
		virtual void onTrigger( physx::PxTriggerPair* pairs, physx::PxU32 count ) override;
		virtual void onAdvance( const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count ) override;
	};

}

#else
#error You need to use nvidia for physx!
#endif // USE_NVIDIA