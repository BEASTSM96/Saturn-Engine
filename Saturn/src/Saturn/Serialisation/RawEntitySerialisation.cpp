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

#include "sppch.h"
#include "RawEntitySerialisation.h"

#include "RawSerialisation.h"
#include "Saturn/Asset/AssetManager.h"

namespace Saturn {

	template<typename Component, typename Func>
	void WriteComponent( Ref<Entity>& rEntity, std::ofstream& rStream, Func Function )
	{
		bool hasT = rEntity->HasComponent<Component>();

		RawSerialisation::WriteObject( hasT, rStream );

		if( hasT )
		{
			Function();
		}
	}

	template<typename Component, typename IStream, typename Func>
	void ReadComponent( Ref<Entity>& rEntity, IStream& rStream, Func Function )
	{
		bool hadT = false;
		RawSerialisation::ReadObject( hadT, rStream );

		// If the entity ever had Component before then add it and invoke function.
		if( hadT )
		{
			rEntity->AddComponent<Component>();

			Function();
		}
	}

	void RawEntitySerialisation::SerialiseEntity( Ref<Entity>& rEntity, std::ofstream& rStream )
	{
		RawSerialisation::WriteObject( rEntity->GetComponent<IdComponent>().ID, rStream );
		RawSerialisation::WriteObject( rEntity->GetHandle(), rStream );

		bool isPrefab = rEntity->HasComponent<PrefabComponent>();
		
		// Tag Component.
		WriteComponent<TagComponent>( rEntity, rStream, [&]() 
			{
				RawSerialisation::WriteString( rEntity->GetComponent<TagComponent>().Tag, rStream );
			} );

		// Transform Component.
		WriteComponent<TransformComponent>( rEntity, rStream, [&]()
			{
				auto& tc = rEntity->GetComponent<TransformComponent>();

				RawSerialisation::WriteVec3( tc.Position, rStream );
				RawSerialisation::WriteVec3( tc.GetRotationEuler(), rStream );
				RawSerialisation::WriteVec3( tc.Scale, rStream );
			} );


		// Relationship Component.
		WriteComponent<RelationshipComponent>( rEntity, rStream, [&]()
			{
				auto& rc = rEntity->GetComponent< RelationshipComponent >();

				RawSerialisation::WriteObject( rc.Parent, rStream );

				RawSerialisation::WriteVector( rc.ChildrenID, rStream );
			} );
		
		
		// Relationship Component
		WriteComponent<PrefabComponent>( rEntity, rStream, [&]()
			{
				auto& pc = rEntity->GetComponent< PrefabComponent >();

				RawSerialisation::WriteObject( pc.AssetID, rStream );
				RawSerialisation::WriteObject( pc.Modified, rStream );
			} );

		// Mesh Component
		WriteComponent<StaticMeshComponent>( rEntity, rStream, [&]()
			{
				auto& mc = rEntity->GetComponent< StaticMeshComponent >();

				AssetID ID = 0;

				if( mc.Mesh )
					ID = mc.Mesh->ID;

				RawSerialisation::WriteObject( ID, rStream );

				bool HasRegistry = mc.MaterialRegistry != nullptr;

				RawSerialisation::WriteObject( HasRegistry, rStream );

				if( HasRegistry )
					MaterialRegistry::Serialise( mc.MaterialRegistry, rStream );
			} );

		// Script Component
		WriteComponent<ScriptComponent>( rEntity, rStream, [&]()
			{
				auto& sc = rEntity->GetComponent< ScriptComponent >();

				RawSerialisation::WriteObject( sc.ScriptName, rStream );
				RawSerialisation::WriteObject( sc.AssetID, rStream );
			} );

		// Sky light component
		WriteComponent<SkylightComponent>( rEntity, rStream, [&]()
			{
				auto& slc = rEntity->GetComponent< SkylightComponent >();

				RawSerialisation::WriteObject( slc.DynamicSky, rStream );

				if( slc.DynamicSky )
				{
					RawSerialisation::WriteObject( slc.Turbidity, rStream );
					RawSerialisation::WriteObject( slc.Azimuth, rStream );
					RawSerialisation::WriteObject( slc.Inclination, rStream );
				}
			} );

		// Directional Light Component
		WriteComponent<DirectionalLightComponent>( rEntity, rStream, [&]()
			{
				auto& dlc = rEntity->GetComponent< DirectionalLightComponent>();

				RawSerialisation::WriteVec3( dlc.Radiance, rStream );
				RawSerialisation::WriteObject( dlc.Intensity, rStream );
				RawSerialisation::WriteObject( dlc.CastShadows, rStream );
			} );


		// Point Light Component
		WriteComponent<PointLightComponent>( rEntity, rStream, [&]()
			{
				auto& plc = rEntity->GetComponent< PointLightComponent >();

				RawSerialisation::WriteVec3( plc.Radiance, rStream );
				RawSerialisation::WriteObject( plc.Intensity, rStream );
				RawSerialisation::WriteObject( plc.Falloff, rStream );
				RawSerialisation::WriteObject( plc.LightSize, rStream );
				RawSerialisation::WriteObject( plc.MinRadius, rStream );
				RawSerialisation::WriteObject( plc.Multiplier, rStream );
				RawSerialisation::WriteObject( plc.Radius, rStream );
			} );

		// Box collider
		WriteComponent<BoxColliderComponent>( rEntity, rStream, [&]()
			{
				auto& bcc = rEntity->GetComponent< BoxColliderComponent >();

				RawSerialisation::WriteVec3( bcc.Extents, rStream );
				RawSerialisation::WriteVec3( bcc.Offset, rStream );
				RawSerialisation::WriteObject( bcc.IsTrigger, rStream );
			} );
		
		// Sphere collider
		WriteComponent<SphereColliderComponent>( rEntity, rStream, [&]()
			{
				auto& scc = rEntity->GetComponent< SphereColliderComponent >();

				RawSerialisation::WriteObject( scc.Radius, rStream );
				RawSerialisation::WriteVec3( scc.Offset, rStream );
				RawSerialisation::WriteObject( scc.IsTrigger, rStream );
			} );

		// Capsule collider
		WriteComponent<CapsuleColliderComponent>( rEntity, rStream, [&]()
			{
				auto& ccc = rEntity->GetComponent< CapsuleColliderComponent >();

				RawSerialisation::WriteObject( ccc.Height, rStream );
				RawSerialisation::WriteObject( ccc.Radius, rStream );
				RawSerialisation::WriteVec3( ccc.Offset, rStream );
				RawSerialisation::WriteObject( ccc.IsTrigger, rStream );
			} );

		// Rigid body
		WriteComponent<RigidbodyComponent>( rEntity, rStream, [&]()
			{
				auto& rbc = rEntity->GetComponent< RigidbodyComponent >();

				RawSerialisation::WriteObject( rbc.IsKinematic, rStream );
				RawSerialisation::WriteObject( rbc.UseCCD, rStream );
				RawSerialisation::WriteObject( rbc.Mass, rStream );
				RawSerialisation::WriteObject( rbc.LockFlags, rStream );
			} );

		// Physics material
		WriteComponent<PhysicsMaterialComponent>( rEntity, rStream, [&]()
			{
				auto& pmc = rEntity->GetComponent< PhysicsMaterialComponent >();

				RawSerialisation::WriteObject( pmc.AssetID, rStream );
			} );

		// Camera Component
		WriteComponent<CameraComponent>( rEntity, rStream, [&]()
			{
				auto& cc = rEntity->GetComponent< CameraComponent >();

				RawSerialisation::WriteObject( cc.MainCamera, rStream );
			} );

		// Audio Player Component
		WriteComponent<AudioPlayerComponent>( rEntity, rStream, [&]()
			{
				auto& spc = rEntity->GetComponent< AudioPlayerComponent >();

				RawSerialisation::WriteObject( spc.AssetID, rStream );
				RawSerialisation::WriteObject( spc.Loop, rStream );
				RawSerialisation::WriteObject( spc.Mute, rStream );
			} );

		// Audio Listener Component
		WriteComponent<AudioListenerComponent>( rEntity, rStream, [&]()
			{
				auto& alc = rEntity->GetComponent< AudioListenerComponent >();

				RawSerialisation::WriteObject( alc.Primary, rStream );
				RawSerialisation::WriteVec3( alc.Direction, rStream );
				RawSerialisation::WriteObject( alc.ConeInnerAngle, rStream );
				RawSerialisation::WriteObject( alc.ConeOuterAngle, rStream );
			} );
	}

	void RawEntitySerialisation::DeserialiseEntity( Ref<Entity>& rEntity, std::istream& rStream )
	{
		RawSerialisation::ReadObject( rEntity->GetComponent<IdComponent>().ID, rStream );
		
		entt::entity handle{ entt::null };
		RawSerialisation::ReadObject( handle, rStream );

		// Tag Component
		ReadComponent<TagComponent>( rEntity, rStream, [&]()
			{
				rEntity->GetComponent<TagComponent>().Tag = RawSerialisation::ReadString( rStream );
			} );

		// Transform Component
		ReadComponent<TransformComponent>( rEntity, rStream, [&]()
			{
				auto& tc = rEntity->GetComponent<TransformComponent>();

				glm::vec3 rotation{};

				RawSerialisation::ReadVec3( tc.Position, rStream );
				RawSerialisation::ReadVec3( rotation, rStream );
				RawSerialisation::ReadVec3( tc.Scale, rStream );

				tc.SetRotation( rotation );
			} );

		// Relationship Component
		ReadComponent<RelationshipComponent>( rEntity, rStream, [&]()
			{
				auto& rc = rEntity->GetComponent< RelationshipComponent >();

				RawSerialisation::ReadObject( rc.Parent, rStream );

				RawSerialisation::ReadVector( rc.ChildrenID, rStream );
			} );

		// Prefab Component
		ReadComponent<PrefabComponent>( rEntity, rStream, [&]()
			{
				auto& pc = rEntity->GetComponent< PrefabComponent >();

				RawSerialisation::ReadObject( pc.AssetID, rStream );
				RawSerialisation::ReadObject( pc.Modified, rStream );
			} );

		// Mesh Component
		ReadComponent<StaticMeshComponent>( rEntity, rStream, [&]()
			{
				auto& mc = rEntity->GetComponent< StaticMeshComponent >();

				AssetID ID = 0;

				if( mc.Mesh )
					ID = mc.Mesh->ID;

				RawSerialisation::ReadObject( ID, rStream );

				bool HasRegistry = false;
				RawSerialisation::ReadObject( HasRegistry, rStream );

				mc.MaterialRegistry = Ref<MaterialRegistry>::Create();

				if( HasRegistry )
					MaterialRegistry::Deserialise( mc.MaterialRegistry, rStream );

				if( ID != 0 )
				{
					// Load Mesh
					auto mesh = AssetManager::Get().GetAssetAs<StaticMesh>( ID );
					mc.Mesh = mesh;
				
					if( !mc.MaterialRegistry->HasAnyOverrides() ) 
					{
						mc.MaterialRegistry->Copy( mc.Mesh->GetMaterialRegistry() );
					}
					
					mc.MaterialRegistry->SetMesh( mc.Mesh );
				}
			} );

		// Script Component
		ReadComponent<ScriptComponent>( rEntity, rStream, [&]()
			{
				auto& sc = rEntity->GetComponent< ScriptComponent >();

				RawSerialisation::ReadObject( sc.ScriptName, rStream );
				RawSerialisation::ReadObject( sc.AssetID, rStream );
			} );

		// Sky light component
		ReadComponent<SkylightComponent>( rEntity, rStream, [&]()
			{
				auto& slc = rEntity->GetComponent< SkylightComponent >();

				RawSerialisation::ReadObject( slc.DynamicSky, rStream );

				if( slc.DynamicSky )
				{
					RawSerialisation::ReadObject( slc.Turbidity, rStream );
					RawSerialisation::ReadObject( slc.Azimuth, rStream );
					RawSerialisation::ReadObject( slc.Inclination, rStream );
				}
			} );
		
		// Directional Light Component
		ReadComponent<DirectionalLightComponent>( rEntity, rStream, [&]()
			{
				auto& dlc = rEntity->GetComponent< DirectionalLightComponent>();

				RawSerialisation::ReadVec3( dlc.Radiance, rStream );
				RawSerialisation::ReadObject( dlc.Intensity, rStream );
				RawSerialisation::ReadObject( dlc.CastShadows, rStream );
			} );

		// Point Light Component
		ReadComponent<PointLightComponent>( rEntity, rStream, [&]()
			{
				auto& plc = rEntity->GetComponent< PointLightComponent >();

				RawSerialisation::ReadVec3( plc.Radiance, rStream );
				RawSerialisation::ReadObject( plc.Intensity, rStream );
				RawSerialisation::ReadObject( plc.Falloff, rStream );
				RawSerialisation::ReadObject( plc.LightSize, rStream );
				RawSerialisation::ReadObject( plc.MinRadius, rStream );
				RawSerialisation::ReadObject( plc.Multiplier, rStream );
				RawSerialisation::ReadObject( plc.Radius, rStream );
			} );

		// Box collider
		ReadComponent<BoxColliderComponent>( rEntity, rStream, [&]()
			{
				auto& bcc = rEntity->GetComponent< BoxColliderComponent >();

				RawSerialisation::ReadVec3( bcc.Extents, rStream );
				RawSerialisation::ReadVec3( bcc.Offset, rStream );
				RawSerialisation::ReadObject( bcc.IsTrigger, rStream );
			} );
		
		// Sphere collider
		ReadComponent<SphereColliderComponent>( rEntity, rStream, [&]()
			{
				auto& scc = rEntity->GetComponent< SphereColliderComponent >();

				RawSerialisation::ReadObject( scc.Radius, rStream );
				RawSerialisation::ReadVec3( scc.Offset, rStream );
				RawSerialisation::ReadObject( scc.IsTrigger, rStream );
			} );
		
		// Capsule collider
		ReadComponent<CapsuleColliderComponent>( rEntity, rStream, [&]()
			{
				auto& ccc = rEntity->GetComponent< CapsuleColliderComponent >();

				RawSerialisation::ReadObject( ccc.Height, rStream );
				RawSerialisation::ReadObject( ccc.Radius, rStream );
				RawSerialisation::ReadVec3( ccc.Offset, rStream );
				RawSerialisation::ReadObject( ccc.IsTrigger, rStream );
			} );
		
		// Rigid body
		ReadComponent<RigidbodyComponent>( rEntity, rStream, [&]()
			{
				auto& rbc = rEntity->GetComponent< RigidbodyComponent >();

				RawSerialisation::ReadObject( rbc.IsKinematic, rStream );
				RawSerialisation::ReadObject( rbc.UseCCD, rStream );
				RawSerialisation::ReadObject( rbc.Mass, rStream );
				RawSerialisation::ReadObject( rbc.LockFlags, rStream );
			} );
		
		// Physics material
		ReadComponent<PhysicsMaterialComponent>( rEntity, rStream, [&]()
			{
				auto& pmc = rEntity->GetComponent< PhysicsMaterialComponent >();

				RawSerialisation::ReadObject( pmc.AssetID, rStream );
			} );
		
		// Camera Component
		ReadComponent<CameraComponent>( rEntity, rStream, [&]()
			{
				auto& cc = rEntity->GetComponent< CameraComponent >();

				RawSerialisation::ReadObject( cc.MainCamera, rStream );
			} );

		// Audio Component
		ReadComponent<AudioPlayerComponent>( rEntity, rStream, [&]()
			{
				auto& spc = rEntity->GetComponent< AudioPlayerComponent >();

				RawSerialisation::ReadObject( spc.AssetID, rStream );
				RawSerialisation::ReadObject( spc.Loop, rStream );
				RawSerialisation::ReadObject( spc.Mute, rStream );
			} );

		// Audio Listener Component
		ReadComponent<AudioListenerComponent>( rEntity, rStream, [&]()
			{
				auto& alc = rEntity->GetComponent< AudioListenerComponent >();

				RawSerialisation::ReadObject( alc.Primary, rStream );
				RawSerialisation::ReadVec3( alc.Direction, rStream );
				RawSerialisation::ReadObject( alc.ConeInnerAngle, rStream );
				RawSerialisation::ReadObject( alc.ConeOuterAngle, rStream );
			} );
	}

}