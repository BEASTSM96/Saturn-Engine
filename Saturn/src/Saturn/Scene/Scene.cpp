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
#include "Scene.h"

#include "Entity.h"

#include "Components.h"

#include "Saturn/Renderer/SceneRenderer.h"

#include "SceneManager.h"

#include "Saturn/Application.h"
#include "Saturn/GameFramework/HotReload.h"

#include "ScriptableEntity.h"

#include "Saturn/GameFramework/Character.h"

#include "Saturn/Physics/beastPhysics/PhysicsWorld.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	static const std::string DefaultEntityName = "Entity";

	std::unordered_map<UUID, Scene*> s_ActiveScenes;

	struct SceneComponent
	{
		UUID SceneID;
	};

	void Scene::PhysicsComponentCreate( entt::registry& r, entt::entity ent )
	{
		if (!r.has<TransformComponent>(ent))
		{
			SAT_CORE_ERROR( "PhysicsComponent needs a TransformComponent!" );
			return;
		}

		auto& physics = r.get<RigidbodyComponent>( ent );
		auto& trans = r.get<TransformComponent>( ent );
		physics.m_body = new Rigidbody( m_ReactPhysicsScene.Raw(), true, trans.Position );

		if (physics.isKinematic)
		{
			physics.m_body->SetKinematic( true );
		}


		if( r.has<BoxColliderComponent>( ent ) )
		{
			physics.m_body->AddBoxCollider(r.get<BoxColliderComponent>(ent).Extents);
		}

		if( r.has<SphereColliderComponent>( ent ) )
		{
			physics.m_body->AddSphereCollider( r.get<SphereColliderComponent>( ent ).Radius );
		}

		if ( !r.has<SphereColliderComponent>(ent) && !r.has<BoxColliderComponent>( ent ) )
		{
			SAT_CORE_WARN("You need to add a ColliderComponent Sphere or box.");
		}

	}

	Scene::Scene( void )
	{
		SAT_PROFILE_FUNCTION();

		auto skyboxShader = Shader::Create( "assets/shaders/Skybox.glsl" );
		m_SkyboxMaterial = MaterialInstance::Create( Material::Create( skyboxShader ) );
		m_SkyboxMaterial->SetFlag( MaterialFlag::DepthTest, false );

		m_ReactPhysicsScene = Ref<PhysicsScene>::Create( this );
		m_PhysicsWorld = Ref<PhysicsWorld>::Create( this );

		m_Registry.on_construct<RigidbodyComponent>().connect<&Scene::PhysicsComponentCreate>( this );
	}

	Scene::~Scene( void )
	{
		//s_ActiveScenes.erase(m_SceneID);
		m_Registry.clear();
	}

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	void Scene::OnUpdate(Timestep ts)
	{
		m_ReactPhysicsScene->Update(ts);

		if (m_RuntimeRunning)
		{
			UpdateRuntime(ts);
		}

	}

	void Scene::OnRenderEditor(Timestep ts, const EditorCamera& editorCamera)
	{

		/////////////////////////////////////////////////////////////////////
		// RENDER 3D SCENE
		/////////////////////////////////////////////////////////////////////
		m_SkyboxMaterial->Set("u_TextureLod", m_SkyboxLod);

		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		SceneRenderer::BeginScene(this, { editorCamera, editorCamera.GetViewMatrix() });
		for (auto entity : group)
		{
			auto& [meshComponent, transformComponent] = group.get<MeshComponent, TransformComponent>(entity);
			if (meshComponent.Mesh)
			{
				meshComponent.Mesh->OnUpdate(ts);

				// TODO: Should we render (logically)

				if (m_SelectedEntity == entity)
					SceneRenderer::SubmitSelectedMesh(meshComponent, transformComponent.GetTransform());
				else
					SceneRenderer::SubmitMesh(meshComponent, transformComponent.GetTransform());
			}
		}

		SceneRenderer::EndScene();

	}

	void Scene::OnRenderRuntime( Timestep ts, const SceneCamera& sceneCamera )
	{
		/////////////////////////////////////////////////////////////////////
		// RENDER 3D SCENE
		/////////////////////////////////////////////////////////////////////
		m_SkyboxMaterial->Set( "u_TextureLod", m_SkyboxLod );

		auto group = m_Registry.group<MeshComponent>( entt::get<TransformComponent> );
		SceneRenderer::BeginScene( this, { sceneCamera, sceneCamera.GetViewMatrix() } );
		for( auto entity : group )
		{
			auto& [meshComponent, transformComponent] = group.get<MeshComponent, TransformComponent>( entity );
			if( meshComponent.Mesh )
			{
				meshComponent.Mesh->OnUpdate( ts );

				// TODO: Should we render (logically)

				if( m_SelectedEntity == entity )
					SceneRenderer::SubmitSelectedMesh( meshComponent, transformComponent.GetTransform() );
				else
					SceneRenderer::SubmitMesh( meshComponent, transformComponent.GetTransform() );
			}
		}

		SceneRenderer::EndScene();
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		SAT_PROFILE_FUNCTION();

		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned Entity" : name;

		auto& ID = entity.AddComponent<IdComponent>();

		return entity;
	}

	ScriptableEntity Scene::CreateScriptableEntity( const std::string& name )
	{
		SAT_PROFILE_FUNCTION();

		ScriptableEntity entity;
		entity.m_Entity = CreateEntity( name );
		entity.m_Scene = this;
		entity.m_Entity.AddComponent<NativeScriptComponent>();
		auto& ncs = entity.m_Entity.GetComponent<NativeScriptComponent>();

		return entity;
	}

	ScriptableEntity* Scene::CreateScriptableEntityptr( const std::string& name )
	{
		SAT_PROFILE_FUNCTION();

		ScriptableEntity* entity = new ScriptableEntity();
		entity->m_Entity = CreateEntity( name );
		entity->m_Scene = this;
		entity->AddComponent<NativeScriptComponent>();
		auto& ncs = entity->GetComponent<NativeScriptComponent>();
		ncs.Instance = entity;
		if( ncs.Instance == nullptr )
		{
			SAT_CORE_ERROR( "NativeScriptComponent.Instance null" );
			SAT_CORE_INFO( "Retrying!" );

			ncs.Instance = entity;

			SAT_CORE_ASSERT( ncs.Instance != nullptr, "NativeScriptComponent.Instance null" );
		}
		ncs.Instance->OnCreate();
	

		m_ScriptableEntitys.push_back(entity);

		return entity;
	}

	Entity Scene::CreateEntityWithID( UUID uuid, const std::string& name, bool runtimeMap )
	{
		SAT_PROFILE_FUNCTION();

		auto entity = Entity{ m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<IdComponent>();
		idComponent.ID = uuid;

		entity.AddComponent<TransformComponent>();
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);

		//SAT_CORE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end());
		m_EntityIDMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		SAT_PROFILE_FUNCTION();

		m_Registry.destroy(entity.m_EntityHandle);
	}

	void Scene::SetViewportSize(uint32_t width, uint32_t height)
	{
		SAT_PROFILE_FUNCTION();

		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	void Scene::SetEnvironment(const Environment& environment)
	{
		m_Environment = environment;
		SetSkybox(environment.RadianceMap);
	}

	void Scene::SetSkybox(const Ref<TextureCube>& skybox)
	{
		m_SkyboxTexture = skybox;
		m_SkyboxMaterial->Set("u_Texture", skybox);
	}

	Entity Scene::GetMainCameraEntity( void )
	{
		//todo: add
		return {};
	}

	Environment Environment::Load(const std::string& filepath)
	{
		SAT_PROFILE_FUNCTION();

		auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
		return { filepath, radiance, irradiance };
	}

	void Scene::PhysicsUpdate( float delta )
	{
		SAT_PROFILE_FUNCTION();

		m_PhysicsWorld->Step( delta );

		auto view = GetRegistry().view<TransformComponent, RigidbodyComponent>();

		for( const auto& entity : view )
		{
			auto [tc, rb] = view.get<TransformComponent, RigidbodyComponent>( entity );

			//tc.Position = pc.Position;

			if( rb.isKinematic )
			{
				rb.m_body->SetMass( 0.0f );
			}

			tc.Position = rb.m_body->GetPosition();

			SAT_CORE_INFO( "tc.Position.y {0}, tc.Position.x {1} ", tc.Position.y, tc.Position.x );
		}
	}

	void Scene::ContactStay(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other) 
	{
	}

	void Scene::ContactEnter(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other) 
	{
	}

	void Scene::ContactExit(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other) 
	{
	}

	void Scene::Contact( rp3d::CollisionBody* body )
	{
	}

	/*------------------------ Runtime helpers ------------------------ */

	template<typename T>
	static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto components = srcRegistry.view<T>();
		for (auto srcEntity : components)
		{
			entt::entity destEntity = enttMap.at(srcRegistry.get<IdComponent>(srcEntity).ID);

			auto& srcComponent = srcRegistry.get<T>(srcEntity);
			auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
		}
	}

	template<typename T>
	static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
	{
		if (registry.has<T>(src))
		{
			auto& srcComponent = registry.get<T>(src);
			registry.emplace_or_replace<T>(dst, srcComponent);
		}
	}

	/**
	* Copies the scene and returns the new scene
	*/
	Ref<Scene> Scene::CopyScene(const Ref<Scene>& CurrentScene)
	{
		SAT_PROFILE_FUNCTION();

		SAT_CORE_WARN("Copying Scene!");
		Ref<Scene> CScene = Ref<Scene>::Create();
		CScene->m_CurrentLevel = CurrentScene->m_CurrentLevel;
		CScene->m_data = CurrentScene->m_data;
		CScene->m_DebugName = CurrentScene->m_DebugName;
		CScene->m_EntityIDMap = CurrentScene->m_EntityIDMap;
		CScene->m_Environment = CurrentScene->m_Environment;
		CScene->m_Light = CurrentScene->m_Light;
		CScene->m_LightMultiplier = CurrentScene->m_LightMultiplier;
		CScene->m_SkyboxLod = CurrentScene->m_SkyboxLod;
		CScene->m_SkyboxMaterial = CurrentScene->m_SkyboxMaterial;
		CScene->m_SkyboxTexture = CurrentScene->m_SkyboxTexture;
		CScene->m_ViewportHeight = CurrentScene->m_ViewportHeight;
		CScene->m_ViewportWidth = CurrentScene->m_ViewportWidth;
		CScene->m_ReactPhysicsScene = CurrentScene->m_ReactPhysicsScene;
		CScene->m_RuntimeData = CurrentScene->m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = CScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(CScene->m_Registry, m_Registry, enttMap);

		return CScene;
	}

	/**
	* Copies the scene and returns the new scene
	*/
	Scene* Scene::CopyScene(const Scene*& CurrentScene)
	{
		SAT_CORE_WARN("Copying Scene!");
		Scene* CScene = new Scene();
		CScene->m_CurrentLevel = CurrentScene->m_CurrentLevel;
		CScene->m_data = CurrentScene->m_data;
		CScene->m_DebugName = CurrentScene->m_DebugName;
		CScene->m_EntityIDMap = CurrentScene->m_EntityIDMap;
		CScene->m_Environment = CurrentScene->m_Environment;
		CScene->m_Light = CurrentScene->m_Light;
		CScene->m_LightMultiplier = CurrentScene->m_LightMultiplier;
		CScene->m_SkyboxLod = CurrentScene->m_SkyboxLod;
		CScene->m_SkyboxMaterial = CurrentScene->m_SkyboxMaterial;
		CScene->m_SkyboxTexture = CurrentScene->m_SkyboxTexture;
		CScene->m_ViewportHeight = CurrentScene->m_ViewportHeight;
		CScene->m_ViewportWidth = CurrentScene->m_ViewportWidth;
		CScene->m_ReactPhysicsScene = CurrentScene->m_ReactPhysicsScene;
		CScene->m_RuntimeData = CurrentScene->m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = CScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(CScene->m_Registry, m_Registry, enttMap);

		return CScene;
	}

	/**
	* Copies the scene and returns the new scene, and fills the 'NewScece' value
	*/
	Ref<Scene> Scene::CopyScene(const Ref<Scene>& CurrentScene, Ref<Scene> NewScene)
	{
		SAT_CORE_WARN("Copying Scene!");
		NewScene->m_CurrentLevel = CurrentScene->m_CurrentLevel;
		NewScene->m_data = CurrentScene->m_data;
		NewScene->m_DebugName = CurrentScene->m_DebugName;
		NewScene->m_EntityIDMap = CurrentScene->m_EntityIDMap;
		NewScene->m_Environment = CurrentScene->m_Environment;
		NewScene->m_Light = CurrentScene->m_Light;
		NewScene->m_LightMultiplier = CurrentScene->m_LightMultiplier;
		NewScene->m_SkyboxLod = CurrentScene->m_SkyboxLod;
		NewScene->m_SkyboxMaterial = CurrentScene->m_SkyboxMaterial;
		NewScene->m_SkyboxTexture = CurrentScene->m_SkyboxTexture;
		NewScene->m_ViewportHeight = CurrentScene->m_ViewportHeight;
		NewScene->m_ViewportWidth = CurrentScene->m_ViewportWidth;
		NewScene->m_ReactPhysicsScene = CurrentScene->m_ReactPhysicsScene;
		NewScene->m_RuntimeData = CurrentScene->m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = NewScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(NewScene->m_Registry, m_Registry, enttMap);

		return NewScene;
	}

	/**
	* Copies the scene and returns the new scene, and fills the 'NewScece' value
	*/
	Scene* Scene::CopyScene(const Scene*& CurrentScene, Scene* NewScene)
	{
		SAT_CORE_WARN("Copying Scene!");
		NewScene->m_CurrentLevel = CurrentScene->m_CurrentLevel;
		NewScene->m_data = CurrentScene->m_data;
		NewScene->m_DebugName = CurrentScene->m_DebugName;
		NewScene->m_EntityIDMap = CurrentScene->m_EntityIDMap;
		NewScene->m_Environment = CurrentScene->m_Environment;
		NewScene->m_Light = CurrentScene->m_Light;
		NewScene->m_LightMultiplier = CurrentScene->m_LightMultiplier;
		NewScene->m_SkyboxLod = CurrentScene->m_SkyboxLod;
		NewScene->m_SkyboxMaterial = CurrentScene->m_SkyboxMaterial;
		NewScene->m_SkyboxTexture = CurrentScene->m_SkyboxTexture;
		NewScene->m_ViewportHeight = CurrentScene->m_ViewportHeight;
		NewScene->m_ViewportWidth = CurrentScene->m_ViewportWidth;
		NewScene->m_ReactPhysicsScene = CurrentScene->m_ReactPhysicsScene;
		NewScene->m_RuntimeData = CurrentScene->m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = NewScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(NewScene->m_Registry, m_Registry, enttMap);

		return NewScene;
	}

	/**
	* Copies the scene and fills the 'NewScece' value
	*/
	void Scene::CopyScene(Ref<Scene>& NewScene)
	{
		SAT_PROFILE_FUNCTION();

		SAT_CORE_WARN("Copying Scene!");
		NewScene->m_CurrentLevel = m_CurrentLevel;
		NewScene->m_data = m_data;
		NewScene->m_DebugName = m_DebugName;
		NewScene->m_EntityIDMap = m_EntityIDMap;
		NewScene->m_Environment = m_Environment;
		NewScene->m_Light = m_Light;
		NewScene->m_LightMultiplier = m_LightMultiplier;
		NewScene->m_SkyboxLod = m_SkyboxLod;
		NewScene->m_SkyboxMaterial = m_SkyboxMaterial;
		NewScene->m_SkyboxTexture = m_SkyboxTexture;
		NewScene->m_ViewportHeight = m_ViewportHeight;
		NewScene->m_ViewportWidth = m_ViewportWidth;
		NewScene->m_ReactPhysicsScene = m_ReactPhysicsScene;
		NewScene->m_RuntimeData = m_RuntimeData;
		NewScene->m_ReactPhysicsScene->RegLog();

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for( auto entity : idComponents )
		{
			auto uuid = m_Registry.get<IdComponent>( entity ).ID;
			Entity e = NewScene->CreateEntityWithID( uuid, "", true );
			enttMap[ uuid ] = e.m_EntityHandle;
		}


		CopyComponent<TagComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<TransformComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<MeshComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<RelationshipComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<PhysicsComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<BoxColliderComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<SphereColliderComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<SpriteRendererComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<NativeScriptComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<RigidbodyComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<CameraComponent>( NewScene->m_Registry, m_Registry, enttMap );

		NewScene->m_ScriptableEntitys = m_ScriptableEntitys;

	}

	void Scene::BeginRuntime( void )
	{
		SAT_CORE_WARN("[Runtime] Begining!");
		StartRuntime();
	}

	void Scene::StartRuntime( void )
	{
		SAT_CORE_WARN("[Runtime] Starting!");
		m_RuntimeRunning = true;

		for( auto entity : m_ScriptableEntitys )
		{
			auto ncs = entity->GetComponent<NativeScriptComponent>();
		
			ncs.Instance->BeginPlay();

		}
	}

	void Scene::EndRuntime( void )
	{
		m_RuntimeRunning = false;

		for( auto entity : m_ScriptableEntitys )
		{
			auto ncs = entity->GetComponent<NativeScriptComponent>();

			ncs.Instance->OnDestroy();
		}
	}

	void Scene::ResetRuntime( const Ref<Scene>& EditorScene )
	{
		
	}

	void Scene::UpdateRuntime( Timestep ts )
	{
		SAT_PROFILE_FUNCTION();

		for( auto entity : m_ScriptableEntitys )
		{

			if (!entity->HasComponent<NativeScriptComponent>())
			{
				entity->AddComponent<NativeScriptComponent>();
				entity->GetComponent<NativeScriptComponent>().Instance = entity;


				entity->GetComponent<NativeScriptComponent>().Instance->OnCreate();
				entity->GetComponent<NativeScriptComponent>().Instance->BeginPlay();

			}

			auto ncs = entity->GetComponent<NativeScriptComponent>();

			ncs.Instance->OnUpdate(ts);

		}

		Application::Get().GetHotReload()->OnHotReload();
	}


}