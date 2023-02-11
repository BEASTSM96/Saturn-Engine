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
#include "PhysXRuntime.h"

namespace Saturn {

	static physx::PxScene* s_Scene;
	
	void PhysXRuntime::Update( Timestep ts, Scene& scene )
	{
		s_Scene->simulate( ts );
		s_Scene->fetchResults( true );
		scene.OnUpdatePhysics( ts );
	}

	void PhysXRuntime::Clear()
	{
		s_Scene = nullptr;
	}

	void PhysXRuntime::CreateScene()
	{
		SAT_CORE_ASSERT( s_Scene == nullptr, "Scene already created!" );
		s_Scene = PhysXFnd::Get().CreateScene();
	}

	void PhysXRuntime::AddRigidbody( Entity& entity )
	{
		PhysXFnd::Get().AddRigidBody( entity );
	}

	void* PhysXRuntime::GetScene()
	{
		return s_Scene;
	}

}