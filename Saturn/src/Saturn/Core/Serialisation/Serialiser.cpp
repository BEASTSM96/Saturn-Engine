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

#include "sppch.h"
#include "Serialiser.h"

#include <yaml-cpp/yaml.h>

#include "Saturn/Application.h"
#include "Saturn/Scene/Scene.h"

#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Entity.h"
#include "Saturn/Core/EngineSettings/EngineSettings.h"

#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <string.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode( const glm::vec2& rhs )
		{
			Node node;
			node.push_back( rhs.x );
			node.push_back( rhs.y );
			return node;
		}

		static bool decode( const Node& node, glm::vec2& rhs )
		{
			if( !node.IsSequence() || node.size() != 2 )
				return false;

			rhs.x = node[ 0 ].as<float>();
			rhs.y = node[ 1 ].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode( const glm::vec3& rhs )
		{
			Node node;
			node.push_back( rhs.x );
			node.push_back( rhs.y );
			node.push_back( rhs.z );
			node.SetStyle( EmitterStyle::Flow );
			return node;
		}

		static bool decode( const Node& node, glm::vec3& rhs )
		{
			if( !node.IsSequence() || node.size() != 3 )
				return false;

			rhs.x = node[ 0 ].as<float>();
			rhs.y = node[ 1 ].as<float>();
			rhs.z = node[ 2 ].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode( const glm::vec4& rhs )
		{
			Node node;
			node.push_back( rhs.x );
			node.push_back( rhs.y );
			node.push_back( rhs.z );
			node.push_back( rhs.w );
			node.SetStyle( EmitterStyle::Flow );
			return node;
		}

		static bool decode( const Node& node, glm::vec4& rhs )
		{
			if( !node.IsSequence() || node.size() != 4 )
				return false;

			rhs.x = node[ 0 ].as<float>();
			rhs.y = node[ 1 ].as<float>();
			rhs.z = node[ 2 ].as<float>();
			rhs.w = node[ 3 ].as<float>();
			return true;
		}
	};

	template <>
	struct convert<glm::quat>
	{
		static Node encode( const glm::quat& rhs )
		{
			Node node;
			node.push_back( rhs.w );
			node.push_back( rhs.x );
			node.push_back( rhs.y );
			node.push_back( rhs.z );
			node.SetStyle( EmitterStyle::Flow );
			return node;
		}

		static bool decode( const Node& node, glm::quat& rhs )
		{
			if( !node.IsSequence() || node.size() != 4 )
			{
				return false;
			}

			rhs.w = node[ 0 ].as<float>();
			rhs.x = node[ 1 ].as<float>();
			rhs.y = node[ 2 ].as<float>();
			rhs.z = node[ 3 ].as<float>();

			return true;
		}
	};

}


namespace Saturn {

	YAML::Emitter& operator<<( YAML::Emitter& out, const glm::vec2& v )
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<( YAML::Emitter& out, const glm::vec3& v )
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}


	YAML::Emitter& operator<<( YAML::Emitter& out, const glm::vec4& v )
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<( YAML::Emitter& out, const glm::quat& v )
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	Serialiser::Serialiser( const std::string& objectname, bool shouldSerialise ) : m_shouldSerialise( true ), m_ObjectName( objectname )
	{
	}

	Serialiser::Serialiser( const Ref<Scene>& scene )
		: m_Scene( scene ), m_shouldSerialise( true )
	{
	}

	Serialiser::Serialiser()
	{
	}

	Serialiser::~Serialiser() { }

	void Serialiser::SerialiseEntity( YAML::Emitter& out, Entity entity )
	{
		UUID uuid = entity.GetComponent<IdComponent>().ID;
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << uuid; // TODO: Entity ID goes here

		if( entity.HasComponent<TagComponent>() )
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if( entity.HasComponent<TransformComponent>() )
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << tc.Position;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if( entity.HasComponent<MeshComponent>() )
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; // MeshComponent

			auto& mesh = entity.GetComponent<MeshComponent>().Mesh;
			out << YAML::Key << "AssetPath" << YAML::Value << mesh->GetFilePath();

			out << YAML::EndMap; // MeshComponent
		}

		//PhysX

		if( entity.HasComponent<PhysXRigidbodyComponent>() )
		{
			out << YAML::Key << "PhysXRigidbodyComponent";
			out << YAML::BeginMap;

			auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
			auto& mat = entity.GetComponent<PhysXMaterialComponent>();
			out << YAML::Key << "Kinematic" << YAML::Value << rb.isKinematic;
			out << YAML::Key << "Mass" << YAML::Value << rb.Mass;
			out << YAML::Key << "CCD" << YAML::Value << rb.UseCCD;
			out << YAML::Key << "Material-DynamicFriction" << YAML::Value << mat.DynamicFriction;
			out << YAML::Key << "Material-StaticFriction" << YAML::Value << mat.StaticFriction;
			out << YAML::Key << "Material-Restitution" << YAML::Value << mat.Restitution;

			out << YAML::EndMap;
		}

		if( entity.HasComponent<PhysXBoxColliderComponent>() )
		{
			out << YAML::Key << "PhysXBoxColliderComponent";
			out << YAML::BeginMap;

			auto& bc = entity.GetComponent<PhysXBoxColliderComponent>();
			out << YAML::Key << "Extents" << YAML::Value << bc.Extents;

			out << YAML::EndMap;
		}

		if( entity.HasComponent<PhysXSphereColliderComponent>() )
		{
			out << YAML::Key << "PhysXSphereColliderComponent";
			out << YAML::BeginMap;

			auto& sc = entity.GetComponent<PhysXSphereColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << sc.Radius;

			out << YAML::EndMap;
		}

		if( entity.HasComponent<PhysXCapsuleColliderComponent>() )
		{
			out << YAML::Key << "PhysXCapsuleColliderComponent";
			out << YAML::BeginMap;

			auto& cc = entity.GetComponent<PhysXCapsuleColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << cc.Radius;
			out << YAML::Key << "Height" << YAML::Value << cc.Height;

			out << YAML::EndMap;
		}

		if( entity.HasComponent<SkyLightComponent>() )
		{
			out << YAML::Key << "SkyLightComponent";
			out << YAML::BeginMap;

			auto& skl = entity.GetComponent<SkyLightComponent>();
			out << YAML::Key << "Environment File path" << YAML::Value << skl.EnvironmentFilepath;

			out << YAML::EndMap;
		}

		out << YAML::EndMap; // Entity
	}

	void Serialiser::SerialiseVC( const std::string& filepath )
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Version Ctrl for Saturn";
		out << YAML::Value << " ";
		out << YAML::Value << "uuid" << YAML::Value << (uint64_t)Application::Get().GetFixedVersionUUID();
		out << YAML::Value << "Branch" << YAML::Value << Application::Get().GetVersionCtrl().Branch;
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout( filepath );
		fout << out.c_str();
	}

	void Serialiser::DeserialiseVC( const std::string& filepath )
	{
		if( filepath.empty() )
			return;
		YAML::Node data = YAML::LoadFile( filepath );
		if( !data[ "Version Ctrl for Saturn" ] )
			return;

		SAT_CORE_INFO( "Starting deserializing of version control info" );
		uint64_t vcUUID = data[ "uuid" ].as<uint64_t>();
		std::string branch = data[ "Branch" ].as<std::string>();

		Application::Get().GetVersionCtrl().FixedUUID = vcUUID;
		Application::Get().GetVersionCtrl().Branch = branch;
	}

	void Serialiser::SerialiseProjectSettings( const std::string& filepath )
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Project Settings";
		out << YAML::Value << " ";
		out << YAML::Value << "Startup Scene" << YAML::Value << ProjectSettings::GetStartupSceneName();
		out << YAML::Value << "Startup Project Name" << YAML::Value << ProjectSettings::GetStartupProjectName();
		out << YAML::Value << "Startup Project Folder" << YAML::Value << ProjectSettings::GetStartupProjectFolder();
		out << YAML::Key << "Last project settings" << YAML::Value;
		out << YAML::BeginMap; // Last project
		out << YAML::Value << "Last Project Folder" << YAML::Value << ProjectSettings::GetCurrentProject()->GetAssetsFolderPath();
		out << YAML::Value << "Last Project Name" << YAML::Value << ProjectSettings::GetCurrentProject()->GetName();
		out << YAML::EndMap; // Last project
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout( filepath );
		fout << out.c_str();
	}

	void Serialiser::DeserialiseProjectSettings( const std::string& filepath )
	{
		if( filepath.empty() )
			return;

		YAML::Node data = YAML::LoadFile( filepath );

		if( !data[ "Project Settings" ] )
			return;

		SAT_CORE_INFO( "Starting deserializing of Project Settings" );

		std::string sceneName = data[ "Startup Scene" ].as<std::string>();
		std::string startupProjectName = data[ "Startup Project Name" ].as<std::string>();
		std::string startupProjectFolder = data[ "Startup Project Folder" ].as<std::string>();

		std::string projectFolder;
		std::string projectName;

		auto lastProjectNode = data[ "Last project settings" ];
		if( lastProjectNode )
		{
			projectFolder = lastProjectNode[ "Last Project Folder" ].as<std::string>();
			projectName = lastProjectNode[ "Last Project Name" ].as<std::string>();
		}

		Ref<Project> project;

		if( startupProjectFolder.empty() && startupProjectName.empty() )
			project = Ref<Project>::Create( projectFolder, projectName );
		else
			project = Ref<Project>::Create( "assets\\" + startupProjectFolder, startupProjectName );

		ProjectSettings::SetCurrentProject( project );
		ProjectSettings::SetStartupSceneName( sceneName );
	}

	void Serialiser::SerialiseProject( const std::string& filepath )
	{

	}

	void Serialiser::DeserialiseProject( const std::string& filepath )
	{

	}

	static void SerialiseEnvironment( YAML::Emitter& out, const Ref<Scene>& scene )
	{
		out << YAML::Key << "Environment";
		out << YAML::Value;
		out << YAML::BeginMap; // Environment
		out << YAML::Key << "AssetPath" << YAML::Value << scene->GetEnvironment().FilePath;
		const auto& light = scene->GetLight();
		out << YAML::Key << "Light" << YAML::Value;
		out << YAML::BeginMap; // Light
		out << YAML::Key << "Direction" << YAML::Value << light.Direction;
		out << YAML::Key << "Radiance" << YAML::Value << light.Radiance;
		out << YAML::Key << "Multiplier" << YAML::Value << light.Multiplier;
		out << YAML::EndMap; // Light
		out << YAML::EndMap; // Environment
	}

	void Serialiser::Serialise( const std::string& filepath )
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << "Scene Name";
		SerialiseEnvironment( out, m_Scene );
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each( [&]( auto entityID )
			{
				Entity entity ={ entityID, m_Scene.Raw() };

				if( !entity )
					return;

				if( entity.HasComponent<SceneComponent>() )
					return;

				SerialiseEntity( out, entity );

			} );
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout( filepath );
		fout << out.c_str();

	}

	void Serialiser::Deserialise( const std::string& filepath )
	{
		if( filepath.empty() )
			return;

		YAML::Node data = YAML::LoadFile( filepath );

		if( !data[ "Scene" ] )
			return;

		std::string sceneName = data[ "Scene" ].as<std::string>();
		SAT_CORE_INFO( "Deserializing scene '{0}'", sceneName );

		auto environment = data[ "Environment" ];
		if( environment )
		{
			std::string envPath = environment[ "AssetPath" ].as<std::string>();
			//if( envPath != "null" )
				//m_Scene->SetEnvironment( Environment::Load( envPath ) );

			auto lightNode = environment[ "Light" ];
			if( lightNode )
			{
				auto& light = m_Scene->GetLight();
				light.Direction = lightNode[ "Direction" ].as<glm::vec3>();
				light.Radiance = lightNode[ "Radiance" ].as<glm::vec3>();
				light.Multiplier = lightNode[ "Multiplier" ].as<float>();
			}
		}

		auto entities = data[ "Entities" ];
		if( entities )
		{
			for( auto entity : entities )
			{
				uint64_t uuid = entity[ "Entity" ].as<uint64_t>();

				std::string name;
				auto tagComponent = entity[ "TagComponent" ];
				if( tagComponent )
					name = tagComponent[ "Tag" ].as<std::string>();

				SAT_CORE_INFO( "Deserialized entity with ID ({0}), name = {1}", uuid, name );

				Entity deserializedEntity = m_Scene->CreateEntityWithID( uuid, name );

				auto transformComponent = entity[ "TransformComponent" ];
				if( transformComponent )
				{
					// Entities always have transforms
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Position = transformComponent[ "Position" ].as<glm::vec3>();
					tc.Rotation = transformComponent[ "Rotation" ].as<glm::vec3>();
					tc.Scale = transformComponent[ "Scale" ].as<glm::vec3>();

					SAT_CORE_INFO( "  Entity Transform:" );
					SAT_CORE_INFO( "    Translation: {0}, {1}, {2}", tc.Position.x, tc.Position.y, tc.Position.z );
					SAT_CORE_INFO( "    Rotation: {0}, {1}, {2}", tc.Rotation.x, tc.Rotation.y, tc.Rotation.z );
					SAT_CORE_INFO( "    Scale: {0}, {1}, {2}", tc.Scale.x, tc.Scale.y, tc.Scale.z );

				}

				auto meshComponent = entity[ "MeshComponent" ];
				if( meshComponent )
				{
					std::string meshPath = meshComponent[ "AssetPath" ].as<std::string>();
					if( !deserializedEntity.HasComponent<MeshComponent>() )
						deserializedEntity.AddComponent<MeshComponent>( Ref<Mesh>::Create( meshPath ) );

					SAT_CORE_INFO( "  Mesh Asset Path: {0}", meshPath );
				}

				auto nativeScriptComponent = entity[ "NativeScriptComponent" ];
				if( nativeScriptComponent )
				{
					std::string instancePointer = meshComponent[ "Instance pointer" ].as<std::string>();
					//if( !deserializedEntity.HasComponent<MeshComponent>() )
					//	deserializedEntity.AddComponent<MeshComponent>( Ref<Mesh>::Create( meshPath ) );

					SAT_CORE_INFO( " NativeScriptComponent Instance Pointer: {0}", instancePointer );
				}

				//PhysX

				auto physxrigidbodyComponent = entity[ "PhysXRigidbodyComponent" ];
				if( physxrigidbodyComponent )
				{
					bool iskinematic = physxrigidbodyComponent[ "Kinematic" ].as<bool>();
					float mass = physxrigidbodyComponent[ "Mass" ].as<float>();
					bool ccd = physxrigidbodyComponent[ "CCD" ].as<bool>();

					int dynamicFriction = physxrigidbodyComponent["Material-DynamicFriction"].as<float>();
					int restitution = physxrigidbodyComponent[ "Material-Restitution" ].as<float>();
					int staticFriction = physxrigidbodyComponent[ "Material-StaticFriction" ].as<float>();

					if( !deserializedEntity.HasComponent<PhysXRigidbodyComponent>() ) 
					{
						deserializedEntity.AddComponent<PhysXRigidbodyComponent>().isKinematic = iskinematic;
						deserializedEntity.AddComponent<PhysXMaterialComponent>();
						auto& rb = deserializedEntity.GetComponent<PhysXRigidbodyComponent>();
						rb.Mass = mass;
						rb.UseCCD = ccd;
						auto& mat = deserializedEntity.GetComponent<PhysXMaterialComponent>();
						mat.DynamicFriction = dynamicFriction;
						mat.Restitution = restitution;
						mat.StaticFriction = staticFriction;
					}

					SAT_CORE_INFO( " PhysXRigidbodyComponent isKinematic: {0}", iskinematic );
				}

				auto physxboxcolliderComponent = entity[ "PhysXBoxColliderComponent" ];
				if( physxboxcolliderComponent )
				{
					glm::vec3 extents = physxboxcolliderComponent[ "Extents" ].as<glm::vec3>();
					if( !deserializedEntity.HasComponent<PhysXBoxColliderComponent>() )
						deserializedEntity.AddComponent<PhysXBoxColliderComponent>().Extents = extents;

					SAT_CORE_INFO( " PhysXBoxColliderComponent Extents: {0}, {1}, {2}", extents.x, extents.y, extents.z );
				}

				auto physxspherecolliderComponent = entity[ "PhysXSphereColliderComponent" ];
				if( physxspherecolliderComponent )
				{
					float radius = physxspherecolliderComponent[ "Radius" ].as<float>();
					if( !deserializedEntity.HasComponent<PhysXSphereColliderComponent>() )
						deserializedEntity.AddComponent<PhysXSphereColliderComponent>().Radius = radius;

					SAT_CORE_INFO( " PhysXSphereColliderComponent Radius: {0}", radius );
				}

				auto physxcapsulecolliderComponent = entity[ "PhysXCapsuleColliderComponent" ];
				if( physxcapsulecolliderComponent )
				{
					float radius = physxcapsulecolliderComponent[ "Radius" ].as<float>();
					float height = physxcapsulecolliderComponent[ "Height" ].as<float>();
					if( !deserializedEntity.HasComponent<PhysXCapsuleColliderComponent>() )
						deserializedEntity.AddComponent<PhysXCapsuleColliderComponent>().Radius = radius;
					deserializedEntity.GetComponent<PhysXCapsuleColliderComponent>().Height = height;

					SAT_CORE_INFO( " PhysXCapsuleColliderComponent Radius, Height: {0} {1}", radius, height );
				}

				auto skylightcomponent = entity[ "SkyLightComponent" ];
				if( skylightcomponent )
				{
					std::string filepath = skylightcomponent[ "Environment File path" ].as<std::string>();

					if( !deserializedEntity.HasComponent<SkyLightComponent>() )
						deserializedEntity.AddComponent<SkyLightComponent>().EnvironmentFilepath = filepath;

					deserializedEntity.GetComponent<SkyLightComponent>().EnvironmentFilepath = filepath;

					m_Scene->SetEnvironment( Environment::Load( filepath ) );

					SAT_CORE_INFO( " SkyLightComponent Filepath: {0}", filepath );
				}
			}
		}

	}

}
