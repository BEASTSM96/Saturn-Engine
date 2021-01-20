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

namespace Saturn {
	
	class Scene;

	class PhysXScene : public RefCounted
	{
	public:
		PhysXScene( Scene* scene );
		~PhysXScene();

		void Update( Timestep ts );
		void RenderPhysXDebug( const SceneCamera& camera );
		void RenderPhysXDebug( const EditorCamera& camera );

		PhysXErrorCallback m_DefaultErrorCallback;
		physx::PxDefaultAllocator m_DefaultAllocatorCallback;
		physx::PxFoundation* m_Foundation = NULL;
		physx::PxDefaultCpuDispatcher* m_Dispatcher = NULL;
		physx::PxCooking* m_Cooking = NULL;
		physx::PxPhysics* m_Physics = NULL;
		physx::PxScene* m_PhysXScene = NULL;
		physx::PxPvd* m_PVD = NULL;

	protected:
		Scene* m_Scene;

	private:
		_declspec( align( 16 ) ) std::uint8_t MemoryBlock[65536];
	};
}

#else
#error You need to use nvidia for physx!
#endif // USE_NVIDIA
