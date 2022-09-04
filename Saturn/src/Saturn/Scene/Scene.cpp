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
#include "Scene.h"

#include "Saturn/Vulkan/SceneRenderer.h"
#include "Saturn/Vulkan/VulkanContext.h"

#include "Saturn/PhysX/PhysXRuntime.h"
#include "Saturn/PhysX/PhysXRigidBody.h"

#include "Entity.h"
#include "Components.h"

#define GLM_ENABLE_EXPERIMENTAL
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

		// Destroy all entities with mesh component.
		auto group = m_Registry.group<MeshComponent>( entt::get<TransformComponent> );
		
		for ( const auto& e : group )
		{
			Entity entity( e, this );

			Ref< Mesh >& mesh = entity.GetComponent<MeshComponent>().Mesh;

			if( mesh )
				mesh = nullptr;
		}

		m_Registry.clear();
	}

	void Scene::OnUpdate( Timestep ts )
	{
		if( m_RuntimeRunning ) 
		{
			auto view = m_Registry.view<PhysXRigidbodyComponent>();

			for( auto entity : view )
			{
				Entity e{ entity, this };
				auto& rb = e.GetComponent<PhysXRigidbodyComponent>();
				rb.m_Rigidbody->SetUserData( e );
			}

			m_PhysXRuntime->Update( ts, *this );
		}
	}

	void Scene::OnUpdatePhysics( Timestep ts )
	{
		auto PhysXView = m_Registry.view<TransformComponent, PhysXRigidbodyComponent>();
		
		for ( const auto& entity : PhysXView )
		{
			auto [tc, rb] = PhysXView.get<TransformComponent, PhysXRigidbodyComponent>( entity );
			
			tc.Position = rb.m_Rigidbody->GetPosition();
			rb.m_Rigidbody->GetPosition() = tc.Position;
		}
	}

	void Scene::OnRenderEditor( const EditorCamera& rCamera, Timestep ts )
	{
		auto group = m_Registry.group<MeshComponent>( entt::get<TransformComponent> );
		
		SceneRenderer::Get().SetCurrentScene( this );

		// Lights
		{
			auto lights = m_Registry.group<DirectionalLightComponent>( entt::get<TransformComponent> );
			uint32_t lightCount = 0;
			for( const auto& e : lights )
			{
				auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>( e );
				
				glm::vec3 direction = -glm::normalize( glm::mat3( transformComponent.GetTransform() ) * glm::vec3( 1.0f ) );
				
				m_DirectionalLight[ lightCount++ ] = { direction, lightComponent.Radiance, lightComponent.Intensity };
			}
		}

		for( const auto e : group )
		{
			Entity entity( e, this );
			
			auto [meshComponent, transformComponent] = group.get<MeshComponent, TransformComponent>( entity );
			
			if( meshComponent.Mesh ) 
			{
				if( m_SelectedEntity == e )
					SceneRenderer::Get().SubmitSelectedMesh( entity, meshComponent.Mesh, transformComponent.GetTransform() );
				else
					SceneRenderer::Get().SubmitMesh( entity, meshComponent.Mesh, transformComponent.GetTransform() );
			}

			SceneRenderer::Get().SetEditorCamera( rCamera );
		}	
	}

	Entity Scene::CreateEntity( const std::string& name /*= "" */ )
	{
		Entity entity ={ m_Registry.create(), this };
		
		auto& idComponent = entity.AddComponent<IdComponent>().ID = {};
		entity.AddComponent<TransformComponent>();
		auto& tagComponent = entity.AddComponent<TagComponent>( name.empty() ? "Empty Entity" : name );

		entity.AddComponent<VisibilityComponent>();
		
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
				if( !srcRegistry.has<SceneComponent>( srcEntity ) )
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
			if( rRegistry.has<V>( src ) )
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

		CopyComponentIfExists( AllComponents{}, newEntity, entity, m_Registry );
	}

	void Scene::DeleteEntity( Entity entity )
	{
		m_Registry.destroy( entity );
	}

	void Scene::CopyScene( Ref<Scene>& NewScene )
	{
		NewScene->m_EntityIDMap = m_EntityIDMap;
		NewScene->m_Name = m_Name;
		NewScene->m_Filepath = m_Filepath;

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
		m_PhysXRuntime = new PhysXRuntime();
		m_PhysXRuntime->CreateScene();

		auto view = m_Registry.view<PhysXRigidbodyComponent>();
		
		for( auto entity : view )
		{
			Entity e{ entity, this };
			m_PhysXRuntime->AddRigidbody( e );
			auto& rb = e.GetComponent<PhysXRigidbodyComponent>();
			rb.m_Rigidbody->AddActorToScene();
		}
	}

	void Scene::OnRuntimeEnd()
	{
		m_PhysXRuntime->Clear();
		delete m_PhysXRuntime;
	}

}