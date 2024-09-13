/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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

#include "Saturn/Core/Math.h"

#include "Saturn/Vulkan/Mesh.h"

#include "Saturn/Core/UUID.h"

#include "EntityVisibility.h"

#include "Saturn/Core/Renderer/SceneCamera.h"

#include <string>

namespace Saturn {

	struct TransformComponent
	{
	private:
		friend class SceneSerialiser;
	private:
		// Quat's in GLM are W,X,Y,Z
		// I want to change it X,Y,Z,W
		// I don't want to use quat's however quat's are just better for rotations than a Vector3
		glm::quat  RotationQuat ={ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3  Rotation = { 0.0f, 0.0f, 0.0f };
	public:
		glm::vec3  Position ={ 0.0f , 0.0f, 0.0f };
		glm::vec3  Scale	={ 1.0f , 1.0f, 1.0f };

		static constexpr glm::vec3 Up ={ 0.0f, 1.0f, 0.0f };
		static constexpr glm::vec3 Right ={ 1.0f, 0.0f, 0.0f };
		static constexpr glm::vec3 Forward ={ 0.0f, 0.0f, -1.0f };

		TransformComponent( void ) = default;
		TransformComponent( const TransformComponent& ) = default;
		TransformComponent( const glm::vec3& rPosition )
			: Position( rPosition )
		{
		}

		glm::mat4 GetTransform() const
		{
			return glm::translate( glm::mat4( 1.0f ), Position )
				* glm::toMat4( RotationQuat )
				* glm::scale( glm::mat4( 1.0f ), Scale );
		}
		
		void SetTransform( const glm::mat4& rTransfrom )
		{
			Math::DecomposeTransform( rTransfrom, Position, RotationQuat, Scale );
			Rotation = glm::eulerAngles( RotationQuat );
		}

		// Where rotation is a euler angle.
		// Rotational values must be radians.
		void SetRotation( const glm::vec3& rotation ) 
		{
			Rotation = rotation;
			RotationQuat = glm::quat( rotation );
		}

		// Rotational values must be radians.
		void SetRotation( const glm::quat& rotation )
		{
			RotationQuat = rotation;
			Rotation = glm::eulerAngles( rotation );
		}

		// Rotational values will be in radians.
		glm::quat GetRotation() const
		{
			return RotationQuat;
		}

		// Rotational values will be in radians.
		glm::vec3 GetRotationEuler() const
		{
			return Rotation;
		}

		operator glm::mat4 ( ) { return GetTransform(); }
		operator const glm::mat4& ( ) const { return GetTransform(); }
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent( const TagComponent& ) = default;
		TagComponent( const std::string& tag ) : Tag( tag ) {}
	};

	struct IdComponent
	{
		UUID ID;

		IdComponent() = default;
		IdComponent( const IdComponent& ) = default;
		IdComponent( const UUID& uuid ) : ID( uuid ) {}
	};

	struct StaticMeshComponent
	{
		UUID AssetID = 0;

		// TODO: Change to Asset ID
		Ref<Saturn::StaticMesh> Mesh;
		// We always want to store our own material registry because there will be one in the asset however that is global for all of the same meshes in the scene and what if we want to just locally change one asset.
		Ref<Saturn::MaterialRegistry> MaterialRegistry;

		StaticMeshComponent() = default;
		StaticMeshComponent( const StaticMeshComponent& other ) = default;
		StaticMeshComponent( Ref<Saturn::StaticMesh>& rMesh )
			: Mesh( rMesh ) {}

		operator Ref<Saturn::StaticMesh>() { return Mesh; }
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
		bool MainCamera = false;
		float Fov = 45.0f;
	};

	// Preetham sky
	struct SkylightComponent 
	{
		EnvironmentMap Map;

		bool DynamicSky = true;

		float Turbidity = 2.0f;
		float Azimuth = 0.0f;
		float Inclination = 0.0f;

		SkylightComponent() = default;
		SkylightComponent( const SkylightComponent& other ) = default;
	};

	struct BoxColliderComponent
	{
		glm::vec3 Extents = { 1.0f, 1.0f, 1.0f };
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };

		bool IsTrigger = false;

		BoxColliderComponent() = default;
		BoxColliderComponent( const glm::vec3& extents ) : Extents( extents ) { }
	};

	struct SphereColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
		float Radius = 1.0f;

		bool IsTrigger = false;

		SphereColliderComponent() = default;
		SphereColliderComponent( float radius ) : Radius( radius ) { }
	};

	struct CapsuleColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };

		float Radius = 1.0f;
		float Height = 1.0f;

		bool IsTrigger = false;

		CapsuleColliderComponent() = default;
		CapsuleColliderComponent( float radius, float height ) : Radius( radius ), Height( height ) { }
	};

	struct MeshColliderComponent
	{
		bool IsTrigger = false;
	};

	// TODO: Do we really want to store the rigid body here?
	class PhysicsRigidBody;
	struct RigidbodyComponent
	{
		PhysicsRigidBody* Rigidbody = nullptr;

		bool IsKinematic = false;
		bool UseCCD = false;
		float Mass = 2.0f;
		float LinearDrag = 1.0f;
		uint32_t LockFlags = 0;

		UUID MaterialAssetID = 0;

		RigidbodyComponent() = default;
		RigidbodyComponent( bool isKinematic ) : IsKinematic( isKinematic ) { }
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
		UUID AssetID = 0;
	};

	struct RelationshipComponent
	{
		UUID Parent = 0;

		std::vector<UUID> ChildrenID;

		RelationshipComponent() = default;
		RelationshipComponent( RelationshipComponent& other ) = default;
		RelationshipComponent( UUID parent ) : Parent( parent ) {  }
	};

	struct PrefabComponent
	{
		UUID AssetID = 0;
		bool Modified = false;

		PrefabComponent() = default;
		PrefabComponent( PrefabComponent& other ) = default;
		PrefabComponent( UUID id ) : AssetID( id ) {  }
	};

	struct AudioPlayerComponent
	{
		UUID SpecAssetID = 0;
		UUID UniqueID;
		bool Loop = false;
		bool Mute = false;
		bool Spatialization = false;
		float VolumeMultiplier = 1.0f;
		float PitchMultiplier = 1.0f;
	};

	struct AudioListenerComponent
	{
		bool Primary = false;
		glm::vec3 Direction = TransformComponent::Forward;

		// Radians not degrees
		float ConeInnerAngle = 0.0f;
		float ConeOuterAngle = 0.0f;
	};

	struct BillboardComponent
	{
		UUID AssetID = 0;

		BillboardComponent() = default;
		BillboardComponent( BillboardComponent& other ) = default;
		BillboardComponent( UUID id ) : AssetID( id ) {  }
	};

	template<typename... V>
	struct ComponentGroup {};

	// Where T is the component and V is the target ComponentGroup.
	template<typename T, typename... V>
	struct ExcludeComponent;

	template<typename T, typename... V>
	struct ExcludeComponent<T, ComponentGroup<V...>>
	{
		using type = ComponentGroup<std::conditional_t<std::is_same_v<T, V>, T, V>...>;
	};

	using AllComponents = ComponentGroup<TransformComponent, TagComponent, IdComponent, RelationshipComponent, PrefabComponent,
		StaticMeshComponent, 
		DirectionalLightComponent, SkylightComponent, PointLightComponent,
		CameraComponent,
		BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, MeshColliderComponent, RigidbodyComponent,
		ScriptComponent, 
		AudioPlayerComponent, AudioListenerComponent,
		BillboardComponent>;
}