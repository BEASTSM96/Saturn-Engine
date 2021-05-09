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
#include "ScriptableEntity.h"
#include "Saturn/Physics/PhysX/PhysXRuntime.h"

#include "Saturn/Script/ScriptEngine.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	static const std::string DefaultEntityName = "Entity";

	std::unordered_map<UUID, Scene*> s_ActiveScenes;


	void Scene::PhysicsComponentCreate( entt::registry& r, entt::entity ent )
	{
		
	}

	void Scene::PhysXRigidbodyComponentCreate( entt::registry& r, entt::entity ent )
	{
	}

	void Scene::PhysXBoxComponentCreate( entt::registry& r, entt::entity ent )
	{
	}

	void Scene::PhysXBoxSphereComponentCreate( entt::registry& r, entt::entity ent )
	{
	}

	void Scene::CameraComponentCreate( entt::registry& r, entt::entity ent )
	{
		if( !r.has<TransformComponent>( ent ) )
		{
			SAT_CORE_ERROR( "CameraComponent needs a TransformComponent!" );
			return;
		}

		auto& cameraComponent = r.get<CameraComponent>( ent );
		cameraComponent.Camera = Ref<SceneCamera>::Create();
		cameraComponent.Camera->SetProjectionMatrix( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 10000.0f ) );
		cameraComponent.Camera->SetPosition( r.get<TransformComponent>( ent ).Position );
	}

	void Scene::PhysXCapsuleColliderComponentCreate( entt::registry& r, entt::entity ent )
	{
		if( !r.has<TransformComponent>( ent ) )
		{
			SAT_CORE_ERROR( "PhysXCapsuleColliderComponent needs a TransformComponent!" );
			return;
		}

		Entity e( ent, this );
	}

	void Scene::ScriptComponentCreate( entt::registry& r, entt::entity ent )
	{
		auto view = r.view<SceneComponent>();
		UUID sceneId = r.get<SceneComponent>( view.front() ).SceneID;
		Scene* scene = s_ActiveScenes[ sceneId ];

		auto enttId = r.get<IdComponent>( ent ).ID;
		SAT_CORE_ASSERT( scene->m_EntityIDMap.find( enttId ) != scene->m_EntityIDMap.end() );
		ScriptEngine::OnInitEntity( scene->m_EntityIDMap.at(enttId) );
	}

	const EntityMap& Scene::GetEntityMap() const
	{
		return m_EntityIDMap;
	}

	void Scene::DuplicateEntity( Entity entity )
	{
	}

	void Scene::CreatePhysxScene()
	{
		//m_PhysXScene = Ref<PhysXScene>::Create( this );
	}

	Scene::Scene( void )
	{
		SAT_PROFILE_FUNCTION();

		s_ActiveScenes[ m_SceneID ] = this;
		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>( m_SceneEntity, m_SceneID );

		auto skyboxShader = Shader::Create( "assets/shaders/Skybox.glsl" );
		m_SkyboxMaterial = MaterialInstance::Create( Material::Create( skyboxShader ) );
		m_SkyboxMaterial->SetFlag( MaterialFlag::DepthTest, false );

		m_Registry.on_construct<PhysXRigidbodyComponent>().connect<&Scene::PhysXRigidbodyComponentCreate>( this );
		m_Registry.on_construct<PhysXBoxColliderComponent>().connect<&Scene::PhysXBoxComponentCreate>( this );
		m_Registry.on_construct<PhysXSphereColliderComponent>().connect<&Scene::PhysXBoxSphereComponentCreate>( this );
		m_Registry.on_construct<PhysXCapsuleColliderComponent>().connect<&Scene::PhysXCapsuleColliderComponentCreate>( this );
		m_Registry.on_construct<CameraComponent>().connect<&Scene::CameraComponentCreate>( this );
		m_Registry.on_construct<ScriptComponent>().connect<&Scene::ScriptComponentCreate>( this );
	}

	Scene::~Scene( void )
	{
		s_ActiveScenes.erase(m_SceneID);

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

	Entity Scene::CreateEntity( const std::string& name )
	{
		SAT_PROFILE_FUNCTION();

		Entity entity ={ m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned Entity" : name;

		auto& IDcomponent = entity.AddComponent<IdComponent>();
		entity.GetComponent<IdComponent>().ID ={};

		m_EntityIDMap[ IDcomponent.ID ] = entity;
		return entity;
	}

	Entity Scene::CreateEntityWithID( UUID uuid, const std::string& name, bool runtimeMap )
	{
		SAT_PROFILE_FUNCTION();

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

	void Scene::DestroyEntity( Entity entity )
	{
		SAT_PROFILE_FUNCTION();

		m_Registry.destroy( entity.m_EntityHandle );
	}

	void Scene::SetViewportSize( uint32_t width, uint32_t height )
	{
		SAT_PROFILE_FUNCTION();

		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	void Scene::SetEnvironment( const Environment& environment )
	{
		m_Environment = environment;
		SetSkybox( environment.RadianceMap );
	}

	void Scene::SetSkybox( const Ref<TextureCube>& skybox )
	{
		m_SkyboxTexture = skybox;
		m_SkyboxMaterial->Set( "u_Texture", skybox );
	}

	Entity Scene::GetMainCameraEntity( void )
	{
		auto view = m_Registry.view<TransformComponent, CameraComponent>();
		for( auto entity : view )
		{
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>( entity );

			if( camera.Primary )
			{
				return { entity, this };
			}
		}
		return {};
	}

	Environment Environment::Load(const std::string& filepath)
	{
		SAT_PROFILE_FUNCTION();

		auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
		return { filepath, radiance, irradiance };
	}


	void Scene::PhysicsUpdate( PhysicsType type, float delta )
	{
		SAT_PROFILE_FUNCTION();

		auto PhysXView = m_Registry.view<TransformComponent, PhysXRigidbodyComponent>();

		switch( type )
		{
			case Saturn::PhysicsType::None:
				break;
			case Saturn::PhysicsType::PhysX:
				for( const auto& entity : PhysXView )
				{
					auto [tc, rb] = PhysXView.get<TransformComponent, PhysXRigidbodyComponent>( entity );

					if( rb.isKinematic )
					{
						//rb.m_Rigidbody->GetPxBody().setMass( 0.0f );
					}

					tc.Position = rb.m_Rigidbody->GetPos();
					rb.m_Rigidbody->GetPos() = tc.Position;
					
				}
				break;
			default:
				break;
		}
	}

	Entity& Scene::CreateEntityFromPhysXData( void* data )
	{
		Entity& a = *( Entity* )data;
		Entity e = CreateEntity( "" );
		e.m_EntityHandle = a.m_EntityHandle;
		return e;
	}

	/*------------------------ Runtime helpers ------------------------ */

	template<typename T>
	static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto components = srcRegistry.view<T>();
		for (auto srcEntity : components)
		{
			if ( !srcRegistry.has<SceneComponent>( srcEntity ))
			{
				SAT_CORE_INFO( "{0}", srcRegistry.get<IdComponent>( srcEntity ).ID );
				entt::entity destEntity = enttMap.at( srcRegistry.get<IdComponent>( srcEntity ).ID );

				auto& srcComponent = srcRegistry.get<T>( srcEntity );
				auto& destComponent = dstRegistry.emplace_or_replace<T>( destEntity, srcComponent );
			}
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
	* Copies the scene and fills the 'NewScene' value
	*/
	void Scene::CopyScene(Ref<Scene>& NewScene)
	{
		SAT_PROFILE_FUNCTION();

		NewScene->m_data = m_data;
		NewScene->m_DebugName = m_DebugName;
		NewScene->m_EntityIDMap = m_EntityIDMap;
		NewScene->m_EntityMonoIDMap = m_EntityMonoIDMap;
		NewScene->m_Environment = m_Environment;
		NewScene->m_Light = m_Light;
		NewScene->m_LightMultiplier = m_LightMultiplier;
		NewScene->m_SkyboxLod = m_SkyboxLod;
		NewScene->m_SkyboxMaterial = m_SkyboxMaterial;
		NewScene->m_SkyboxTexture = m_SkyboxTexture;
		NewScene->m_ViewportHeight = m_ViewportHeight;
		NewScene->m_ViewportWidth = m_ViewportWidth;
		NewScene->m_RuntimeData = m_RuntimeData;

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
		CopyComponent<SpriteRendererComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<PhysXRigidbodyComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<PhysXBoxColliderComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<PhysXSphereColliderComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<PhysXCapsuleColliderComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<CameraComponent>( NewScene->m_Registry, m_Registry, enttMap );
		CopyComponent<ScriptComponent>( NewScene->m_Registry, m_Registry, enttMap );
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

		m_PhysXRuntime = new PhysXRuntime();
		m_PhysXRuntime->CreateScene();

		auto view = m_Registry.view<ScriptComponent>();
		for( auto entt : view )
		{
			Entity e ={ entt, this };
			ScriptEngine::OnCreateEntity( e );
			ScriptEngine::OnEntityBeginPlay( e );
		}

		{
			auto view = m_Registry.view<PhysXRigidbodyComponent>();
			for( auto entity : view )
			{
				Entity e ={ entity, this };
				PhysXRuntime::CreatePhysXCompsForEntity( e );
				auto& rb = e.GetComponent<PhysXRigidbodyComponent>();
				rb.m_Rigidbody->AddActorToScene();
			}
		}
	}

	void Scene::EndRuntime( void )
	{
		m_RuntimeRunning = false;
		m_PhysXRuntime->Clear();
		delete m_PhysXRuntime;
		m_PhysXRuntime = nullptr;
	}

	void Scene::ResetRuntime( const Ref<Scene>& EditorScene )
	{
		
	}

	void Scene::UpdateRuntime( Timestep ts )
	{
		SAT_PROFILE_FUNCTION();

		auto view = m_Registry.view<ScriptComponent>();
		for (auto entt : view) 
		{
			Entity e ={ entt, this };
			ScriptEngine::OnUpdateEntity( e, ts );
		}

		{
			auto view = m_Registry.view<PhysXRigidbodyComponent>();
			for( auto entity : view )
			{
				Entity e ={ entity, this };
				auto& rb = e.GetComponent<PhysXRigidbodyComponent>();
				rb.m_Rigidbody->SetUserData( e );
			}
		}

		m_PhysXRuntime->Update( ts, *this );
	}
}