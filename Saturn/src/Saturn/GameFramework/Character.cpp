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
#include "Character.h"

#include "Saturn/Physics/PhysicsRigidBody.h"

namespace Saturn {

	Character::Character()
	{

	}

	Character::~Character()
	{

	}

	void Character::BeginPlay()
	{
		Super::BeginPlay();

		m_RigidBody = GetComponent<RigidbodyComponent>().Rigidbody;

		m_RigidBody->SetOnCollisionHit( SAT_BIND_EVENT_FN( OnMeshHit ) );
		m_RigidBody->SetOnCollisionExit( SAT_BIND_EVENT_FN( OnMeshExit ) );
	}

	void Character::OnUpdate( Timestep ts )
	{
		Super::OnUpdate( ts );

		HandleRotation( ts );
		HandleMovement();
	}

	void Character::OnPhysicsUpdate( Timestep ts )
	{
		Super::OnPhysicsUpdate( ts );
	}

	void Character::OnMeshHit( Entity Other )
	{

	}

	void Character::OnMeshExit( Entity Other )
	{

	}

	glm::vec3 Character::CalculateRight()
	{
		return {};
	}

	glm::vec3 Character::CalculateForward()
	{
		return {};
	}

	void Character::HandleMovement()
	{

	}

	void Character::HandleRotation( Timestep ts )
	{

	}

}

//////////////////////////////////////////////////////////////////////////
// Build Tool
//////////////////////////////////////////////////////////////////////////

Saturn::SClass* _Z_Create_Character()
{
	using namespace Saturn;

	SClass* pClass = new SClass();

	pClass->m_SuperClass = _Z_Create_Entity();

	Character* pChar = (Character*)pClass;
	pChar->__DefaultConstructor();

	return pChar;
}
