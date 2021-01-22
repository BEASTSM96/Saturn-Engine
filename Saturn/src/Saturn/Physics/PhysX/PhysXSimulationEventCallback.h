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

#ifdef USE_NVIDIA

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <physx/PxPhysicsAPI.h>

#include "PhysXScene.h"
#include "PhysXCollider.h"

namespace Saturn {

	class PhysXSimulationEventCallback : public physx::PxSimulationEventCallback
	{
	public:
		PhysXSimulationEventCallback( physx::PxSceneDesc PxSceneDesc );
		~PhysXSimulationEventCallback();

		void SetSceneContext( PhysXScene* scene );

	public:
		void onConstraintBreak( physx::PxConstraintInfo* constraints, physx::PxU32 count ) override;
		void onWake( physx::PxActor** actors, physx::PxU32 count ) override;
		void onSleep( physx::PxActor** actors, physx::PxU32 count ) override;
		void onContact( const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs ) override;
		void onTrigger( physx::PxTriggerPair* pairs, physx::PxU32 count ) override;
		void onAdvance( const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count ) override;

	protected:
		std::function<void( PhysXCollider, PhysXCollider )> m_OnContact;
		std::function<void()> m_OnTrigger;
		std::function<void()> m_OnWake;
		std::function<void()> m_OnSleep;
		std::function<void()> m_OnConstraintBreak;
	private:
		PhysXScene* m_Scene;
	};

}

#endif