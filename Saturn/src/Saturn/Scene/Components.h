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
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Saturn/Scene/SceneCamera.h"
#include "Saturn/Core/UUID.h"
#include "Saturn/Physics/Rigidbody.h"
#include "Saturn/Physics/PhysX/PhysXRigidBody.h"
#include "Saturn/Physics/PhysX/PhysXBoxCollider.h"
#include "Saturn/Physics/PhysX/PhysXSphereCollider.h"

namespace Saturn {

	struct Component {};

	/** @brief A TransformComponent.
	*
	* @code
	*
	* glm::mat4 Transform (default 1.0f)
	*
	* TransformComponent()
	* TransformComponent(const TransformComponent&)
	* TransformComponent(const glm::mat4 & Transform)
	*
	* 
	*	operator glm::mat4& ()
	*
	*	operator const glm::mat4& ()
	* 
	* @endcode
	*/

	struct TransformComponent : Component
	{
		glm::vec3  Position ={ 0.0f , 0.0f, 0.0f };
		glm::quat  Rotation;
		glm::vec3  Scale	={ 1.0f , 1.0f, 1.0f };

		TransformComponent( void ) = default;
		TransformComponent( const TransformComponent& ) = default;
		TransformComponent( const glm::vec3 & Position )
			: Position( Position )
		{
		}

		glm::mat4 GetTransform() const
		{
			glm::mat4 position = glm::translate( glm::mat4( 1.0f ), Position );
			glm::mat4 rotation = glm::toMat4( Rotation );
			glm::mat4 scale = glm::scale( glm::mat4( 1.0f ), Scale );

			if ( Position.x > FLT_MAX )
			{
				SAT_CORE_ASSERT(false, " Position.x > FLT_MAX" );
			}

			if( Position.y > FLT_MAX )
			{
				SAT_CORE_ASSERT( false, " Position.y > FLT_MAX" );
			}

			if( Position.z > FLT_MAX )
			{
				SAT_CORE_ASSERT( false, " Position.z > FLT_MAX" );
			}

			if( Rotation.x > FLT_MAX )
			{
				SAT_CORE_ASSERT( false, " Rotation.x > FLT_MAX" );
			}

			if( Rotation.y > FLT_MAX )
			{
				SAT_CORE_ASSERT( false, " Rotation.y > FLT_MAX" );
			}

			if( Rotation.z > FLT_MAX )
			{
				SAT_CORE_ASSERT( false, " Rotation.z > FLT_MAX" );
			}

			if( Scale.x > FLT_MAX )
			{
				SAT_CORE_ASSERT( false, " Scale.x > FLT_MAX" );
			}

			if( Scale.y > FLT_MAX )
			{
				SAT_CORE_ASSERT( false, " Scale.y > FLT_MAX" );
			}

			if( Scale.z > FLT_MAX )
			{
				SAT_CORE_ASSERT( false, " Scale.z > FLT_MAX" );
			}

			return position * rotation * scale;

			//return glm::translate(glm::mat4(1.0f), Position)
			//	* rotation
			//	* glm::scale(glm::mat4(1.0f), Scale);
		}

		operator glm::mat4& ( ) { return GetTransform(); }
		operator const glm::mat4& ( ) const { return GetTransform(); }

	};


	/** @brief A SpriteRendererComponent.
	*
	* @code
	* 
	* glm::vec4 Color (default  1.0f)
	* 
	* SpriteRendererComponent()
	* SpriteRendererComponent(const SpriteRendererComponent&)
	* SpriteRendererComponent(const glm::vec4& color)
	* 
	* @endcode
	*/
	struct SpriteRendererComponent : Component
	{
		glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent( const SpriteRendererComponent& ) = default;
		SpriteRendererComponent( const glm::vec4& color )
			: Color( color ) 

		{
		}
	};

	/** @brief A CameraComponent.
	*
	* @code
	*
	* Ref<SceneCamera> Camera
	* bool Primary
	* bool FixedAspectRatio
	*
	* CameraComponent()
	* CameraComponent(const CameraComponent&)
	*
	* @endcode
	*/
	struct CameraComponent : Component
	{
		Ref<Saturn::SceneCamera> Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent( const CameraComponent& ) = default;
	};


	/** @brief A TagComponent.
	*
	* @code
	*
	* std::string Tag;
	*
	* TagComponent()
	* TagComponent(const TagComponent&) = default
	* TagComponent(const std::string& tag)
	*
	* @endcode
	*/
	struct TagComponent : Component
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent( const TagComponent& ) = default;
		TagComponent( const std::string& tag )
			: Tag( tag )

		{
		}
	};

	/** @brief A SkyLightComponent.
	*
	* @code
	*
	* Environment SkyLightEnvironment;
	*
	* SkyLightComponent()
	* SkyLightComponent(const SkyLightComponent&)
	* SkyLightComponent(const std::string& tag)
	*
	* @endcode
	*/
	struct SkyLightComponent : Component
	{
		Saturn::Environment SkyLightEnvironment;

		SkyLightComponent() = default;
		SkyLightComponent( const SkyLightComponent& ) = default;
		SkyLightComponent( const Saturn::Environment& environment )
			: SkyLightEnvironment( environment )

		{
		}
	};

	/** @brief A IdComponent.
	*
	* @code
	* 
	* UUID ID;
	*
	* IdComponent()
	* IdComponent(const IdComponent&) = default
	* IdComponent(const UUID& uuid)
	* 
	* @endcode
	*/
	struct IdComponent : Component
	{
		UUID ID;

		IdComponent() = default;
		IdComponent( const IdComponent& ) = default;
		IdComponent( const UUID& uuid )
			: ID( uuid ) 
		{
		}
	};


	/** @brief A MeshComponent.
	*
	* @code
	*
	* Model * Model;
	*
	*
	* MeshComponent()
	* MeshComponent(const MeshComponent&) = default
	* MeshComponent(const Model& Model)
	*
	*
	* @endcode
	*/

	class Mesh;

	struct MeshComponent : Component
	{
		Ref<Saturn::Mesh> Mesh;

		MeshComponent() = default;
		MeshComponent( const MeshComponent & other ) = default;
		MeshComponent( Ref<Saturn::Mesh> & model )
			: Mesh( model )
		{
		}

		operator Ref<Saturn::Mesh>() { return Mesh; }
	};

	/** @brief RelationshipComponent.
	*
	*/

	struct RelationshipComponent : Component
	{
		entt::entity Parent{ entt::null };
		std::unordered_set < entt::entity > Children;

		glm::mat4 RelativeTransform = glm::mat4{ 0.0f };
	};

	struct PhysicsComponent : Component
	{
		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec3 Force;
		float Mass = 1.0f;
	};

	struct BoxColliderComponent : Component
	{
		glm::vec3 Extents;

		BoxColliderComponent() = default;
		BoxColliderComponent( const glm::vec3 & extents ) : Extents( extents ) { }
	};

	struct SphereColliderComponent : Component
	{
		float Radius;

		SphereColliderComponent() = default;
		SphereColliderComponent( float radius ) : Radius( radius ) { }
	};

	struct RigidbodyComponent : Component
	{
		bool isKinematic;

		Rigidbody* m_body;

		RigidbodyComponent() = default;
		RigidbodyComponent( Rigidbody* m_Body, bool IsKinematic ) : m_body( m_Body ), isKinematic(IsKinematic) { }
	};

	struct PhysXRigidbodyComponent : Component
	{
		bool isKinematic;

		PhysXRigidbody* m_body;

		PhysXRigidbodyComponent() = default;
		PhysXRigidbodyComponent( PhysXRigidbody* m_Body, bool IsKinematic ) : m_body( m_Body ), isKinematic( IsKinematic ) { }
	};

	struct PhysXBoxColliderComponent : Component
	{
		glm::vec3 Extents ={ 1.0f, 1.0f, 1.0f };

		PhysXBoxCollider* m_body;

		PhysXBoxColliderComponent() = default;
		PhysXBoxColliderComponent( PhysXBoxCollider* m_Body, const glm::vec3& extents ) : m_body( m_Body ), Extents( extents ) { }
	};

	struct PhysXSphereColliderComponent : Component
	{
		float Radius = 1.0f;

		PhysXSphereCollider* m_body;

		PhysXSphereColliderComponent() = default;
		PhysXSphereColliderComponent( PhysXSphereCollider* m_Body, float radius ) : m_body( m_Body ), Radius( radius ) { }
	};

	//
	// ScriptComponents
	//
	class ScriptableEntity;


	//For coding in c++ 
	//@see https://beastsm96.github.io/Saturn-Engine/api/v0.a01/Scene/Components
	struct NativeScriptComponent : Component
	{
	
		ScriptableEntity* Instance = nullptr;

		//TODO:Going to remove
		template<typename T>
		void Bind()
		{
		}
	};
}
