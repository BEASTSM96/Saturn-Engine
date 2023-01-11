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

#include "sppch.h"
#include "SceneSerialiser.h"

#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Components.h"
#include "Saturn/Vulkan/Mesh.h"

#include "YamlAux.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace Saturn {

	SceneSerialiser::SceneSerialiser( const Ref< Scene >& rScene )
	{
		m_Scene = rScene;
	}

	SceneSerialiser::~SceneSerialiser()
	{

	}

	void SceneSerialiser::Serialise( const std::string& rFilePath )
	{
		YAML::Emitter out;
		
		out << YAML::BeginMap;

		out << YAML::Key << "Scene" << YAML::Value << "Untitled Scene";

		out << YAML::Key << "FilePath" << YAML::Value << rFilePath;

		m_Scene->m_Filepath = rFilePath;

		out << YAML::Key << "Entities";

		out << YAML::BeginSeq;
		
		m_Scene->m_Registry.each( [&]( auto EntityID ) 
		{
			Entity entity = { EntityID, m_Scene.Pointer() };

			if( !entity )
				return;
		
			if( entity.HasComponent<SceneComponent>() )
				return;

			SerialiseEntity( out, entity );
		} );

		out << YAML::EndSeq;
		out << YAML::EndMap;
		
		std::ofstream FileOut( rFilePath );
		FileOut << out.c_str();
	}

	void SceneSerialiser::Deserialise( const std::string& rFilePath )
	{
		std::ifstream FileIn( rFilePath );
		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );
		
		if( data.IsNull() )
			return;

		if( !data["Scene"] )
			return;

		std::string sceneName = data["Scene"].as< std::string >();
		SAT_CORE_INFO( "Deserialising scene '{0}'", sceneName );

		m_Scene->m_Filepath = rFilePath;

		auto entities = data["Entities"];

		if( entities.IsNull() )
			return;

		for( auto entity : entities )
		{
			UUID entityID = entity["Entity"].as< uint64_t >();

			std::string Tag = "";

			if( entity["TagComponent"] )
				Tag = entity["TagComponent"]["Tag"].as< std::string >();
			
			SAT_CORE_INFO( "Deserialised entity with ID: {0}, with name : {1}", entityID, Tag );

			Entity DeserialisedEntity = m_Scene->CreateEntityWithID( entityID, Tag );
			
			auto tc = entity[ "TransformComponent" ];
			if( tc )
			{
				auto& t = DeserialisedEntity.GetComponent< TransformComponent >();

				t.Position = tc["Position"].as< glm::vec3 >();
				t.Rotation = glm::radians( tc["Rotation"].as< glm::vec3 >() );
				t.Scale = tc["Scale"].as< glm::vec3 >();
			}

			auto mc = entity[ "MeshComponent" ];
			if( mc )
			{
				auto& m = DeserialisedEntity.AddComponent< MeshComponent >();

				m.Mesh = Ref<Mesh>::Create( mc[ "Filepath" ].as<std::string>() );
			}

			auto slc = entity[ "SkyLightComponent" ];
			if( slc )
			{
				auto& s = DeserialisedEntity.AddComponent< SkylightComponent >();

				s.DynamicSky = slc["IsPreetham"].as< bool >();

				if( s.DynamicSky )
				{
					auto PreethamSettings = slc["Preetham Settings"];

					s.Turbidity = PreethamSettings["Turbidity"].as< float >();
					s.Azimuth = PreethamSettings["Azimuth"].as< float >();
					s.Inclination = PreethamSettings["Inclination"].as< float >();
				}
				else
				{
					// TODO...
				}
			}

			auto dlc = entity[ "DirectionalLightComponent" ];
			if( dlc )
			{
				auto& d = DeserialisedEntity.AddComponent< DirectionalLightComponent >();

				d.Radiance = dlc["Radiance"].as< glm::vec3 >();
				d.Intensity = dlc["Intensity"].as< float >();
				d.CastShadows = dlc["CastShadows"].as< bool >();
			}

			auto plc = entity[ "PointLightComponent" ];
			if( plc )
			{
				auto& p = DeserialisedEntity.AddComponent< PointLightComponent >();

				p.Radiance = plc[ "Radiance" ].as< glm::vec3 >();
				p.Intensity = plc[ "Intensity" ].as< float >();
				p.Multiplier = plc[ "Multiplier" ].as< float >();
				p.LightSize = plc[ "LightSize" ].as< float >();
				p.Radius = plc[ "Radius" ].as< float >();
				p.MinRadius = plc[ "MinRadius" ].as< float >();
				p.Falloff = plc[ "Falloff" ].as< float >();
			}

			auto bcc = entity[ "PhysXBoxColliderComponent" ];
			if( bcc )
			{
				auto& b = DeserialisedEntity.AddComponent< PhysXBoxColliderComponent >();

				b.Extents = bcc["Extents"].as< glm::vec3 >();
				b.Offset = bcc["Offset"].as< glm::vec3 >();
				b.IsTrigger = bcc["IsTrigger"].as< bool >();
			}

			auto scc = entity[ "PhysXSphereColliderComponent" ];
			if( scc )
			{
				auto& s = DeserialisedEntity.AddComponent< PhysXSphereColliderComponent >();

				s.Radius = scc[ "Radius" ].as< float >();
				s.Offset = scc[ "Offset" ].as< glm::vec3 >();
				s.IsTrigger = scc[ "IsTrigger" ].as< bool >();
			}
			
			auto ccc = entity[ "PhysXCapsuleColliderComponent" ];
			if( ccc )
			{
				auto& c = DeserialisedEntity.AddComponent< PhysXCapsuleColliderComponent >();

				c.Height = ccc[ "Height" ].as< float >();
				c.Radius = ccc[ "Radius" ].as< float >();
				c.Offset = ccc[ "Offset" ].as< glm::vec3 >();
				c.IsTrigger = ccc[ "IsTrigger" ].as< bool >();
			}

			auto rbc = entity[ "PhysXRigidbodyComponent" ];
			if( rbc )
			{
				auto& rb = DeserialisedEntity.AddComponent< PhysXRigidbodyComponent >();

				rb.IsKinematic = rbc["IsKinematic"].as< bool >();
				rb.UseCCD = rbc["CCD"].as< bool >();
				rb.Mass = rbc["Mass"].as< int >();
			}

			auto pmc = entity[ "PhysXMaterialComponent" ];
			if( pmc )
			{
				auto& m = DeserialisedEntity.AddComponent< PhysXMaterialComponent >();

				m.StaticFriction = pmc["StaticFriction"].as< float >();
				m.DynamicFriction = pmc["DynamicFriction"].as< float >();
				m.Restitution = pmc["Restitution"].as< float >();
			}
		}
	}

}