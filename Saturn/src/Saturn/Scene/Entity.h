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

#pragma once

#include "Components.h"
#include "Scene.h"
#include "EntityVisibility.h"

#include "Saturn/GameFramework/SClass.h"

#include <glm/glm.hpp>
#include "entt.hpp"

namespace Saturn {

	class Entity : public SClass
	{
	public:
		Entity() = default;
		Entity( entt::entity handle, Scene* scene ) : m_EntityHandle( handle ), m_Scene( scene ) { }
		Entity( const Entity& other ) = default;

		template<typename T, typename... Args>
		T& AddComponent( Args&&... args )
		{
			SAT_CORE_ASSERT( !HasComponent<T>(), "Entity already has component!" );
			return m_Scene->m_Registry.emplace<T>( m_EntityHandle, std::forward<Args>( args )... );
		}

		template<typename T>
		T& GetComponent()
		{
			SAT_CORE_ASSERT( HasComponent<T>(), "Entity does not have component!" );
			return m_Scene->m_Registry.get<T>( m_EntityHandle );
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.any_of<T>( m_EntityHandle );
		}

		template<typename T>
		void RemoveComponent()
		{
			if( HasComponent<T>() )
				m_Scene->m_Registry.remove<T>( m_EntityHandle );
		}

		bool Vaild()
		{
			return m_Scene->m_Registry.valid( m_EntityHandle );
		}

		Scene& GetScene() { return *m_Scene; }
		const Scene& GetScene() const { return *m_Scene; }

		void SetScene( Scene* sceneIn )
		{
			m_Scene = sceneIn;
		}

		glm::mat4 Transform() { return m_Scene->m_Registry.get<TransformComponent>( m_EntityHandle ).GetTransform(); }
		//const glm::mat4& Transform() const { return m_Scene->m_Registry.get<TransformComponent>( m_EntityHandle ); }

		const std::string& Tag() const { return m_Scene->m_Registry.get<TagComponent>( m_EntityHandle ).Tag; }
		std::string& Tag() { return m_Scene->m_Registry.get<TagComponent>( m_EntityHandle ).Tag; }

		operator bool() const { return m_EntityHandle != entt::null && m_Scene != nullptr && m_Scene->m_Registry.valid( m_EntityHandle ); }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t () const { return ( uint32_t ) m_EntityHandle; }

		bool operator==( const Entity& other ) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=( const Entity& other ) const
		{
			return !( *this == other );
		}

		UUID GetUUID() { return GetComponent<IdComponent>().ID; }

		void BeginPlay() override {}
		void OnUpdate( Saturn::Timestep ts ) override {}

		virtual void __create_entity( Entity* e ) {};

	protected:

		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

	private:

		friend class Scene;
	};
}