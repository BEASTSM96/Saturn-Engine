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
#include "Scene.h"

#include "Entity.h"
#include "Components.h"

#include "Saturn/Vulkan/SceneRenderer.h"
#include "Saturn/Vulkan/Renderer2D.h"
#include "Saturn/Vulkan/VulkanContext.h"

#include "Saturn/Asset/Prefab.h"
#include "Saturn/Asset/AssetManager.h"

#include "Saturn/Core/OptickProfiler.h"
#include "Saturn/Core/VirtualFS.h"
#include "Saturn/Core/MemoryStream.h"
#include "Saturn/Core/Renderer/SceneFlyCamera.h"

#include "Saturn/Physics/PhysicsScene.h"
#include "Saturn/Physics/PhysicsRigidBody.h"

#include "Saturn/GameFramework/Core/GameModule.h"

#include "Saturn/Serialisation/SceneSerialiser.h"

#include "Saturn/Audio/AudioSystem.h"

#include "Saturn/ImGui/EditorIcons.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

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
		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>( m_SceneEntity, m_InternalID );
	}

	Scene::~Scene()
	{
		Empty();
	}

	void Scene::Empty()
	{
		ClearSelectedEntities();

		{
			auto staticMeshes = GetAllEntitiesWith<StaticMeshComponent>();

			for( auto& entity : staticMeshes )
			{
				auto& rMeshComponent = entity->GetComponent<StaticMeshComponent>();

				if( rMeshComponent.Mesh )
					rMeshComponent.Mesh = nullptr;

				if( rMeshComponent.MaterialRegistry )
					rMeshComponent.MaterialRegistry = nullptr;
			}

			// TODO: Is really needed? As the physics scene will destroy all of this.

			auto rigidBodies = GetAllEntitiesWith<RigidbodyComponent>();

			for( auto& entity : rigidBodies )
			{
				if( entity->GetComponent<RigidbodyComponent>().Rigidbody ) 
				{
					delete entity->GetComponent<RigidbodyComponent>().Rigidbody;
					entity->GetComponent<RigidbodyComponent>().Rigidbody = nullptr;
				}
			}

			StopAudioPlayers();
		}

		// Destroy all entities.
		for( auto&& [id, entity] : m_EntityIDMap )
		{
			entity = nullptr;
		}

		m_EntityIDMap.clear();
		m_Registry.clear();
	}

	// TODO: We don't want to search for the main camera entity every frame.
	Ref<Entity> Scene::GetMainCameraEntity( bool force )
	{
		if( m_MainCameraEntity && !force )
			return m_MainCameraEntity;

		auto entities = GetAllEntitiesWith<CameraComponent>();

		for( auto& entity : entities )
		{
			if( entity->GetComponent<CameraComponent>().MainCamera )
			{
				m_MainCameraEntity = entity;
				return m_MainCameraEntity;
			}
		}

		return nullptr;
	}

	void Scene::AddSelectedEntity( Ref<Entity> entity )
	{
		m_SelectedEntities.push_back( entity );
	}

	void Scene::DeselectEntity( Ref<Entity> entity )
	{
		m_SelectedEntities.erase( std::remove( m_SelectedEntities.begin(), m_SelectedEntities.end(), entity ), m_SelectedEntities.end() );
	}

	void Scene::ClearSelectedEntities()
	{
		m_SelectedEntities.clear();
	}

	void Scene::OnUpdate( Timestep ts )
	{
		SAT_PF_EVENT();

		// Update Cycle.
		// Step 1: Update and simulate the physics scene.
		// Step 2: Update all entities.

		// TODO: We might want to change the order of this update cycle.
		if( RuntimeRunning ) 
		{
			// Simulate the physics scene.
			m_PhysicsScene->Update( ts );
			OnUpdatePhysics( ts );

			for( auto&& [id, entity] : m_EntityIDMap )
			{
				entity->OnUpdate( ts );
			}

			UpdateAudioListeners();
		}
	}
	
	void Scene::OnUpdatePhysics( Timestep ts )
	{
		SAT_PF_EVENT();

		auto rigidBodies = GetAllEntitiesWith<RigidbodyComponent>();
		
		constexpr float FixedTimestep = 1.0f / 100.0f;
		for( auto&& [id, entity] : m_EntityIDMap )
		{
			entity->OnPhysicsUpdate( FixedTimestep );
		}

		for( auto& entity : rigidBodies )
		{
			auto& rb = entity->GetComponent<RigidbodyComponent>();
			rb.Rigidbody->SyncTransfrom();
		}
	}

	void Scene::OnRenderEditor( const EditorCamera& rCamera, Timestep ts, SceneRenderer& rSceneRenderer )
	{
		SAT_PF_EVENT();

		m_RendererCamera.Camera = rCamera;
		m_RendererCamera.ViewMatrix = rCamera.ViewMatrix();

		Renderer2D::Get().SetCamera( m_RendererCamera );
		Renderer2D::Get().PreRender();

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

				if( points.size() )
				{
					uint32_t plIndex = 0;
					Ref<Texture2D> pointLightBillboardTex = EditorIcons::GetIcon( "Billboard_PointLight" );

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

						Renderer2D::Get().SubmitBillboardTextured( pl.Position, glm::vec4( 1.0f ), pointLightBillboardTex, glm::vec2( 1.5f ) );

						plIndex++;
					}
				}
			}
		}

		// Audio Billboards
		{
			auto players = m_Registry.group<AudioPlayerComponent>( entt::get<TransformComponent> );
			if( players.size() )
			{
				Ref<Texture2D> audio = EditorIcons::GetIcon( "Billboard_Audio" );
				Ref<Texture2D> audioMuted = EditorIcons::GetIcon( "Billboard_AudioMuted" );
				Ref<Texture2D> audioLooped = EditorIcons::GetIcon( "Billboard_AudioLooping" );

				for( const auto& e : players )
				{
					auto [transformComponent, playerComponent] = players.get<TransformComponent, AudioPlayerComponent>( e );

					Ref<Texture2D> submissionTexture = audio;

					if( playerComponent.Loop )
						submissionTexture = audioLooped;
					else if( playerComponent.Mute )
						submissionTexture = audioMuted;

					Renderer2D::Get().SubmitBillboardTextured(
						transformComponent.Position,
						glm::vec4( 1.0f ),
						submissionTexture, glm::vec2( 1.0f ) );
				}
			}

			auto listeners = m_Registry.group<AudioListenerComponent>( entt::get<TransformComponent> );
			if( listeners.size() )
			{
				Ref<Texture2D> listenTexture = EditorIcons::GetIcon( "Billboard_AudioListen" );

				for( const auto& e : listeners )
				{
					auto [transformComponent, comp] = listeners.get<TransformComponent, AudioListenerComponent>( e );

					auto pos = glm::vec3( transformComponent.Position.x, transformComponent.Position.y + 2.5f, transformComponent.Position.z );

					Renderer2D::Get().SubmitBillboardTextured(
						pos,
						glm::vec4( 1.0f ),
						listenTexture, glm::vec2( 1.0f ) );

					// Use billboard pos as starting pos
					auto start = pos;
					auto end = start + glm::normalize( comp.Direction ) * 2.0f;

					Renderer2D::Get().SubmitLine( start, end, glm::vec4( 1.0f ) );
				}
			}
		}

		// Physics Colliders (selected meshes only)
		{
			for( auto& rSelectedEntity : m_SelectedEntities )
			{
				if( rSelectedEntity->HasComponent<RigidbodyComponent>() && rSelectedEntity->HasComponent<StaticMeshComponent>() )
				{
					auto& rbComp = rSelectedEntity->GetComponent<RigidbodyComponent>();
					auto& meshComponent = rSelectedEntity->GetComponent<StaticMeshComponent>();
					auto transform = GetTransformRelativeToParent( rSelectedEntity );

					if( meshComponent.Mesh ) 
					{
						Ref<MaterialRegistry> targetMaterialRegistry = meshComponent.Mesh->GetMaterialRegistry();

						if( meshComponent.MaterialRegistry && meshComponent.MaterialRegistry->HasAnyOverrides() )
							targetMaterialRegistry = meshComponent.MaterialRegistry;

						rSceneRenderer.SubmitPhysicsCollider( rSelectedEntity, meshComponent.Mesh, targetMaterialRegistry, transform );
					}
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

					rSceneRenderer.SubmitStaticMesh( entity, meshComponent.Mesh, targetMaterialRegistry, transform );
				}
			}
		}

		rSceneRenderer.SetCamera( m_RendererCamera );
	}

	void Scene::OnRenderRuntime( Timestep ts, SceneRenderer& rSceneRenderer )
	{
		SAT_PF_EVENT();

		// Try find new main camera entity if current saved one is null
		if( !m_MainCameraEntity )
		{
			m_MainCameraEntity = GetMainCameraEntity();
		}

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

				Ref<MaterialRegistry> targetMaterialRegistry = meshComponent.Mesh->GetMaterialRegistry();

				if( meshComponent.MaterialRegistry->HasAnyOverrides() )
					targetMaterialRegistry = meshComponent.MaterialRegistry;

				if( meshComponent.Mesh )
					rSceneRenderer.SubmitStaticMesh( entity, meshComponent.Mesh, targetMaterialRegistry, transform );
			}
		}

		// Check twice because we are always going to have to set the projection
		if( m_MainCameraEntity )
		{
			auto& rCamera = m_MainCameraEntity->GetComponent<CameraComponent>().Camera;
			rCamera.SetViewportSize( rSceneRenderer.Width(), rSceneRenderer.Height() );
			auto view = glm::inverse( GetTransformRelativeToParent( m_MainCameraEntity ) );
			
			m_RendererCamera.Camera = rCamera;
			m_RendererCamera.ViewMatrix = view;
		}
		else
		{
			// TODO:
		}

		rSceneRenderer.SetCamera( m_RendererCamera );
		Renderer2D::Get().SetCamera( m_RendererCamera );
	}

	Ref<Entity> Scene::CreateEntityWithIDScript( UUID uuid, const std::string& name /*= "" */, const std::string& rScriptName )
	{
		Scene* ActiveScene = GActiveScene;
		if( GActiveScene != this )
			GActiveScene = this;

		Ref<Entity> entity = GameModule::Get().CreateEntity( rScriptName );
		entity->SetName( name );
		entity->GetComponent<IdComponent>().ID = uuid;

		GActiveScene = ActiveScene;

		return entity;
	}

	Ref<Entity> Scene::FindEntityByTag( const std::string& tag )
	{
		SAT_PF_EVENT();

		for( auto&& [handle, entity] : m_EntityIDMap )
		{
			if( entity->GetComponent<TagComponent>().Tag == tag )
				return entity;
		}

		return nullptr;
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

		const UUID& rParentID = entity->GetParent();

		if( rParentID != 0 )
		{
			Ref<Entity> parent = FindEntityByID( rParentID );
			if( parent )
				transform = GetTransformRelativeToParent( parent );
		}

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

	bool Scene::Raycast( const glm::vec3& Origin, const glm::vec3& Direction, float MaxDistance, RaycastHitResult* pOut )
	{
		if( m_PhysicsScene )
			return m_PhysicsScene->Raycast( Origin, Direction, MaxDistance, pOut );

		return false;
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

	Ref<Entity> Scene::DuplicateEntity( Ref<Entity> entity, Ref<Entity> parent )
	{
		Ref<Entity> newEntity = Ref<Entity>::Create();
		newEntity->SetName( entity->GetComponent<TagComponent>().Tag );

		// Without TagComponent, IdComponent, RelationshipComponent
		using DesiredComponents = ComponentGroup<TransformComponent, PrefabComponent,
			StaticMeshComponent,
			DirectionalLightComponent, SkylightComponent, PointLightComponent,
			CameraComponent,
			BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, MeshColliderComponent, RigidbodyComponent,
			ScriptComponent,
			AudioPlayerComponent, AudioListenerComponent,
			BillboardComponent>;

		CopyComponentIfExists( DesiredComponents{}, newEntity->GetHandle(), entity->GetHandle(), m_Registry );

		auto& relationshipComponent = newEntity->GetComponent<RelationshipComponent>();
		auto& sourceRelationship = entity->GetComponent<RelationshipComponent>();
		
		relationshipComponent.ChildrenID.resize( entity->GetChildren().size() );
		
		// parent should only be a valid pointer if we are calling this recursively.
		if( parent )
		{
			newEntity->SetParent( parent->GetUUID() );
		}

		if( entity->HasParent() && !parent )
		{
			Ref<Entity> parent = FindEntityByID( entity->GetParent() );
			Ref<Entity> newParent = DuplicateEntity( parent, nullptr );

			newEntity->SetParent( newParent->GetUUID() );
		}

		for( const auto& rID : sourceRelationship.ChildrenID )
		{
			Ref<Entity> child = FindEntityByID( rID );
			Ref<Entity> newChild = DuplicateEntity( child, newEntity );

			newEntity->GetChildren().push_back( newChild->GetUUID() );
		}

		return newEntity;
	}

	void Scene::DeleteEntity( Ref<Entity> entity )
	{
		for( auto& rChild : entity->GetChildren() )
		{
			auto child = FindEntityByID( rChild );

			DeleteEntity( child );
		}

		m_EntityIDMap.erase( entity->GetHandle() );
		m_Registry.destroy( entity->GetHandle() );
	}

	void Scene::CopyScene( Ref<Scene>& NewScene )
	{
		// Copy entities
		// I know we can just use the "=" operator, but we need to recreate the entities from the game.
		for( auto&& [id, entity] : m_EntityIDMap )
		{
			auto& rSourceEntity = m_EntityIDMap[ id ];

			if( rSourceEntity->HasComponent<ScriptComponent>() )
			{
				auto& rScriptComponent = rSourceEntity->GetComponent<ScriptComponent>();

				NewScene->m_EntityIDMap[ id ] = NewScene->CreateEntityWithIDScript( rSourceEntity->GetUUID(), rSourceEntity->GetName(), rScriptComponent.ScriptName );
			}
			else
			{
				NewScene->m_EntityIDMap[ id ] = Ref<Entity>::Create( rSourceEntity->GetName(), rSourceEntity->GetUUID() );
			}	
		}

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

		RuntimeRunning = true;

		m_PhysicsScene = new PhysicsScene( this );

		for( auto&& [id, entity] : m_EntityIDMap )
		{
			entity->BeginPlay();
		}

		StartAudioPlayers();

		// Init new scene camera
		m_MainCameraEntity = GetMainCameraEntity( true );
		auto& rCameraComponent = m_MainCameraEntity->GetComponent<CameraComponent>();
		rCameraComponent.Camera.SetFOV( rCameraComponent.Fov );
	}

	void Scene::UpdateAudioListeners() 
	{
		auto linsteners = GetAllEntitiesWith< AudioListenerComponent >();

		for( auto& entity : linsteners )
		{
			auto& rComp = entity->GetComponent<AudioListenerComponent>();
			
			if( rComp.Primary )
			{
				auto& rTransform = entity->GetComponent<TransformComponent>();
				AudioSystem::Get().SetPrimaryListenerPos( rTransform.Position );
			}
		}
	}

	void Scene::StartAudioPlayers()
	{
		auto sndPlayers = GetAllEntitiesWith< AudioPlayerComponent >();

		for( auto& entity : sndPlayers )
		{
			auto& rComp = entity->GetComponent<AudioPlayerComponent>();
			Ref<Asset> soundSpec = AssetManager::Get().FindAsset( rComp.SpecAssetID );

			if( !soundSpec )
				continue;
			
			if( soundSpec->Type == AssetType::GraphSound ) [[unlikely]]
			{
				AudioSystem::Get().PlayGraphSound( rComp.SpecAssetID, rComp.UniqueID );
			}
			else [[likely]]
			{
				Ref<Sound> sound = nullptr;

				if( rComp.Spatialization )
				{
					sound = AudioSystem::Get().PlaySoundAtLocation( rComp.SpecAssetID, rComp.UniqueID, entity->GetComponent<TransformComponent>().Position );
				}
				else
				{
					sound = AudioSystem::Get().RequestNewSound( rComp.SpecAssetID, rComp.UniqueID );
				}

				if( rComp.Loop )
				{
					sound->WaitUntilLoaded();
					sound->Loop();
				}

				sound->SetVolumeMultiplier( rComp.VolumeMultiplier );
				sound->SetPitchMultiplier( rComp.PitchMultiplier );
			}
		}
	}

	void Scene::StopAudioPlayers() 
	{
		auto sndPlayers = GetAllEntitiesWith< AudioPlayerComponent >();

		for( auto& entity : sndPlayers )
		{
			auto& rComp = entity->GetComponent<AudioPlayerComponent>();
			
			AudioSystem::Get().StopAndResetSound( rComp.UniqueID );
		}
	}

	void Scene::DestroyAudioPlayers()
	{
		auto sndPlayers = GetAllEntitiesWith< AudioPlayerComponent >();

		for( auto& entity : sndPlayers )
		{
			auto& rComp = entity->GetComponent<AudioPlayerComponent>();

			AudioSystem::Get().UnloadSound( rComp.UniqueID );
		}
	}

	void Scene::OnRuntimeEnd()
	{
		if( m_PhysicsScene )
			delete m_PhysicsScene;

		DestroyAudioPlayers();

		m_MainCameraEntity = nullptr;

		RuntimeRunning = false;
	}

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

	//////////////////////////////////////////////////////////////////////////
	// #WARNING This should not be confused with AssetSerialisers. This is for raw binary serialisation!

	void Scene::SerialiseData()
	{
		std::filesystem::path out = Project::GetActiveProject()->GetTempDir();
		out /= std::to_string( ID );
		out.replace_extension( ".vfs" );

		std::ofstream stream( out, std::ios::binary | std::ios::trunc );

		/////////////////////////////////////

		SerialiseInternal( stream );

		stream.close();
	}
	
	void Scene::SerialiseInternal( std::ofstream& rStream )
	{
		Lights::Serialise( m_Lights, rStream );

		// Serialise the map manually.
		size_t mapSize = m_EntityIDMap.size();
		rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

		for( auto& [k, v] : m_EntityIDMap )
		{
			// K (entt::entity) is always trivial
			RawSerialisation::WriteObject( k, rStream );

			bool isScriptClass = v->HasComponent<ScriptComponent>();
			RawSerialisation::WriteObject( isScriptClass, rStream );

			if( isScriptClass )
			{
				std::string name = v->GetComponent<ScriptComponent>().ScriptName;
				RawSerialisation::WriteString( name, rStream );
			}

			// V (Entity) is not trivial
			Entity::Serialise( v, rStream );
		}
	}

	void Scene::DeserialiseData()
	{
		const std::string& rMountBase = Project::GetActiveConfig().Name;
		Ref<VFile> file = VirtualFS::Get().FindFile( rMountBase, Path );

		if( !file )
			return;

		PakFileMemoryBuffer membuf( file->FileContent );

		std::istream stream( &membuf );

		/////////////////////////////////////

		DeserialiseInternal( stream );
	}

	template<typename IStream>
	void Scene::DeserialiseInternal( IStream& rStream )
	{
		Lights::Deserialise( m_Lights, rStream );

		// Read the map manually.
		size_t mapSize = 0;
		rStream.read( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

		// We can not guarantee that we are the active scene, so temporarily set it while loading.
		Scene* ActiveScene = GActiveScene;
		GActiveScene = this;

		for( size_t i = 0; i < mapSize; i++ )
		{
			entt::entity K{};

			// K is always trivial
			RawSerialisation::ReadObject( K, rStream );

			bool isScriptClass = false;
			RawSerialisation::ReadObject( isScriptClass, rStream );

			Ref<Entity> V = nullptr;

			if( isScriptClass )
			{
				std::string className = RawSerialisation::ReadString( rStream );
				V = GameModule::Get().CreateEntity( className );
			}
			else
			{
				V = Ref<Entity>::Create();
			}

			// V is always non-trivial
			Entity::Deserialise( V, rStream );

			m_EntityIDMap[ K ] = V;
		}

		GActiveScene = ActiveScene;
	}

}