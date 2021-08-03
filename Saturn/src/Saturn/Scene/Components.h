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
#include "Saturn/Physics/PhysX/PhysXRigidBody.h"

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
		glm::vec3  Rotation ={ 0.0f, 0.0f, 0.0f };
		glm::vec3  Scale	={ 1.0f , 1.0f, 1.0f };

		glm::vec3 Up ={ 0.0f, 1.0f, 0.0f };
		glm::vec3 Right ={ 1.0f, 0.0f, 0.0f };
		glm::vec3 Forward ={ 0.0f, 0.0f, -1.0f };

		TransformComponent( void ) = default;
		TransformComponent( const TransformComponent& ) = default;
		TransformComponent( const glm::vec3 & Position )
			: Position( Position )
		{
		}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4( glm::quat( Rotation ) );

			return glm::translate( glm::mat4( 1.0f ), Position )
				* rotation
				* glm::scale( glm::mat4( 1.0f ), Scale );
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
		SceneCamera Camera;
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
		std::string EnvironmentFilepath = "";

		float Intensity = 1.0f;
		float Angle = 0.0f;

		SkyLightComponent() = default;
		SkyLightComponent( const SkyLightComponent& ) = default;
		SkyLightComponent( const Saturn::Environment& environment )
			: SkyLightEnvironment( environment )

		{
		}
	};

	/** @brief A PointLightComponent.
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
	struct PointLightComponent : Component
	{
		glm::vec3 Position ={ 0.0f , 0.0f, 0.0f };
		bool Main = true;

		PointLightComponent() = default;
		PointLightComponent( const PointLightComponent& ) = default;
		PointLightComponent( const glm::vec3& position )
			: Position( position )

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
		MeshComponent( const MeshComponent& other ) = default;
		MeshComponent( Ref<Saturn::Mesh> & model )
			: Mesh( model )
		{
		}

		operator Ref<Saturn::Mesh>() { return Mesh; }
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance ={ 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
		bool CastShadows = true;
		bool SoftShadows = true;
		float LightSize = 0.5f; // For PCSS
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

	struct PhysXRigidbodyComponent : Component
	{
		bool isKinematic;
		bool UseCCD;
		int Mass;


		PhysXRigidbody* m_Rigidbody;

		PhysXRigidbodyComponent() = default;
		PhysXRigidbodyComponent( bool IsKinematic ) : isKinematic( IsKinematic ) { }
	};

	struct PhysXBoxColliderComponent : Component
	{
		glm::vec3 Extents ={ 1.0f, 1.0f, 1.0f };	
		glm::vec3 Offset ={ 0.0f, 0.0f, 0.0f };

		Ref<Mesh> DebugMesh;

		bool IsTrigger = false;

		PhysXBoxColliderComponent() = default;
		PhysXBoxColliderComponent( const glm::vec3& extents ) : Extents( extents ) { }
	};

	struct PhysXSphereColliderComponent : Component
	{
		glm::vec3 Offset ={ 0.0f, 0.0f, 0.0f };
		float Radius = 1.0f;

		bool IsTrigger = false;

		Ref<Mesh> DebugMesh;

		PhysXSphereColliderComponent() = default;
		PhysXSphereColliderComponent( float radius ) : Radius( radius ) { }
	};

	struct PhysXCapsuleColliderComponent : Component
	{
		glm::vec3 Offset ={ 0.0f, 0.0f, 0.0f };

		float Radius = 1.0f;
		float Height = 1.0f;

		bool IsTrigger = false;

		Ref<Mesh> DebugMesh;

		PhysXCapsuleColliderComponent() = default;
		PhysXCapsuleColliderComponent( float radius ) : Radius( radius ) { }
	};

	struct PhysXMaterialComponent : Component
	{
		float StaticFriction = 0.6f;
		float DynamicFriction = 0.6f;
		float Restitution = 0.0f;
	};

	struct PhysXMeshColliderComponent : Component
	{
		Ref<Mesh> CollisionMesh;
		std::vector<Ref<Mesh>> ProcessedMeshes;
		bool IsConvex = false;
		bool IsTrigger = false;
		bool OverrideMesh = false;

		Ref<Mesh> DebugMesh;

		PhysXMeshColliderComponent() = default;
		PhysXMeshColliderComponent( const PhysXMeshColliderComponent & other ) = default;
		PhysXMeshColliderComponent( const Ref<Mesh>&mesh )
			: CollisionMesh( mesh )
		{
		}

		operator Ref<Mesh>() { return CollisionMesh; }
	};

	//
	// ScriptComponents
	//
	struct ScriptComponent : Component
	{
		std::string ModuleName = "ExampleApp.Null";
	};
}
