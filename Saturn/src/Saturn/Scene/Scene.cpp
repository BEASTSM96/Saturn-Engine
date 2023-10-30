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

#include "sppch.h"
#include "Scene.h"

#include "Saturn/Vulkan/SceneRenderer.h"
#include "Saturn/Vulkan/VulkanContext.h"

#include "Entity.h"
#include "Components.h"

#include "Saturn/Asset/Prefab.h"
#include "Saturn/Asset/AssetManager.h"

#include "Saturn/Core/OptickProfiler.h"

#include "Saturn/Physics/PhysicsScene.h"
#include "Saturn/Physics/PhysicsRigidBody.h"

#include "Saturn/GameFramework/Core/GameModule.h"

#include "Saturn/Serialisation/SceneSerialiser.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	static const std::string DefaultEntityName = "Empty Entity";
	std::unordered_map<UUID, Scene*> s_ActiveScenes;
	std::unordered_map< std::string, Ref<Asset> > s_PendingTravels;

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition( const glm::mat4& transform )
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose( transform, scale, orientation, translation, skew, perspective );

		return { translation, orientation, scale };
	}

	Scene::Scene()
	{
		s_ActiveScenes[ m_SceneID ] = this;
		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>( m_SceneEntity, m_SceneID );

		SetName( "Empty Scene" );
	}

	Scene::~Scene()
	{
		// Destroy All Physics Entities and static meshes
		{
			auto staticMeshes = GetAllEntitiesWith<StaticMeshComponent>();

			for( auto& entity : staticMeshes )
			{
				auto& rMeshComponent = entity->GetComponent<StaticMeshComponent>();

				if( rMeshComponent.Mesh )
					rMeshComponent.Mesh = nullptr;

				rMeshComponent.MaterialRegistry = nullptr;
			}

			// TODO: Is really needed? As the physics scene will destroy all of this.
			
			auto rigidBodies = GetAllEntitiesWith<RigidbodyComponent>();

			for( auto& entity : rigidBodies )
			{
				if( entity->GetComponent<RigidbodyComponent>().Rigidbody )
					delete entity->GetComponent<RigidbodyComponent>().Rigidbody;
			}
		}

		m_EntityIDMap.clear();
		m_Registry.clear();

		s_ActiveScenes.erase( m_SceneID );
	}

	// TODO: We don't want to search for the main camera entity every frame.
	Ref<Entity> Scene::GetMainCameraEntity()
	{
		auto entities = GetAllEntitiesWith<CameraComponent>();

		for( auto& entity : entities )
		{
			if( entity->GetComponent<CameraComponent>().MainCamera )
				return entity;
		}

		return nullptr;
	}

	void Scene::OnUpdate( Timestep ts )
	{
		SAT_PF_EVENT();

		// Update Cycle.
		// Step 1: Update and simulate the physics scene.
		// Step 2: Update all entities.

		// TODO: We might want to change the order of this update cycle.
		if( m_RuntimeRunning ) 
		{
			// Simulate the physics scene.
			m_PhysicsScene->Update( ts );
			OnUpdatePhysics( ts );

			for( auto&& [id, entity] : m_EntityIDMap )
			{
				entity->OnUpdate( ts );
			}
		}
	}
	
	void Scene::OnUpdatePhysics( Timestep ts )
	{
		SAT_PF_EVENT();

		// We can use a entt::view here because we are only accessing the rigid body.
		auto rigidBodies = GetAllEntitiesWith<RigidbodyComponent>();
		
		for( auto& entity : rigidBodies )
		{
			auto& rb = entity->GetComponent<RigidbodyComponent>();
			
			rb.Rigidbody->SyncTransfrom();
		}

		for( auto&& [id, entity] : m_EntityIDMap )
		{
			entity->OnPhysicsUpdate( ts );
		}
	}

	void Scene::OnRenderEditor( const EditorCamera& rCamera, Timestep ts, SceneRenderer& rSceneRenderer )
	{
		SAT_PF_EVENT();

		// Lights
		{
			m_Lights = Lights();

			// Directional Lights
			{
				auto lights = m_Registry.group<DirectionalLightComponent>( entt::get<TransformComponent> );
				uint32_t lightCount = 0;
				for( const auto& e : lights )
				{
					auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>( e );

					glm::vec3 direction = -glm::normalize( glm::mat3( transformComponent.GetTransform() ) * glm::vec3( 1.0f ) );

					m_Lights.DirectionalLights[ lightCount++ ] = { direction, lightComponent.Radiance, lightComponent.Intensity };
				}
			}

			// Point lights
			{
				auto points = m_Registry.group<PointLightComponent>( entt::get<TransformComponent> );

				uint32_t plIndex = 0;

				for( const auto& e : points )
				{
					auto [transformComponent, lightComponent] = points.get<TransformComponent, PointLightComponent>( e );

					PointLight pl = { 
						.Position = transformComponent.Position,
						.Radiance = lightComponent.Radiance,
						.Multiplier = lightComponent.Multiplier,
						.LightSize = lightComponent.LightSize,
						.Radius = lightComponent.Radius,
						.MinRadius = lightComponent.MinRadius,
						.Falloff = lightComponent.Falloff };

					m_Lights.PointLights.push_back( pl );

					plIndex++;
				}
			}
		}

		// Static meshes
		{
			auto entities = GetAllEntitiesWith<StaticMeshComponent>();

			for( auto& entity : entities )
			{
				auto& meshComponent = entity->GetComponent<StaticMeshComponent>();

				auto transform = GetTransformRelativeToParent( entity );

				if( meshComponent.Mesh )
				{
					Ref<MaterialRegistry> targetMaterialRegistry = meshComponent.Mesh->GetMaterialRegistry();
			
					if( meshComponent.MaterialRegistry && meshComponent.MaterialRegistry->HasAnyOverrides() )
						targetMaterialRegistry = meshComponent.MaterialRegistry;
					else
						targetMaterialRegistry = meshComponent.Mesh->GetMaterialRegistry();

					rSceneRenderer.SubmitStaticMesh( entity, meshComponent.Mesh, targetMaterialRegistry, transform );
				}
			}
		}

		rSceneRenderer.SetCamera( { rCamera, rCamera.ViewMatrix() } );
	}

	void Scene::OnRenderRuntime( Timestep ts, SceneRenderer& rSceneRenderer )
	{
		SAT_PF_EVENT();

		// Camera
		Ref<Entity> cameraEntity = GetMainCameraEntity();

		if( !cameraEntity )
			return;

		auto view = glm::inverse( GetTransformRelativeToParent( cameraEntity ) );
		SceneCamera& camera = cameraEntity->GetComponent<CameraComponent>().Camera;
	
		// Lights
		{
			m_Lights = Lights();

			// Directional Lights
			{
				auto lights = m_Registry.group<DirectionalLightComponent>( entt::get<TransformComponent> );
				uint32_t lightCount = 0;
				for( const auto& e : lights )
				{
					auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>( e );

					glm::vec3 direction = -glm::normalize( glm::mat3( transformComponent.GetTransform() ) * glm::vec3( 1.0f ) );

					m_Lights.DirectionalLights[ lightCount++ ] = { direction, lightComponent.Radiance, lightComponent.Intensity };
				}
			}

			// Point lights
			{
				auto points = m_Registry.group<PointLightComponent>( entt::get<TransformComponent> );

				//m_Lights.PointLights.resize( points.size() );

				uint32_t plIndex = 0;

				for( const auto& e : points )
				{
					auto [transformComponent, lightComponent] = points.get<TransformComponent, PointLightComponent>( e );

					PointLight pl = {
						.Position = transformComponent.Position,
						.Radiance = lightComponent.Radiance,
						.Multiplier = lightComponent.Multiplier,
						.LightSize = lightComponent.LightSize,
						.Radius = lightComponent.Radius,
						.MinRadius = lightComponent.MinRadius,
						.Falloff = lightComponent.Falloff };

					m_Lights.PointLights.push_back( pl );

					plIndex++;
				}
			}
		}

		// Static meshes
		{
			auto entities = GetAllEntitiesWith<StaticMeshComponent>();

			for( auto& entity : entities )
			{
				auto& meshComponent = entity->GetComponent<StaticMeshComponent>();

				auto transform = GetTransformRelativeToParent( entity );

				Ref<MaterialRegistry> targetMaterialRegistry = nullptr;

				if( meshComponent.MaterialRegistry->HasAnyOverrides() )
					targetMaterialRegistry = meshComponent.MaterialRegistry;
				else
					targetMaterialRegistry = meshComponent.Mesh->GetMaterialRegistry();

				if( meshComponent.Mesh )
					rSceneRenderer.SubmitStaticMesh( entity, meshComponent.Mesh, targetMaterialRegistry, transform );
			}
		}

		camera.SetViewportSize( rSceneRenderer.Width(), rSceneRenderer.Height() );
		rSceneRenderer.SetCamera( { camera, view } );
	}

	Ref<Entity> Scene::CreateEntityWithIDScript( UUID uuid, const std::string& name /*= "" */, const std::string& rScriptName )
	{
		Ref<Entity> entity = GameModule::Get().FindAndCallRegisterFunction( rScriptName );
		entity->SetName( name );
		entity->GetComponent<IdComponent>().ID = uuid;

		return entity;
	}

	Entity Scene::FindEntityByTag( const std::string& tag )
	{
		SAT_PF_EVENT();

		auto view = m_Registry.view<TagComponent>();
		for( auto entity : view )
		{
			const auto& canditate = view.get<TagComponent>( entity ).Tag;
			if( canditate == tag )
				return Entity( entity, this );
		}

		return Entity{};
	}

	Saturn::Ref<Saturn::Entity> Scene::FindEntityByID( const UUID& id )
	{
		SAT_PF_EVENT();

		for( auto&& [handle, entity] : m_EntityIDMap )
		{
			if( entity->GetUUID() == id )
				return entity;
		}

		return nullptr;
	}

	glm::mat4 Scene::GetTransformRelativeToParent( Ref<Entity> entity )
	{
		SAT_PF_EVENT();

		glm::mat4 transform( 1.0f );

		Ref<Entity> parent = FindEntityByID( entity->GetParent() );
		if( parent )
			transform = GetTransformRelativeToParent( parent );

		return transform * entity->GetComponent<TransformComponent>().GetTransform();
	}

	TransformComponent Scene::GetWorldSpaceTransform( Ref<Entity> entity )
	{
		SAT_PF_EVENT();

		TransformComponent tc;

		glm::mat4 worldSpace = GetTransformRelativeToParent( entity );
		glm::quat rotation{};

		Math::DecomposeTransform( worldSpace, tc.Position, rotation, tc.Scale );

		tc.SetRotation( rotation );

		return tc;
	}

	template<typename ...V>
	static void CopyComponent( entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap )
	{
		( [&]() 
		{
			auto components = srcRegistry.view<V>();
			for( auto srcEntity : components )
			{
				// Don't add to the scene entity.
				if( !srcRegistry.any_of<SceneComponent>( srcEntity ) )
				{
					entt::entity destEntity = enttMap.at( srcRegistry.get<IdComponent>( srcEntity ).ID );

					auto& srcComponent = srcRegistry.get<V>( srcEntity );
					auto& destComponent = dstRegistry.emplace_or_replace<V>( destEntity, srcComponent );
				}
			}
		}( ), ... );
	}

	template<typename ...V>
	static void CopyComponent( ComponentGroup<V...>, entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap )
	{
		CopyComponent<V...>( dstRegistry, srcRegistry, enttMap );
	}
	
	template<typename... V>
	static void CopyComponentIfExists( entt::entity dst, entt::entity src, entt::registry& rRegistry )
	{
		([&]()
		{
			if( rRegistry.any_of<V>( src ) )
			{
				auto& srcComponent = rRegistry.get<V>( src );
				rRegistry.emplace_or_replace<V>( dst, srcComponent );
			}
		}(), ... );
	}
	
	template<typename... V>
	static void CopyComponentIfExists( ComponentGroup<V...>, entt::entity dst, entt::entity src, entt::registry& rRegistry )
	{
		CopyComponentIfExists<V...>( dst, src, rRegistry );
	}

	void Scene::DuplicateEntity( Ref<Entity> entity )
	{
		Ref<Entity> newEntity = Ref<Entity>::Create();
		newEntity->SetName( entity->GetComponent<TagComponent>().Tag );

		// Without TagComponent and IdComponent
		using DesiredComponents = ComponentGroup<TransformComponent, RelationshipComponent, PrefabComponent,
			StaticMeshComponent,
			LightComponent, DirectionalLightComponent, SkylightComponent, PointLightComponent,
			CameraComponent,
			BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, MeshColliderComponent, RigidbodyComponent, PhysicsMaterialComponent,
			ScriptComponent, 
			AudioComponent>;

		CopyComponentIfExists( DesiredComponents{}, newEntity->GetHandle(), entity->GetHandle(), m_Registry );
	}

	void Scene::DeleteEntity( Ref<Entity> entity )
	{
		for ( auto& rChild : entity->GetChildren() )
		{
			auto child = FindEntityByID( rChild );

			m_EntityIDMap.erase( child->GetHandle() );
		}

		m_EntityIDMap.erase( entity->GetHandle() );
	}

	void Scene::CopyScene( Ref<Scene>& NewScene )
	{
		// Copy entities
		// I know we can just use the "=" operator, but we need to recreate the entities from the game.
		for( auto&& [id, entity] : m_EntityIDMap )
		{
			if( m_EntityIDMap[ id ]->HasComponent<ScriptComponent>() )
			{
				auto& rScriptComponent = m_EntityIDMap[ id ]->GetComponent<ScriptComponent>();

				NewScene->m_EntityIDMap[ id ] = NewScene->CreateEntityWithIDScript( m_EntityIDMap[ id ]->GetUUID(), m_EntityIDMap[ id ]->GetName(), rScriptComponent.ScriptName );
			}
			else
			{
				NewScene->m_EntityIDMap[ id ] = Ref<Entity>::Create( m_EntityIDMap[ id ]->GetName(), m_EntityIDMap[ id ]->GetUUID() );
			}	
		}

		NewScene->m_Name = m_Name;
		NewScene->m_Filepath = m_Filepath;

		NewScene->m_Lights = m_Lights;

		std::unordered_map< UUID, entt::entity > EntityMap;
		
		auto IdComponents = NewScene->GetAllEntitiesWith< IdComponent >();

		for( auto& entity : IdComponents )
			EntityMap[ entity->GetUUID() ] = entity->GetHandle();

		CopyComponent( AllComponents{}, NewScene->m_Registry, m_Registry, EntityMap );
	}

	void Scene::OnRuntimeStart()
	{
		if( m_PhysicsScene )
			delete m_PhysicsScene;

		m_RuntimeRunning = true;

		m_PhysicsScene = new PhysicsScene( this );

		for( auto&& [id, entity] : m_EntityIDMap )
		{
			entity->BeginPlay();
		}
	}

	void Scene::OnRuntimeEnd()
	{
		if( m_PhysicsScene )
			delete m_PhysicsScene;

		m_RuntimeRunning = false;
	}

	// Returns the Entity and the game class (if any).
	// This is not good as the SClass will hold the entity anyway.
	Ref<Entity> Scene::CreatePrefab( Ref<Prefab> prefabAsset )
	{
		Ref<Entity> prefabEntity = prefabAsset->PrefabToEntity( this );

		return prefabEntity;
	}

	void Scene::SetActiveScene( Scene* pScene )
	{
		GActiveScene = pScene;
	}

	Scene* Scene::GetActiveScene()
	{
		return GActiveScene;
	}

	void Scene::OnEntityCreated( Ref<Entity> entity )
	{
		m_EntityIDMap[ entity->GetHandle() ] = entity;
	}

	bool Scene::Travel( const std::string& rSceneName )
	{
		// Because we don't load scenes from the asset importer, we have to manually load it.
		Ref<Asset> asset = AssetManager::Get().FindAsset( rSceneName, AssetType::Scene );

		if( !asset )
			return false;

		// We cannot just immediately start to open the scene we need to do it next frame.
		s_PendingTravels[ rSceneName ] = asset;

		return true;
	}

	bool Scene::AwaitingTravels()
	{
		return false;
	}

	void Scene::DoTravel()
	{
		for( auto&& [Name, asset] : s_PendingTravels )
		{
			Ref<Scene> newScene = Ref<Scene>::Create();
			Scene::SetActiveScene( newScene.Get() );

			SceneSerialiser ss( newScene );
			ss.Deserialise( asset->Path.string() );
		}
	}

}