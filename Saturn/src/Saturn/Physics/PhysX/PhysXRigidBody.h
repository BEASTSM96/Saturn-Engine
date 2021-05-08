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

namespace Saturn {

	enum class ForceType
	{
		Force = 1,
		Acceleration = 2,
		Impulse = 3,
		VelocityChange = 4
	};	

	class Entity;
	class PhysXRigidbody : public RefCounted
	{
	public:
		PhysXRigidbody( Entity entity, glm::vec3 pos, glm::quat rot );
		~PhysXRigidbody();

		glm::mat4 GetTransform()
		{
			auto xpos = m_Body->getGlobalPose().p.x;
			auto ypos = m_Body->getGlobalPose().p.y;
			auto zpos = m_Body->getGlobalPose().p.z;

			auto xq = m_Body->getGlobalPose().q.x;
			auto yq = m_Body->getGlobalPose().q.y;
			auto zq = m_Body->getGlobalPose().q.z;

			auto pos =	glm::mat4( xq * ypos * zpos );
			auto rot =	glm::mat4( xpos * yq * zq );

			return glm::mat4( pos * rot );

		}

		glm::vec3 GetPos();

		glm::quat GetRot();

		void SetKinematic( bool kinematic );

		bool IsKinematic();

		void ApplyForce( glm::vec3 force, ForceType type );
		void AttachShape( physx::PxShape& shape );
		void AddActorToScene();

		physx::PxRigidActor& GetPxBody() { return *m_Body; }
		physx::PxRigidActor* m_Body;
	protected:

	private:
		bool m_Kinematic = false;

		friend class PhysXCollider;
		friend class PhysXMaterial;

	};
}

#endif // #ifdef USE_NVIDIA
