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

#include "Saturn/Scene/Entity.h"
#include "Core/GameScript.h"

#include "PlayerInputController.h"

namespace Saturn {

	// TODO: Make this an SCLASS macro as well be able to spawn this correctly.
	//       However this is need more work because we'll need to use the Build Tool to compile the engine which is not supported right now.
	class Character : public Entity
	{
		//////////////////////////////////////////////////////////////////////////
		// This is here because we aren't using the Build Tool.
		//////////////////////////////////////////////////////////////////////////
		DECLARE_CLASS( Character, Entity );

	public:
		Character();
		~Character();

		virtual void SetupInputBindings() {};

	public:
		//////////////////////////////////////////////////////////////////////////
		// SClass overrides
		//////////////////////////////////////////////////////////////////////////

		virtual void BeginPlay() override;
		virtual void OnUpdate( Timestep ts ) override;
		virtual void OnPhysicsUpdate( Timestep ts ) override;

		Ref<StaticMesh>& GetMesh() { return m_Mesh; }
		const Ref<StaticMesh>& GetMesh() const { return m_Mesh; }
		
	protected:
		//////////////////////////////////////////////////////////////////////////
		// Physics
		//////////////////////////////////////////////////////////////////////////
		void OnMeshHit( Ref<Entity> Other );
		void OnMeshExit( Ref<Entity> Other );

	private:
		//////////////////////////////////////////////////////////////////////////
		// Movement
		//////////////////////////////////////////////////////////////////////////

		glm::vec3 CalculateRight();
		glm::vec3 CalculateForward();

		void HandleMovement() {}
		void HandleRotation( Timestep ts );

		void MoveForward();
		void MoveBack();
		void MoveLeft();
		void MoveRight();

		glm::vec2 m_MovementDirection{};
		glm::vec2 m_LastMousePos{};

		float m_MouseUpMovement = 0.0f;
		float m_MouseSensitivity = 0.0f;

	private:
		// TODO: Change to a base mesh class, we don't know what the user will have.
		Ref<StaticMesh> m_Mesh;
		// TODO: Move this to a movement component.
		PhysicsRigidBody* m_RigidBody = nullptr;

		Ref<Entity> m_CameraEntity = nullptr;

		Ref<PlayerInputController> m_PlayerInputController = nullptr;
	};
}

// Temp
// DO NOT USE! This is only here because we are not using the Build Tool for the engine however any game classes will need this function to constructed properly
Saturn::Entity* _Z_Create_Character();