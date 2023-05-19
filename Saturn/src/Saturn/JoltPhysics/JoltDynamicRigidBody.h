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

#include "Saturn/Core/Base.h"

#include "JoltPhysicsBodyBase.h"

#include "JoltBase.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace Saturn {

	class JoltDynamicRigidBody : public JoltPhysicsBodyBase
	{
	public:
		JoltDynamicRigidBody( Entity entity );
		~JoltDynamicRigidBody();

		virtual void Create( const glm::vec3& Position, const glm::vec3& Rotation ) override;
		void DestroyBody();

		void SetBody( JPH::Body* body );

		void SetKinematic( bool kinematic );
		void ApplyForce( glm::vec3 ForceAmount, ForceMode Type );
		void SetUserData( Entity& rEntity );
		void UseCCD( bool ccd );
		void SetMass( float mass );
		void Rotate( const glm::vec3& rRotation );
		void SetLinearVelocity( glm::vec3 linearVelocity );
		void SetLinearDrag( float value );
		bool IsKinematic() { return m_Kinematic; }

		void AttachBox ( const glm::vec3& Scale = glm::vec3(0.0f) );
		void AttachSphere ( float Extents );
		void AttachCapsule ( float Extents, float Height );

		void SyncTransform();

	private:
		JPH::Body* m_Body = nullptr;

		struct
		{
			PhysicsShape ShapeType;

			glm::vec3 Scale; // Box collider

			float Extents; // Sphere & Capsule
			float Height; // Capsule
		} PendingShapeInfo{};

		bool m_Kinematic = false;
	};
}