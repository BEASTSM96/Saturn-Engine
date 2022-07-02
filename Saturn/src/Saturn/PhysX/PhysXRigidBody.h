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

#include "Saturn/Core/Base.h"

#include "PhysXCore.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <PxPhysicsAPI.h>

namespace Saturn {

	class Entity;
	class PhysXRigidbody : CountedObj
	{
	public:
		PhysXRigidbody( Entity& Owner, glm::vec3& Position, glm::vec3& Rotation );
		~PhysXRigidbody();

		void Create();

		void SetKinematic( bool kinematic );
		void ApplyForce( glm::vec3 ForceAmount, ForceMode Type );
		void SetUserData( Entity& rEntity );
		void UseCCD( bool ccd );
		void SetMass( float mass );
		void Rotate( glm::vec3 rotation );
		void AddActorToScene();
		bool IsKinematic() { return m_Kinematic; }
		bool AttachShape( physx::PxShape& rShape );
		glm::vec3 GetPosition();
		glm::vec3 GetRotation();
		glm::mat4 GetTransform();

		physx::PxRigidActor* GetActor() { return m_Body; }
	private:
		physx::PxRigidActor* m_Body;

		int m_Mas = 1.0;
		bool m_UseCCD = false;
		bool m_Kinematic = false;

		Entity& m_Owner;
	};
}