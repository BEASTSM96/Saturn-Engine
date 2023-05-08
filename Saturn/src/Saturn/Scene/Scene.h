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

#pragma once

#include "Saturn/Core/Base.h"

#include "Saturn/Core/Renderer/EditorCamera.h"

#include "Saturn/Core/UUID.h"
#include "Saturn/Core/Timestep.h"

#include "entt.hpp"

#include <shared_mutex>

namespace Saturn {

	class Entity;
	class Prefab;
	struct TransformComponent;

	using EntityMap = std::unordered_map<UUID, Entity>;

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

	class JoltRuntime;
	class SClass;
	class SceneRenderer;

	class Scene : public CountedObj
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity( const std::string& name =  "" );
		Entity CreateEntityWithID( UUID uuid, const std::string& name = "" );

		void DestroyEntity( Entity entity );

		void OnRenderEditor( const EditorCamera& rCamera, Timestep ts, SceneRenderer& rSceneRenderer );
		void OnRenderRuntime( Timestep ts, SceneRenderer& rSceneRenderer );

		void DuplicateEntity( Entity entity );
		void DeleteEntity( Entity entity );

		template<typename T>
		auto GetAllEntitiesWith( void )
		{
			return m_Registry.view<T>();
		}

		Entity GetMainCameraEntity();

		void OnUpdate( Timestep ts );
		void OnUpdatePhysics( Timestep ts );

		void SetSelectedEntity( entt::entity entity ) { m_SelectedEntity = entity; }
		
		Entity FindEntityByTag( const std::string& tag );
		Entity FindEntityByID( const UUID& id );

		glm::mat4 GetTransformRelativeToParent( Entity entity );
		TransformComponent GetWorldSpaceTransform( Entity entity );

		void CopyScene( Ref<Scene>& NewScene );

		void SetName( const std::string& name ) { m_Name = name; }

		std::string& Name() { return m_Name; }
		const std::string& Name() const { return m_Name; }

		const std::string& Filepath() { return m_Filepath; }

		bool m_RuntimeRunning = false;

		void OnRuntimeStart();
		void OnRuntimeEnd();

		entt::registry& GetRegistry() { return m_Registry; }
		const entt::registry& GetRegistry() const { return m_Registry; }

		std::pair<Entity, SClass*> CreatePrefab( Ref<Prefab> prefabAsset );

		UUID GetId() { return m_SceneID; }
		const UUID GetId() const { return m_SceneID; }

		static void   SetActiveScene( Scene* pScene );
		static Scene* GetActiveScene();

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

	private:

		UUID m_SceneID;

		std::string m_Name;
		std::string m_Filepath;

		EntityMap m_EntityIDMap;

		entt::registry m_Registry;

		entt::entity m_SceneEntity;
		entt::entity m_SelectedEntity;

		Lights m_Lights;

		std::mutex m_Mutex;

		JoltRuntime* m_PhysicsRuntime = nullptr;

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