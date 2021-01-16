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

#pragma once

#include "Saturn/Core/UUID.h"
#include "Saturn/Core/Timestep.h"

#include "Saturn/Renderer/Camera.h"
#include "Saturn/Renderer/Texture.h"
#include "Saturn/Renderer/Material.h"

#include "Saturn/Physics/PhysicsScene.h"
#include "Saturn/Physics/PhysX/PhysXScene.h"

#include "entt.hpp"

#include "Saturn/Scene/SceneCamera.h"
#include "Saturn/Editor/EditorCamera.h"

namespace Saturn {

	class PhysicsWorld;

	struct SceneData {
		UUID SceneID;
		std::string name;
		RefSR<Texture2D> m_SkyboxTexture;
		RefSR<Material> m_SkyboxMaterial;
	};

	struct Environment
	{
		std::string FilePath;
		Ref<TextureCube> RadianceMap;
		Ref<TextureCube> IrradianceMap;

		static Environment Load(const std::string& filepath);
	};

	struct Light
	{
		glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Radiance = { 0.0f, -1.0f, 0.0f };

		float Multiplier = 1.0f;
	};

	struct RuntimeData
	{
		bool Running = false;
		Ref<Scene> RuntimeScene;
		UUID RuntimeID;
	};

	enum class PhysicsType
	{
		None = 0,
		PhysX = 1,
		ReactPhysics = 2
	};

	class Level;
	class Entity;
	class ScriptableEntity;

	using EntityMap = std::unordered_map<UUID, Entity>;

	class Scene : public RefCounted
	{
	public:
		Scene( void );
		~Scene( void );

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithID(UUID uuid, const std::string& name = "", bool runtimeMap = false);
		ScriptableEntity CreateScriptableEntity( const std::string& name = "" );
		ScriptableEntity* CreateScriptableEntityptr( const std::string& name );

		//FOR EDITOR USE!
		// Use SpawnEntity for runtime!
		template<typename T>
		T* CreateScriptableEntityT( const std::string& name )
		{
			SAT_PROFILE_FUNCTION();

			T* entity = new T();
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


			m_ScriptableEntitys.push_back( entity );

			return entity;
		}

		//Spawns a T
		template<typename T>
		T* SpawnEntity( std::string name, glm::vec3 position, glm::quat rotation, bool addncs = true )
		{
			SAT_PROFILE_FUNCTION();

			T* entity = new T();
			entity->m_Entity = CreateEntity( name );
			entity->m_Scene = this;
			entity->GetComponent<TransformComponent>().Position = position;
			entity->GetComponent<TransformComponent>().Rotation = rotation;
			if (addncs)
			{
				entity->AddComponent<NativeScriptComponent>();
				auto& ncs = entity->GetComponent<NativeScriptComponent>();
				ncs.Instance = entity;

				if( ncs.Instance == nullptr )
				{
					SAT_ERROR( "[Runtime Context] NativeScriptComponent.Instance null" );
					SAT_INFO( "[Runtime Context] Retrying!" );

					ncs.Instance = entity;

					SAT_ASSERT( ncs.Instance != nullptr, "[Runtime Context] NativeScriptComponent.Instance null" );
				}

				ncs.Instance->OnCreate();

			}

			m_ScriptableEntitys.push_back( entity );

			return entity;
		}


		void DestroyEntity(Entity entity);

		void OnRenderEditor( Timestep ts, const EditorCamera& editorCamera );
		void OnRenderRuntime( Timestep ts, const SceneCamera& sceneCamera );

		template<typename T>
		auto GetAllEntitiesWith( void )
		{
			return m_Registry.view<T>();
		}

		PhysicsScene* GetPhysicsScene() {
			return &*m_ReactPhysicsScene;
		}

		SceneData& GetData() { return m_data; }
		Level& GetLevel() { return *m_CurrentLevel; }
		entt::registry& GetRegistry() { return m_Registry; }

		void OnUpdate(Timestep ts);
		void SetViewportSize(uint32_t width, uint32_t height);

		void SetEnvironment(const Environment& environment);
		const Environment& GetEnvironment() const { return m_Environment; }
		void SetSkybox(const Ref<TextureCube>& skybox);

		Light& GetLight() { return m_Light; }
		const Light& GetLight() const { return m_Light; }

		Entity GetMainCameraEntity( void );

		float& GetSkyboxLod() { return m_SkyboxLod; }

		void DuplicateEntity(Entity entity);

		// Editor-specific
		void SetSelectedEntity(entt::entity entity) { m_SelectedEntity = entity; }

		//phys
		void PhysicsUpdate( PhysicsType type, float delta );
		void ContactStay(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);
		void ContactEnter(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);
		void ContactExit(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);

		void PhysicsComponentCreate(entt::registry& r, entt::entity ent);

		void PhysXRigidbodyComponentCreate( entt::registry& r, entt::entity ent );
		void PhysXBoxComponentCreate( entt::registry& r, entt::entity ent );

		void Contact(rp3d::CollisionBody* body);

		const EntityMap& GetEntityMap() const { return m_EntityIDMap; }

		/*------------------------ Runtime helpers ------------------------ */
		Ref<Scene> CopyScene( const Ref<Scene>& CurrentScene );
		Ref<Scene> CopyScene( const Ref<Scene>& CurrentScene, Ref<Scene> NewScene );
		Scene* CopyScene( const Scene*& CurrentScene );
		Scene* CopyScene( const Scene*& CurrentScene, Scene* NewScene );
		void CopyScene( Ref<Scene>& NewScene );

		void BeginRuntime( void );
		void StartRuntime( void );
		void EndRuntime( void );
		void ResetRuntime( const Ref<Scene>& EditorScene );
		RuntimeData& GetRuntimeData() { return m_RuntimeData; }
		bool m_RuntimeRunning = false;

	private:
		void UpdateRuntime( Timestep ts );
		std::vector<ScriptableEntity*> m_ScriptableEntitys;
		/*------------------------------------------------------------------ */
	public:
		Ref<PhysicsScene> m_ReactPhysicsScene;
		Ref<PhysXScene> m_PhysXScene;


	private:
		UUID m_SceneID;
		entt::entity m_SceneEntity;
		entt::registry m_Registry;

		EntityMap m_EntityIDMap;

		std::string m_DebugName;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		Environment m_Environment;
		Ref<TextureCube> m_SkyboxTexture;
		Ref<MaterialInstance> m_SkyboxMaterial;

		Ref<PhysicsWorld> m_PhysicsWorld;

		entt::entity m_SelectedEntity;

		float m_SkyboxLod = 1.0f;

		Light m_Light;
		float m_LightMultiplier = 0.3f;

		SceneData m_data;

		Level* m_CurrentLevel;

		RuntimeData m_RuntimeData;

		friend class Entity;
		friend class Serialiser;
		friend class SceneRenderer;
		friend class ScriptableEntity;
		friend class SceneHierarchyPanel;

	};
}