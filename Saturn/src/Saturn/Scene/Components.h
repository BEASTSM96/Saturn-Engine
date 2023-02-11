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

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Saturn/Vulkan/EnvironmentMap.h"
#include "Saturn/PhysX/PhysXRigidBody.h"

#include "Saturn/Core/Math.h"

#include "Saturn/Vulkan/Mesh.h"

#include "Saturn/Core/UUID.h"

#include "EntityVisibility.h"

#include "Saturn/Core/Renderer/SceneCamera.h"

#include <string>

namespace Saturn {

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

	struct TransformComponent
	{
		glm::vec3  Position ={ 0.0f , 0.0f, 0.0f };
		glm::vec3  Rotation ={ 0.0f, 0.0f, 0.0f };
		glm::vec3  Scale	={ 1.0f , 1.0f, 1.0f };

		glm::vec3 Up ={ 0.0f, 1.0f, 0.0f };
		glm::vec3 Right ={ 1.0f, 0.0f, 0.0f };
		glm::vec3 Forward ={ 0.0f, 0.0f, -1.0f };

		TransformComponent( void ) = default;
		TransformComponent( const TransformComponent& ) = default;
		TransformComponent( const glm::vec3& Position )
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
		
		void SetTransform( const glm::mat4& rTransfrom )
		{
			Math::DecomposeTransform( rTransfrom, Position, Rotation, Scale );
		}

		operator glm::mat4 ( ) { return GetTransform(); }
		operator const glm::mat4& ( ) const { return GetTransform(); }
	};

	struct VisibilityComponent
	{
		Visibility visibility = Visibility::Visible;

		VisibilityComponent() = default;
		VisibilityComponent( const VisibilityComponent& other ) = default;
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
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent( const TagComponent& ) = default;
		TagComponent( const std::string& tag )
			: Tag( tag )

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
	struct IdComponent
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
	* Ref of type Mesh
	*
	*
	* MeshComponent()
	* MeshComponent(const MeshComponent&) = default
	* MeshComponent( Ref of type Mesh )
	*
	*
	* @endcode
	*/
	struct MeshComponent
	{
		Ref<Saturn::Mesh> Mesh;

		MeshComponent() = default;
		MeshComponent( const MeshComponent& other ) = default;
		MeshComponent( Ref<Saturn::Mesh>& model )
			: Mesh( model )
		{
		}

		operator Ref<Saturn::Mesh>() { return Mesh; }
	};

	struct LightComponent
	{
		float Intensity = 5.0f;
		glm::vec3 Color = { 1.0f, 1.0f, 1.0f };

		// TODO: Light Types?

		LightComponent() = default;
		LightComponent( const LightComponent& other ) = default;

		operator glm::vec4 ( ) { return std::move( glm::vec4( Color.x, Color.z, Color.y, 1.0f ) ); }
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		
		float Intensity = 1.0f;
		bool CastShadows = true;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool MainCamera;
	};

	// Preetham sky
	struct SkylightComponent 
	{
		EnvironmentMap Map;

		bool DynamicSky = false;

		float Turbidity = 2.0f;
		float Azimuth = 0.0f;
		float Inclination = 0.0f;

		SkylightComponent() = default;
		SkylightComponent( const SkylightComponent& other ) = default;
	};

	struct PhysXBoxColliderComponent
	{
		glm::vec3 Extents = { 1.0f, 1.0f, 1.0f };
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };

		bool IsTrigger = false;

		PhysXBoxColliderComponent() = default;
		PhysXBoxColliderComponent( const glm::vec3& extents ) : Extents( extents ) { }
	};

	struct PhysXSphereColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
		float Radius = 1.0f;

		bool IsTrigger = false;

		PhysXSphereColliderComponent() = default;
		PhysXSphereColliderComponent( float radius ) : Radius( radius ) { }
	};

	struct PhysXCapsuleColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };

		float Radius = 1.0f;
		float Height = 1.0f;

		bool IsTrigger = false;

		PhysXCapsuleColliderComponent() = default;
		PhysXCapsuleColliderComponent( float radius ) : Radius( radius ) { }
	};

	class PhysXRigidbody;
	struct PhysXRigidbodyComponent
	{
		bool IsKinematic;
		bool UseCCD;
		int Mass = 2;

		PhysXRigidbody* m_Rigidbody;

		PhysXRigidbodyComponent() = default;
		PhysXRigidbodyComponent( bool isKinematic ) : IsKinematic( isKinematic ) { }
	};

	struct PhysXMaterialComponent
	{
		float StaticFriction = 0.6f;
		float DynamicFriction = 0.6f;
		float Restitution = 0.0f;
	};

	struct PointLightComponent
	{
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
		float Multiplier = 1.0f;
		float LightSize = 0.5f;
		float Radius = 10.0f;
		float MinRadius = 1.f;
		float Falloff = 1.f;
	};

	struct ScriptComponent
	{
		std::string ScriptName;
		UUID AssetID;
	};

	struct RelationshipComponent
	{
		UUID Parent = 0;

		std::vector<UUID> ChildrenID;

		RelationshipComponent() = default;
		RelationshipComponent( RelationshipComponent& other ) = default;
		RelationshipComponent( UUID parent ) : Parent( parent ) {  }
	};

	template<typename... V>
	struct ComponentGroup {};

	using AllComponents = ComponentGroup<TransformComponent, VisibilityComponent, TagComponent, IdComponent, RelationshipComponent,
		MeshComponent, 
		LightComponent, DirectionalLightComponent, SkylightComponent, PointLightComponent,
		CameraComponent,
		PhysXBoxColliderComponent, PhysXSphereColliderComponent, PhysXCapsuleColliderComponent, PhysXRigidbodyComponent, PhysXMaterialComponent,
		ScriptComponent>;
}