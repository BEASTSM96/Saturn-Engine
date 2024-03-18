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

#include "Components.h"
#include "Scene.h"
#include "EntityVisibility.h"

#include "Saturn/GameFramework/SClass.h"
#include "Saturn/GameFramework/Core/GameScript.h"

#include <glm/glm.hpp>
#include "entt.hpp"

namespace Saturn {

	class Entity : public SClass
	{
		//////////////////////////////////////////////////////////////////////////
		// Needed for game class.

		SAT_DECLARE_CLASS_MOVE( Entity, SClass )
	public:
		Entity();
		Entity( Scene* scene );
		Entity( const std::string& rName, UUID Id );
		Entity( const Entity& other );

		virtual ~Entity();

	public:

		template<typename T, typename... Args>
		T& AddComponent( Args&&... args )
		{
			return m_Scene->AddComponent<T>( m_EntityHandle, std::forward<Args>( args )... );
		}

		template<typename T>
		T& GetComponent()
		{
			return m_Scene->GetComponent<T>( m_EntityHandle );
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->HasComponent<T>( m_EntityHandle );
		}

		template<typename T>
		void RemoveComponent()
		{
			m_Scene->RemoveComponent<T>( m_EntityHandle );
		}

		bool Valid()
		{
			return m_Scene->m_Registry.valid( m_EntityHandle );
		}

		bool Valid() const
		{
			return m_Scene->m_Registry.valid( m_EntityHandle );
		}

		Scene& GetScene() { return *m_Scene; }
		const Scene& GetScene() const { return *m_Scene; }

		glm::mat4 Transform() { return m_Scene->m_Registry.get<TransformComponent>( m_EntityHandle ).GetTransform(); }
		
		const std::string& Name() const { return m_Scene->m_Registry.get<TagComponent>( m_EntityHandle ).Tag; }
		std::string& Name() { return m_Scene->m_Registry.get<TagComponent>( m_EntityHandle ).Tag; }

		void SetName( const std::string& rName );

		entt::entity GetHandle() { return m_EntityHandle; }
		const entt::entity GetHandle() const { return m_EntityHandle; }

	public:
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

	public:
		UUID GetUUID() { return GetComponent<IdComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		void BeginPlay() override {}
		void OnUpdate( Saturn::Timestep ts ) override {}
		void OnPhysicsUpdate( Saturn::Timestep ts ) override {}

		void SetParent( const UUID& rID ) 
		{
			GetComponent<RelationshipComponent>().Parent = rID;
		}

		UUID GetParent()
		{
			return GetComponent<RelationshipComponent>().Parent;
		}

		std::vector<UUID>& GetChildren()             { return GetComponent<RelationshipComponent>().ChildrenID; }
		
		bool HasParent()   { return GetComponent<RelationshipComponent>().Parent != 0; }
		bool HasChildren() { return GetComponent<RelationshipComponent>().ChildrenID.size() > 0; }

	public:
		static void Serialise( const Ref<Entity>& rObject, std::ofstream& rStream );
		static void Deserialise( Ref<Entity>& rObject, std::istream& rStream );

	protected:

		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

	private:

		friend class Scene;
		friend class Prefab;
	};
}