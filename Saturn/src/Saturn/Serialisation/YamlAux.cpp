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
#include "YamlAux.h"

#include "Saturn/Asset/AssetManager.h"

namespace Saturn {

	void SerialiseEntity( YAML::Emitter& rEmitter, Ref<Entity> entity )
	{
		rEmitter << YAML::BeginMap;
		rEmitter << YAML::Key << "Entity" << YAML::Value << entity->GetComponent< IdComponent >().ID;
		bool isPrefab = entity->HasComponent<PrefabComponent>();

		// Tag Component
		if( entity->HasComponent<TagComponent>() )
		{
			rEmitter << YAML::Key << "TagComponent";
			rEmitter << YAML::BeginMap;

			rEmitter << YAML::Key << "Tag" << YAML::Value << entity->GetComponent< TagComponent >().Tag;

			rEmitter << YAML::EndMap;
		}

		// Transform Component
		if( entity->HasComponent<TransformComponent>() )
		{
			rEmitter << YAML::Key << "TransformComponent";
			rEmitter << YAML::BeginMap;

			auto& tc = entity->GetComponent< TransformComponent >();

			rEmitter << YAML::Key << "Position" << YAML::Value << tc.Position;
			rEmitter << YAML::Key << "Rotation" << YAML::Value << glm::degrees( tc.GetRotationEuler() );
			rEmitter << YAML::Key << "Quaternion" << YAML::Value << tc.GetRotation();
			rEmitter << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			rEmitter << YAML::EndMap;
		}

		// Relationship Component
		if( entity->HasComponent<RelationshipComponent>() )
		{
			rEmitter << YAML::Key << "RelationshipComponent";
			rEmitter << YAML::BeginMap;

			auto& rc = entity->GetComponent< RelationshipComponent >();

			rEmitter << YAML::Key << "Parent" << YAML::Value << rc.Parent;

			rEmitter << YAML::Key << "Children";
			rEmitter << YAML::BeginSeq;

			for( const auto& id : rc.ChildrenID )
			{
				rEmitter << YAML::BeginMap;

				rEmitter << YAML::Key << "ID" << YAML::Value << id;

				rEmitter << YAML::EndMap;
			}

			rEmitter << YAML::EndSeq;

			rEmitter << YAML::EndMap;
		}

		// Prefab Component
		if( isPrefab )
		{
			rEmitter << YAML::Key << "PrefabComponent";
			rEmitter << YAML::BeginMap;

			rEmitter << YAML::Key << "AssetID" << YAML::Value << entity->GetComponent< PrefabComponent >().AssetID;
			rEmitter << YAML::Key << "Modified" << YAML::Value << entity->GetComponent< PrefabComponent >().Modified;

			rEmitter << YAML::EndMap;
		}

		// Mesh Component
		if( entity->HasComponent<StaticMeshComponent>() )
		{
			rEmitter << YAML::Key << "MeshComponent";
			rEmitter << YAML::BeginMap;

			auto& mc = entity->GetComponent< StaticMeshComponent >();

			if( mc.Mesh )
				rEmitter << YAML::Key << "Asset" << YAML::Value << mc.Mesh->ID;
			else
				rEmitter << YAML::Key << "Asset" << YAML::Value << 0;

			rEmitter << YAML::Key << "MaterialRegistry";
			rEmitter << YAML::BeginMap;
			
			if( mc.MaterialRegistry ) 
				rEmitter << YAML::Key << "AnyOverrides" << YAML::Value << mc.MaterialRegistry->HasAnyOverrides();
			else
				rEmitter << YAML::Key << "AnyOverrides" << YAML::Value << false;

			rEmitter << YAML::Key << "MaterialOverrides";
			rEmitter << YAML::BeginSeq;

			if( mc.MaterialRegistry )
			{
				int i = 0;
				for( const auto& material : mc.MaterialRegistry->GetMaterials() )
				{
					rEmitter << YAML::BeginMap;

					if( mc.MaterialRegistry->HasOverrides( i ) )
						rEmitter << YAML::Key << i << YAML::Value << material->ID;
					else
						rEmitter << YAML::Key << i << YAML::Value << 0;

					rEmitter << YAML::EndMap;

					i++;
				}
			}

			rEmitter << YAML::EndSeq;

			rEmitter << YAML::EndMap;

			rEmitter << YAML::EndMap;
		}

		// Script Component
		if( entity->HasComponent<ScriptComponent>() )
		{
			rEmitter << YAML::Key << "ScriptComponent";
			rEmitter << YAML::BeginMap;

			auto& sc = entity->GetComponent< ScriptComponent >();

			rEmitter << YAML::Key << "Name" << YAML::Value << sc.ScriptName;
			rEmitter << YAML::Key << "ID" << YAML::Value << sc.AssetID;

			rEmitter << YAML::EndMap;
		}

		// Sky light component
		if( entity->HasComponent<SkylightComponent>() )
		{
			rEmitter << YAML::Key << "SkyLightComponent";
			rEmitter << YAML::BeginMap;

			auto& slc = entity->GetComponent< SkylightComponent >();

			rEmitter << YAML::Key << "IsPreetham" << YAML::Value << slc.DynamicSky;

			if( slc.DynamicSky )
			{
				rEmitter << YAML::Key << "Preetham Settings" << YAML::Value;
				rEmitter << YAML::BeginMap;

				rEmitter << YAML::Key << "Turbidity" << YAML::Value << slc.Turbidity;
				rEmitter << YAML::Key << "Azimuth" << YAML::Value << slc.Azimuth;
				rEmitter << YAML::Key << "Inclination" << YAML::Value << slc.Inclination;

				rEmitter << YAML::EndMap;
			}

			rEmitter << YAML::EndMap;
		}

		// Directional Light Component
		if( entity->HasComponent<DirectionalLightComponent>() )
		{
			rEmitter << YAML::Key << "DirectionalLightComponent";
			rEmitter << YAML::BeginMap;

			auto& dlc = entity->GetComponent< DirectionalLightComponent >();

			rEmitter << YAML::Key << "Radiance" << YAML::Value << dlc.Radiance;
			rEmitter << YAML::Key << "Intensity" << YAML::Value << dlc.Intensity;
			rEmitter << YAML::Key << "CastShadows" << YAML::Value << dlc.CastShadows;

			rEmitter << YAML::EndMap;
		}

		// Point Light Component
		if( entity->HasComponent<PointLightComponent>() )
		{
			rEmitter << YAML::Key << "PointLightComponent";
			rEmitter << YAML::BeginMap;

			auto& plc = entity->GetComponent< PointLightComponent >();

			rEmitter << YAML::Key << "Radiance" << YAML::Value << plc.Radiance;
			rEmitter << YAML::Key << "Intensity" << YAML::Value << plc.Intensity;
			rEmitter << YAML::Key << "Falloff" << YAML::Value << plc.Falloff;
			rEmitter << YAML::Key << "LightSize" << YAML::Value << plc.LightSize;
			rEmitter << YAML::Key << "MinRadius" << YAML::Value << plc.MinRadius;
			rEmitter << YAML::Key << "Multiplier" << YAML::Value << plc.Multiplier;
			rEmitter << YAML::Key << "Radius" << YAML::Value << plc.Radius;

			rEmitter << YAML::EndMap;
		}

		// Box collider
		if( entity->HasComponent<BoxColliderComponent>() )
		{
			rEmitter << YAML::Key << "BoxColliderComponent";
			rEmitter << YAML::BeginMap;

			auto& bcc = entity->GetComponent< BoxColliderComponent >();

			rEmitter << YAML::Key << "Extents" << YAML::Value << bcc.Extents;
			rEmitter << YAML::Key << "Offset" << YAML::Value << bcc.Offset;
			rEmitter << YAML::Key << "IsTrigger" << YAML::Value << bcc.IsTrigger;

			rEmitter << YAML::EndMap;
		}

		// Sphere collider
		if( entity->HasComponent<SphereColliderComponent>() )
		{
			rEmitter << YAML::Key << "SphereColliderComponent";
			rEmitter << YAML::BeginMap;

			auto& scc = entity->GetComponent< SphereColliderComponent >();

			rEmitter << YAML::Key << "Radius" << YAML::Value << scc.Radius;
			rEmitter << YAML::Key << "Offset" << YAML::Value << scc.Offset;
			rEmitter << YAML::Key << "IsTrigger" << YAML::Value << scc.IsTrigger;

			rEmitter << YAML::EndMap;
		}

		// Capsule collider
		if( entity->HasComponent<CapsuleColliderComponent>() )
		{
			rEmitter << YAML::Key << "CapsuleColliderComponent";
			rEmitter << YAML::BeginMap;

			auto& ccc = entity->GetComponent< CapsuleColliderComponent >();

			rEmitter << YAML::Key << "Height" << YAML::Value << ccc.Height;
			rEmitter << YAML::Key << "Radius" << YAML::Value << ccc.Radius;
			rEmitter << YAML::Key << "Offset" << YAML::Value << ccc.Offset;
			rEmitter << YAML::Key << "IsTrigger" << YAML::Value << ccc.IsTrigger;

			rEmitter << YAML::EndMap;
		}

		// Rigid body
		if( entity->HasComponent<RigidbodyComponent>() )
		{
			rEmitter << YAML::Key << "RigidbodyComponent";
			rEmitter << YAML::BeginMap;

			auto& rbc = entity->GetComponent< RigidbodyComponent >();

			rEmitter << YAML::Key << "IsKinematic" << YAML::Value << rbc.IsKinematic;
			rEmitter << YAML::Key << "CCD" << YAML::Value << rbc.UseCCD;
			rEmitter << YAML::Key << "Mass" << YAML::Value << rbc.Mass;

			rEmitter << YAML::Key << "LockFlags" << YAML::Value << (int)rbc.LockFlags;

			rEmitter << YAML::EndMap;
		}

		// Physics material
		if( entity->HasComponent<PhysicsMaterialComponent>() )
		{
			rEmitter << YAML::Key << "PhysicsMaterialComponent";
			rEmitter << YAML::BeginMap;

			auto& pmc = entity->GetComponent< PhysicsMaterialComponent >();

			rEmitter << YAML::Key << "AssetID" << YAML::Value << pmc.AssetID;

			rEmitter << YAML::EndMap;
		}

		// Camera Component
		if ( entity->HasComponent<CameraComponent>() )
		{
			rEmitter << YAML::Key << "CameraComponent";
			rEmitter << YAML::BeginMap;

			auto& cc = entity->GetComponent< CameraComponent >();

			rEmitter << YAML::Key << "MainCamera" << YAML::Value << cc.MainCamera;

			rEmitter << YAML::EndMap;
		}

		rEmitter << YAML::EndMap;
	}

	void DeserialiseEntities( YAML::Node& rNode, Ref<Scene> scene )
	{
		if( rNode.IsNull() )
			return;

		for( auto entity : rNode )
		{
			UUID entityID = entity[ "Entity" ].as< uint64_t >();

			std::string Tag = "";

			if( entity[ "TagComponent" ] )
				Tag = entity[ "TagComponent" ][ "Tag" ].as< std::string >();

			SAT_CORE_INFO( "Deserialised entity with ID: {0}, with name : {1}", entityID, Tag );

			Ref<Entity> DeserialisedEntity = nullptr;

			auto srcc = entity[ "ScriptComponent" ];
			if( srcc )
			{
				std::string ScriptName = srcc[ "Name" ].as< std::string >();

				// Ask the game module to create the entity.
				//DeserialisedEntity = scene->CreateEntityWithIDScript( entityID, Tag, ScriptName );

				DeserialisedEntity = Ref<Entity>::Create();
				DeserialisedEntity->SetName( Tag );
				DeserialisedEntity->GetComponent<IdComponent>().ID = entityID;

				auto& s = DeserialisedEntity->AddComponent< ScriptComponent >();

				s.ScriptName = ScriptName;
				s.AssetID = srcc[ "ID" ].as< uint64_t >();

				SAT_CORE_INFO( "Created entity with class name: {0}", s.ScriptName );
			}
			else
			{
				DeserialisedEntity = Ref<Entity>::Create();
				DeserialisedEntity->SetName( Tag );
				DeserialisedEntity->GetComponent<IdComponent>().ID = entityID;
			}

			auto tc = entity[ "TransformComponent" ];
			if( tc )
			{
				auto& t = DeserialisedEntity->GetComponent< TransformComponent >();

				t.Position = tc[ "Position" ].as< glm::vec3 >();

				t.SetRotation( glm::radians( tc[ "Rotation" ].as< glm::vec3 >() ) );
				
				// This might not be needed.
				//t.SetRotation( tc[ "Quaternion" ].as< glm::quat >() );

				t.Scale = tc[ "Scale" ].as< glm::vec3 >();
			}

			auto mc = entity[ "MeshComponent" ];
			if( mc )
			{
				auto& m = DeserialisedEntity->AddComponent< StaticMeshComponent >();

				auto id = mc[ "Asset" ].as<uint64_t>( 0 );

				if( id != 0 ) 
				{
					auto mesh = AssetManager::Get().GetAssetAs<StaticMesh>( id );

					m.Mesh = mesh;
					m.MaterialRegistry = Ref<MaterialRegistry>::Create();

					auto materialRegistry = mc[ "MaterialRegistry" ];
					if( materialRegistry )
					{
						bool hasOverrides = materialRegistry[ "AnyOverrides" ].as<bool>();

						if( hasOverrides )
						{
							auto materialOverrides = materialRegistry[ "MaterialOverrides" ];

							int i = 0;
							for( auto override : materialOverrides )
							{
								auto id = override[ i ].as<uint64_t>();

								if( id != 0 )
								{
									m.MaterialRegistry->AddAsset( AssetManager::Get().GetAssetAs<MaterialAsset>( id ) );
									m.MaterialRegistry->SetOverries( i, true );
								}

								i++;
							}
						}
						else
						{
							m.MaterialRegistry->Copy( m.Mesh->GetMaterialRegistry() );
						}
					}
					
					m.MaterialRegistry->SetMesh( m.Mesh );
				}
			}

			auto rcNode = entity[ "RelationshipComponent" ];
			auto& rc = DeserialisedEntity->GetComponent<RelationshipComponent>();
			rc.Parent = rcNode[ "Parent" ] ? rcNode[ "Parent" ].as<uint64_t>() : 0;

			auto rcChildren = rcNode[ "Children" ];
			if( rcChildren )
			{
				for( auto child : rcChildren )
				{
					uint64_t id = child[ "ID" ].as<uint64_t>();
					rc.ChildrenID.push_back( id );
				}
			}
			
			auto pc = entity["PrefabComponent" ];
			if( pc )
			{
				auto& p = DeserialisedEntity->AddComponent< PrefabComponent >();

				p.AssetID = pc[ "AssetID" ].as< uint64_t >();
			}

			auto slc = entity[ "SkyLightComponent" ];
			if( slc )
			{
				auto& s = DeserialisedEntity->AddComponent< SkylightComponent >();

				s.DynamicSky = slc[ "IsPreetham" ].as< bool >();

				if( s.DynamicSky )
				{
					auto PreethamSettings = slc[ "Preetham Settings" ];

					s.Turbidity = PreethamSettings[ "Turbidity" ].as< float >();
					s.Azimuth = PreethamSettings[ "Azimuth" ].as< float >();
					s.Inclination = PreethamSettings[ "Inclination" ].as< float >();
				}
				else
				{
					// TODO...
				}
			}

			auto dlc = entity[ "DirectionalLightComponent" ];
			if( dlc )
			{
				auto& d = DeserialisedEntity->AddComponent< DirectionalLightComponent >();

				d.Radiance = dlc[ "Radiance" ].as< glm::vec3 >();
				d.Intensity = dlc[ "Intensity" ].as< float >();
				d.CastShadows = dlc[ "CastShadows" ].as< bool >();
			}

			auto plc = entity[ "PointLightComponent" ];
			if( plc )
			{
				auto& p = DeserialisedEntity->AddComponent< PointLightComponent >();

				p.Radiance = plc[ "Radiance" ].as< glm::vec3 >();
				p.Intensity = plc[ "Intensity" ].as< float >();
				p.Multiplier = plc[ "Multiplier" ].as< float >();
				p.LightSize = plc[ "LightSize" ].as< float >();
				p.Radius = plc[ "Radius" ].as< float >();
				p.MinRadius = plc[ "MinRadius" ].as< float >();
				p.Falloff = plc[ "Falloff" ].as< float >();
			}

			auto bcc = entity[ "BoxColliderComponent" ];
			if( bcc )
			{
				auto& b = DeserialisedEntity->AddComponent< BoxColliderComponent >();

				b.Extents = bcc[ "Extents" ].as< glm::vec3 >();
				b.Offset = bcc[ "Offset" ].as< glm::vec3 >();
				b.IsTrigger = bcc[ "IsTrigger" ].as< bool >();
			}

			auto scc = entity[ "SphereColliderComponent" ];
			if( scc )
			{
				auto& s = DeserialisedEntity->AddComponent< SphereColliderComponent >();

				s.Radius = scc[ "Radius" ].as< float >();
				s.Offset = scc[ "Offset" ].as< glm::vec3 >();
				s.IsTrigger = scc[ "IsTrigger" ].as< bool >();
			}

			auto ccc = entity[ "CapsuleColliderComponent" ];
			if( ccc )
			{
				auto& c = DeserialisedEntity->AddComponent< CapsuleColliderComponent >();

				c.Height = ccc[ "Height" ].as< float >();
				c.Radius = ccc[ "Radius" ].as< float >();
				c.Offset = ccc[ "Offset" ].as< glm::vec3 >();
				c.IsTrigger = ccc[ "IsTrigger" ].as< bool >();
			}

			auto rbc = entity[ "RigidbodyComponent" ];
			if( rbc )
			{
				auto& rb = DeserialisedEntity->AddComponent< RigidbodyComponent >();

				rb.IsKinematic = rbc[ "IsKinematic" ].as< bool >();
				rb.UseCCD = rbc[ "CCD" ].as< bool >();
				rb.Mass = rbc[ "Mass" ].as< float >();

				auto lockNode = rbc[ "LockFlags" ];

				if ( lockNode )
				{
					rb.LockFlags = lockNode.as< int >(0);
				}
				else
				{
					rb.LockFlags = 0;
				}
			}

			auto pmc = entity[ "PhysicsMaterialComponent" ];
			if( pmc )
			{
				auto& m = DeserialisedEntity->AddComponent< PhysicsMaterialComponent >();

				m.AssetID = pmc[ "AssetID" ].as< uint64_t >( 0 );
			}

			auto cc = entity[ "CameraComponent" ];
			if( cc )
			{
				auto& c = DeserialisedEntity->AddComponent< CameraComponent >();

				c.MainCamera = cc[ "MainCamera" ].as< bool >();
			}

		}
	}

}