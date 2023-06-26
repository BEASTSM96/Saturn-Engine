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

#include "Saturn/GameFramework/EntityScriptManager.h"

#include "Entity.h"
#include "Components.h"

#include "Saturn/Asset/Prefab.h"

#include "Saturn/Core/OptickProfiler.h"

#include "Saturn/Physics/PhysicsScene.h"
#include "Saturn/Physics/PhysicsRigidBody.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	static const std::string DefaultEntityName = "Empty Entity";
	std::unordered_map<UUID, Scene*> s_ActiveScenes;

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
		s_ActiveScenes.erase( m_SceneID );
		m_Registry.clear();
	}

	Entity Scene::GetMainCameraEntity()
	{
		auto view = GetAllEntitiesWith<CameraComponent>();

		for ( const auto& entity : view )
		{
			if( m_Registry.get<CameraComponent>( entity ).MainCamera )
				return Entity( entity, this );
		}

		return {};
	}

	void Scene::OnUpdate( Timestep ts )
	{
		SAT_PF_EVENT();

		// TODO: We might want to change the order of this update cycle.
		if( m_RuntimeRunning ) 
		{
			m_PhysicsScene->Update( ts );
			OnUpdatePhysics( ts );

			//EntityScriptManager::Get().UpdateAllScripts( ts );
		}
	}

	void Scene::OnUpdatePhysics( Timestep ts )
	{
		SAT_PF_EVENT();

		auto PhysXView = m_Registry.view<TransformComponent, RigidbodyComponent>();
		
		for( const auto& entity : PhysXView )
		{
			auto [tc, rb] = PhysXView.get<TransformComponent, RigidbodyComponent>( entity );
			rb.Rigidbody->SyncTransfrom();
		}

		//EntityScriptManager::Get().OnPhysicsUpdate( ts );
	}

	void Scene::OnRenderEditor( const EditorCamera& rCamera, Timestep ts, SceneRenderer& rSceneRenderer )
	{
		SAT_PF_EVENT();

		auto group = m_Registry.group<StaticMeshComponent>( entt::get<TransformComponent> );

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
			for( const auto e : group )
			{
				Entity entity( e, this );

				auto [meshComponent, transformComponent] = group.get<StaticMeshComponent, TransformComponent>( entity );

				auto transform = GetTransformRelativeToParent( entity );

				if( meshComponent.Mesh )
					rSceneRenderer.SubmitStaticMesh( entity, meshComponent.Mesh, transform );
			}
		}

		rSceneRenderer.SetCamera( { rCamera, rCamera.ViewMatrix() } );
	}

	void Scene::OnRenderRuntime( Timestep ts, SceneRenderer& rSceneRenderer )
	{
		SAT_PF_EVENT();

		// Camera
		Entity cameraEntity = GetMainCameraEntity();

		if( !cameraEntity )
			return;

		auto view = glm::inverse( GetTransformRelativeToParent( cameraEntity ) );
		SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
	
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
			auto group = m_Registry.group<StaticMeshComponent>( entt::get<TransformComponent> );
			for( const auto e : group )
			{
				Entity entity( e, this );

				auto [meshComponent, transformComponent] = group.get<StaticMeshComponent, TransformComponent>( entity );

				auto transform = GetTransformRelativeToParent( entity );

				if( meshComponent.Mesh )
					rSceneRenderer.SubmitStaticMesh( entity, meshComponent.Mesh, transform );
			}
		}

		camera.SetViewportSize( rSceneRenderer.Width(), rSceneRenderer.Height() );
		rSceneRenderer.SetCamera( { camera, view } );
	}

	Entity Scene::CreateEntity( const std::string& name /*= "" */ )
	{
		Entity entity ={ m_Registry.create(), this };
		entity.AddComponent<RelationshipComponent>();
		entity.AddComponent<TransformComponent>();
		
		auto& idComponent = entity.AddComponent<IdComponent>().ID = {};
		auto& tagComponent = entity.AddComponent<TagComponent>( name.empty() ? "Empty Entity" : name );
		
		m_EntityIDMap[ idComponent ] = entity;
		
		return entity;
	}

	Entity Scene::CreateEntityWithID( UUID uuid, const std::string& name /*= "" */ )
	{
		auto entity = Entity{ m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<IdComponent>();
		idComponent.ID = uuid;

		entity.AddComponent<TransformComponent>();
		if( !name.empty() )
			entity.AddComponent<TagComponent>( name );

		//SAT_CORE_ASSERT( m_EntityIDMap.find( uuid ) == m_EntityIDMap.end(), "Entity has the same name!" );
		m_EntityIDMap[ uuid ] = entity;

		entity.AddComponent<RelationshipComponent>();

		return entity;
	}

	Entity Scene::FindEntityByTag( const std::string& tag )
	{
		auto view = m_Registry.view<TagComponent>();
		for( auto entity : view )
		{
			const auto& canditate = view.get<TagComponent>( entity ).Tag;
			if( canditate == tag )
				return Entity( entity, this );
		}

		return Entity{};
	}

	Entity Scene::FindEntityByID( const UUID& id )
	{
		auto view = m_Registry.view<IdComponent>();
		for( auto entity : view )
		{
			const auto& canditate = view.get<IdComponent>( entity ).ID;
			if( canditate == id )
				return Entity( entity, this );
		}

		return Entity{};
	}

	glm::mat4 Scene::GetTransformRelativeToParent( Entity entity )
	{
		glm::mat4 transform( 1.0f );

		Entity parent = FindEntityByID( entity.GetParent() );
		if( parent )
			transform = GetTransformRelativeToParent( parent );

		return transform * entity.GetComponent<TransformComponent>().GetTransform();
	}

	TransformComponent Scene::GetWorldSpaceTransform( Entity entity )
	{
		TransformComponent tc;

		glm::mat4 worldSpace = GetTransformRelativeToParent( entity );
		Math::DecomposeTransform( worldSpace, tc.Position, tc.Rotation, tc.Scale );

		return tc;
	}

	void Scene::DestroyEntity( Entity entity )
	{
		m_Registry.destroy( entity.m_EntityHandle );
	}

	template<typename ...V>
	static void CopyComponent( entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap )
	{
		( [&]() 
		{
			auto components = srcRegistry.view<V>();
			for( auto srcEntity : components )
			{
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

	void Scene::DuplicateEntity( Entity entity )
	{
		Entity newEntity;

		newEntity = CreateEntity( entity.GetComponent<TagComponent>().Tag );

		// Without TagComponent and IdComponent
		using DesiredComponents = ComponentGroup<TransformComponent, RelationshipComponent, PrefabComponent,
			StaticMeshComponent,
			LightComponent, DirectionalLightComponent, SkylightComponent, PointLightComponent,
			CameraComponent,
			BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, MeshColliderComponent, RigidbodyComponent, PhysicsMaterialComponent,
			ScriptComponent, 
			AudioComponent>;

		CopyComponentIfExists( DesiredComponents{}, newEntity, entity, m_Registry );
	}

	void Scene::DeleteEntity( Entity entity )
	{
		for ( auto& rChild : entity.GetChildren() )
		{
			auto child = FindEntityByID( rChild );

			m_Registry.destroy( child );
		}

		m_Registry.destroy( entity );
	}

	void Scene::CopyScene( Ref<Scene>& NewScene )
	{
		NewScene->m_EntityIDMap = m_EntityIDMap;

		NewScene->m_Name = m_Name;
		NewScene->m_Filepath = m_Filepath;

		NewScene->m_Lights = m_Lights;

		std::unordered_map< UUID, entt::entity > EntityMap;
		
		auto IdComponents = m_Registry.view< IdComponent >();

		for( auto entity : IdComponents )
		{
			auto uuid = m_Registry.get<IdComponent>( entity ).ID;
			Entity e = NewScene->CreateEntityWithID( uuid );
			EntityMap[ uuid ] = e.m_EntityHandle;
		}

		CopyComponent( AllComponents{}, NewScene->m_Registry, m_Registry, EntityMap );
	}

	void Scene::OnRuntimeStart()
	{
		if( m_PhysicsScene )
			delete m_PhysicsScene;

		m_PhysicsScene = new PhysicsScene( this );

		EntityScriptManager::Get().BeginPlay();
	}

	void Scene::OnRuntimeEnd()
	{
		delete m_PhysicsScene;
	}

	// Returns the Entity and the game class (if any).
	// This is not good as the SClass will hold the entity anyway.
	std::pair<Entity, SClass*> Scene::CreatePrefab( Ref<Prefab> prefabAsset )
	{
		Entity prefabEntity;
		SClass* sc = nullptr;

		prefabEntity = prefabAsset->PrefabToEntity( this, prefabEntity );

		if( prefabEntity.HasComponent<ScriptComponent>() ) 
		{
			// Try register
			EntityScriptManager::Get().RegisterScript( prefabEntity.GetComponent<ScriptComponent>().ScriptName );

			sc = EntityScriptManager::Get().CreateScript( prefabEntity.GetComponent<ScriptComponent>().ScriptName, (SClass*)&prefabEntity );
		}

		return { prefabEntity, sc };
	}

	static Scene* s_ActiveScene = nullptr;

	void Scene::SetActiveScene( Scene* pScene )
	{
		s_ActiveScene = pScene;
	}

	Scene* Scene::GetActiveScene()
	{
		return s_ActiveScene;
	}

}