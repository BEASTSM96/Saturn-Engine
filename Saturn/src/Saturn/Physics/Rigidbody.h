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

#include <glm/glm.hpp>

#pragma warning(push)
#include <reactphysics3d/reactphysics3d.h>
#pragma  warning(pop)

namespace Saturn {

	class PhysicsScene;

	class Rigidbody : public RefCounted
	{
	public:
		Rigidbody( PhysicsScene* scene, bool useGravity = true, const glm::vec3& position = glm::vec3( 0.0f ), const glm::vec3& rotation = glm::vec3( 0.0f ) );
		~Rigidbody();

		void AddBoxCollider( const glm::vec3& halfExtents );
		void AddSphereCollider( float radius );

		glm::vec3 GetPosition( void );
		glm::quat GetRotation( void );

		inline bool GetKinematic( void ) const { return m_isKinematic; }
		void SetKinematic( bool set );

		float GetMass( void ) const;
		void SetMass( float mass );

		//
		// Manipulation (rb)
		void ApplyForce( glm::vec3 force );
		void ApplyForceAtLocation( glm::vec3 force, glm::vec3 location );
		void ApplyTorque( glm::vec3 torque );
		void SetCOG( glm::vec3 cog );
		void UseGravity( bool use );
	private:
		PhysicsScene* m_scene;

		std::vector<reactphysics3d::Collider*> m_colliders;
		bool m_isKinematic;
		reactphysics3d::RigidBody* m_body;
	private:
		friend class Scene;
	};
}