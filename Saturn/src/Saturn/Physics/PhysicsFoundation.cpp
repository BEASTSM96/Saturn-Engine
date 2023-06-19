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
#include "PhysicsFoundation.h"

namespace Saturn {

	PhysicsFoundation::PhysicsFoundation()
	{
		SingletonStorage::Get().AddSingleton<PhysicsFoundation>( this );
	}

	PhysicsFoundation::~PhysicsFoundation()
	{
		Terminate();
	}

	void PhysicsFoundation::Init()
	{
		physx::PxTolerancesScale Scale;
		Scale.length = 10.0f;

		m_Foundation = PxCreateFoundation( PX_PHYSICS_VERSION, m_AllocatorCallback, m_ErrorCallback );
		m_Physics = PxCreatePhysics( PX_PHYSICS_VERSION, *m_Foundation, Scale, true );

		m_Pvd = PxCreatePvd( *m_Foundation );
		m_Cooking = PxCreateCooking( PX_PHYSICS_VERSION, *m_Foundation, Scale );
		
		physx::PxSetAssertHandler( m_AssertCallback );
	}

#define TERMINATE_ITEM( x ) if(x) x->release(); x = nullptr

	void PhysicsFoundation::Terminate()
	{
		TERMINATE_ITEM( m_Physics );
		TERMINATE_ITEM( m_Foundation );
	}

}