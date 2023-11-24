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
#include "Core/ClassMetadataHandler.h"

namespace Saturn {

	Character::Character()
	{
		m_PlayerInputController = Ref<PlayerInputController>::Create();

		m_MouseSensitivity = 3.0f;
		m_MouseUpMovement = 0.0f;

		AddComponent<StaticMeshComponent>();
		AddComponent<RigidbodyComponent>();
		AddComponent<CapsuleColliderComponent>();
	}

	Character::~Character()
	{
		m_PlayerInputController = nullptr;
	}

	void Character::BeginPlay()
	{
		Super::BeginPlay();

		SetupInputBindings();

		m_PlayerInputController->BindAction( "Jump", SAT_BIND_EVENT_FN( MoveForward ) );
		m_PlayerInputController->BindAction( "Jump", SAT_BIND_EVENT_FN( MoveBack ) );
		m_PlayerInputController->BindAction( "Jump", SAT_BIND_EVENT_FN( MoveLeft ) );
		m_PlayerInputController->BindAction( "Jump", SAT_BIND_EVENT_FN( MoveRight ) );

		m_RigidBody = GetComponent<RigidbodyComponent>().Rigidbody;

		if( m_RigidBody )
		{
			m_RigidBody->SetOnCollisionHit( SAT_BIND_EVENT_FN( OnMeshHit ) );
			m_RigidBody->SetOnCollisionExit( SAT_BIND_EVENT_FN( OnMeshExit ) );
		}

		m_CameraEntity = m_Scene->GetMainCameraEntity();
	}

	void Character::OnUpdate( Timestep ts )
	{
		Super::OnUpdate( ts );

		m_MovementDirection.x = 0.0f;
		m_MovementDirection.y = 0.0f;

		// Update player input
		m_PlayerInputController->Update();

		HandleRotation( ts );
		HandleMovement();
	}

	void Character::OnPhysicsUpdate( Timestep ts )
	{
		Super::OnPhysicsUpdate( ts );

		if( Input::Get().GetCursorMode() != RubyCursorMode::Locked )
			return;

		TransformComponent& tc = GetComponent<TransformComponent>();

		auto& up = tc.Up;

		tc.SetRotation( tc.GetRotationEuler() += m_MouseUpMovement * up * 0.05f );

		glm::vec3 right, forward;
		right = CalculateRight();
		forward = CalculateForward();

		glm::vec3 Direction = right * m_MovementDirection.x + forward * m_MovementDirection.y;

		if( glm::length( Direction ) > 0.0f )
		{
			glm::vec3 normalMove = glm::normalize( Direction );
			normalMove *= 70.0f;

			m_RigidBody->ApplyForce( normalMove, ForceMode::Force );
		}
	}

	void Character::OnMeshHit( Ref<Entity> Other )
	{

	}

	void Character::OnMeshExit( Ref<Entity> Other )
	{

	}

	glm::vec3 Character::CalculateRight()
	{
		auto& r = GetComponent<TransformComponent>().Right;

		TransformComponent result = m_Scene->GetWorldSpaceTransform( m_CameraEntity );

		return result.GetRotation() * r;
	}

	glm::vec3 Character::CalculateForward()
	{
		auto& f = GetComponent<TransformComponent>().Forward;

		TransformComponent result = m_Scene->GetWorldSpaceTransform( m_CameraEntity );

		return result.GetRotation() * f;
	}

	void Character::HandleRotation( Timestep ts )
	{
		if( Input::Get().GetCursorMode() != RubyCursorMode::Locked )
			return;

		TransformComponent& tc = m_CameraEntity->GetComponent<TransformComponent>();
		auto rot = tc.GetRotationEuler();

		glm::vec2 currentMousePos = Input::Get().MousePosition();
		glm::vec2 delta = m_LastMousePos - currentMousePos;

		if( delta.x != 0.0f )
			m_MouseUpMovement = delta.x * m_MouseSensitivity * ts.Seconds();

		float xRotation = delta.y * ( m_MouseSensitivity * 0.05f ) * ts.Seconds();

		if( xRotation != 0.0f )
			tc.SetRotation( glm::vec3( xRotation, 0.0f, 0.0f ) );

		tc.SetRotation( glm::radians( glm::vec3( glm::clamp( glm::degrees( rot.x ), -80.0f, 80.0f ), 0.0f, 0.0f ) ) );

		m_LastMousePos = currentMousePos;
	}

	void Character::MoveForward()
	{
		m_MovementDirection.y = 1.0f;
	}

	void Character::MoveBack()
	{
		m_MovementDirection.y = -1.0f;
	}

	void Character::MoveLeft()
	{
		m_MovementDirection.x = -1.0f;
	}

	void Character::MoveRight()
	{
		m_MovementDirection.x = 1.0f;
	}

}

//////////////////////////////////////////////////////////////////////////
// Build Tool
//////////////////////////////////////////////////////////////////////////

Saturn::Entity* _Z_Create_Character()
{
	using namespace Saturn;

	Saturn::Ref<Character> Target = Saturn::Ref<Character>::Create();
	Saturn::Ref<Saturn::Entity> TargetReturn = Target.As<Saturn::Entity>();
	return TargetReturn.Get();
}
