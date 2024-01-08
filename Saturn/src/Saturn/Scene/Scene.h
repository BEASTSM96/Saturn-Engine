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

#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Core/Log.h"

#include "SharedGlobals.h"

#include "Saturn/GameFramework/Core/GameScript.h"

#include "Saturn/Core/Renderer/EditorCamera.h"

#include "Saturn/Core/UUID.h"
#include "Saturn/Core/Timestep.h"

#include "entt.hpp"

#include <shared_mutex>

namespace Saturn {

	class Entity;
	class Prefab;
	class PhysicsScene;
	class SClass;
	class SceneRenderer;

	struct TransformComponent;
	struct RaycastHitResult;

	struct SceneComponent
	{
		UUID SceneID;
	};

	struct DirectionalLight
	{
		glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };

		float Intensity = 1.0f;
	};

	struct PointLight
	{
		alignas( 16 ) glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		alignas( 16 ) glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };

		alignas( 4 ) float Multiplier = 1.0f;
		alignas( 4 ) float LightSize = 0.5f;
		alignas( 4 ) float Radius = 10.0f;
		alignas( 4 ) float MinRadius = 0.001f;
		alignas( 4 ) float Falloff = 1.f;
	};

	struct Lights
	{
		DirectionalLight DirectionalLights[ 4 ];
		std::vector<PointLight> PointLights;

		[[nodiscard]] uint32_t GetPointLightSize() { return static_cast<uint32_t>( sizeof( PointLight ) * PointLights.size() ); };
	};

	class Scene : public RefTarget
	{
	public:
		Scene();
		~Scene();

		Ref<Entity> CreateEntityWithIDScript( UUID uuid, const std::string& name = "", const std::string& rScriptName = "" );
	public:

		void OnRenderEditor( const EditorCamera& rCamera, Timestep ts, SceneRenderer& rSceneRenderer );
		void OnRenderRuntime( Timestep ts, SceneRenderer& rSceneRenderer );

		Ref<Entity> DuplicateEntity( Ref<Entity> entity, Ref<Entity> parent = nullptr );
		void DeleteEntity( Ref<Entity> entity );

		void OnUpdate( Timestep ts );
		void OnUpdatePhysics( Timestep ts );

	public:
		template<typename T>
		std::vector<Ref<Entity>> GetAllEntitiesWith( void )
		{
			std::vector<Ref<Entity>> result;

			for( auto&& [ id, entity ] : m_EntityIDMap )
			{
				if( entity->HasComponent<T>() )
					result.push_back( entity );
			}

			return result;
		}

		template<typename Func>
		void Each( Func Function )
		{
			for( auto&& [id, entity] : m_EntityIDMap )
			{
				Function( entity );
			}
		}

		Ref<Entity> GetMainCameraEntity();

		void SetSelectedEntity( Ref<Entity> entity );
		
		Ref<Entity> FindEntityByTag( const std::string& tag );
		Ref<Entity> FindEntityByID( const UUID& id );

		glm::mat4 GetTransformRelativeToParent( Ref<Entity> entity );
		TransformComponent GetWorldSpaceTransform( Ref<Entity> entity );

		[[nodiscard]] bool Raycast( const glm::vec3& Origin, const glm::vec3& Direction, float MaxDistance, RaycastHitResult* pOut );
	public:

		void CopyScene( Ref<Scene>& NewScene );

		void SetName( std::string name ) { m_Name = std::move( name ); }

		std::string& Name() { return m_Name; }
		const std::string& Name() const { return m_Name; }

		const std::string& Filepath() { return m_Filepath; }

		bool m_RuntimeRunning = false;

		void OnRuntimeStart();
		void OnRuntimeEnd();

		entt::registry& GetRegistry() { return m_Registry; }
		const entt::registry& GetRegistry() const { return m_Registry; }

		// This transfers a prefab to an entity.
		// The prefabs holds an entity however that entity is local to it's scene and we want that entity to be our scene.
		Ref<Entity> CreatePrefab( Ref<Prefab> prefabAsset );

		UUID GetId() { return m_SceneID; }
		const UUID GetId() const { return m_SceneID; }

		[[nodiscard]] entt::entity CreateHandle()
		{
			return m_Registry.create();
		}

		void RemoveHandle( entt::entity handle ) 
		{
			// Does the handle exist in out registry.
			if( m_Registry.valid( handle ) )
				m_Registry.destroy( handle );
		}

		static void   SetActiveScene( Scene* pScene );
		static Scene* GetActiveScene();

	protected:
		void OnEntityCreated( Ref<Entity> entity );

	private:

		//////////////////////////////////////////////////////////////////////////
		// TODO: Rework this is as locking a mutex every frame can be bad for performance.

		template<typename Ty, typename... Args>
		Ty& AddComponent(entt::entity entity, Args&&... args )
		{
#if defined( SAT_ENABLE_GAMETHREAD )
			std::unique_lock<std::mutex> Lock( m_Mutex, std::try_to_lock );
#endif
			if( !HasComponent<Ty>( entity ) )
				return m_Registry.emplace<Ty>( entity, std::forward<Args>( args )... );
			else
				return GetComponent<Ty>( entity );
		}

		template<typename Ty>
		bool HasComponent( entt::entity entity )
		{
#if defined( SAT_ENABLE_GAMETHREAD )
			std::unique_lock<std::mutex> Lock( m_Mutex, std::try_to_lock );
#endif
			return m_Registry.any_of<Ty>( entity );
		}

		template<typename Ty>
		void RemoveComponent( entt::entity entity )
		{
#if defined( SAT_ENABLE_GAMETHREAD )
			std::unique_lock<std::mutex> Lock( m_Mutex, std::try_to_lock );
#endif
			if( HasComponent<Ty>( entity ) )
				m_Registry.remove<Ty>( entity );
		}

		template<typename Ty>
		Ty& GetComponent( entt::entity entity )
		{
#if defined( SAT_ENABLE_GAMETHREAD )
			std::unique_lock<std::mutex> Lock( m_Mutex, std::try_to_lock );
#endif
			SAT_CORE_ASSERT( HasComponent<Ty>( entity ), "Entity does not have component!" );

			return m_Registry.get<Ty>( entity );
		}

	public:

		//////////////////////////////////////////////////////////////////////////
		// Loads and opens the scene for play.
		static bool Travel( const std::string& rSceneName );
		static bool AwaitingTravels();
		static void DoTravel();

	private:

		UUID m_SceneID;

		std::string m_Name;
		std::string m_Filepath;

		std::unordered_map<entt::entity, Ref<Entity>> m_EntityIDMap;

		entt::registry m_Registry;

		entt::entity m_SceneEntity{ entt::null };
		Ref<Entity> m_SelectedEntity = nullptr;

		Lights m_Lights;

		std::mutex m_Mutex;

		// TODO: Change raw pointer to Ref?
		PhysicsScene* m_PhysicsScene = nullptr;
	private:

		friend class Entity;
		friend class Prefab;
		friend class SceneHierarchyPanel;
		friend class SceneSerialiser;
		friend class SceneRenderer;
	};
}

namespace std {

	template <>
	struct hash<Saturn::Scene>
	{
		std::size_t operator()( const Saturn::Scene& scene) const
		{
			return hash<uint64_t>()( ( uint64_t ) scene.GetId() );
		}

		std::size_t operator()( Saturn::Scene* scene ) const
		{
			return hash<uint64_t>()( ( uint64_t ) scene->GetId() );
		}

		std::size_t operator()( Saturn::Ref<Saturn::Scene> scene ) const
		{
			return hash<uint64_t>()( ( uint64_t ) scene->GetId() );
		}
	};

}