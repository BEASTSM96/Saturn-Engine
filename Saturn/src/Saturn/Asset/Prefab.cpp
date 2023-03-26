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
#include "Prefab.h"

#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Entity.h"

namespace Saturn {

	template<typename... V>
	static void CopyComponentIfExists( entt::entity dst, entt::entity src, entt::registry& rRegistry, entt::registry& rDstRegistry )
	{
		( [&]()
			{
				if( rRegistry.any_of<V>( src ) )
				{
					auto& srcComponent = rRegistry.get<V>( src );
					rDstRegistry.emplace_or_replace<V>( dst, srcComponent );
				}
			}( ), ... );
	}

	template<typename... V>
	static void CopyComponentIfExists( ComponentGroup<V...>, entt::entity dst, entt::entity src, entt::registry& rRegistry, entt::registry& rDstRegistry )
	{
		CopyComponentIfExists<V...>( dst, src, rRegistry, rDstRegistry );
	}

	Prefab::Prefab()
	{
	}

	Prefab::~Prefab()
	{
	}

	void Prefab::Create( Entity& srcEntity )
	{
		m_Scene = Ref<Scene>::Create();

		m_Scene->SetName( "Prefab scene" );

		if( srcEntity.Vaild() )
			m_Entity = CreateFromEntity( srcEntity );
	}

	void Prefab::Create()
	{
		m_Scene = Ref<Scene>::Create();

		m_Scene->SetName( "Prefab scene" );
	}

	Entity Prefab::PrefabToEntity( Ref<Scene> Scene, Entity entity )
	{
		Entity e = Scene->CreateEntity();
		e.AddComponent<PrefabComponent>().AssetID = ID;

		// Now we need to find the root entity of the prefab.

		auto view = m_Scene->GetAllEntitiesWith<RelationshipComponent>();

		Entity RootEntity;

		for( auto& entity : view )
		{
			Entity ent( entity, m_Scene.Pointer() );

			if( ent.GetParent() == 0 )
			{
				RootEntity = ent;
				break;
			}
		}

		if( !RootEntity )
			RootEntity = m_Entity;

		auto id = e.GetComponent<IdComponent>().ID;

		CopyComponentIfExists( AllComponents{}, e, RootEntity, m_Scene->m_Registry, Scene->m_Registry );

		// We don't want the same id, what if we spawn this prefab and it has the same id?
		e.GetComponent<IdComponent>().ID = id;

		for( auto& childId : RootEntity.GetChildren() )
		{
			Entity child = CreateChildren( m_Scene->FindEntityByID( childId ), Scene );

			child.SetParent( e.GetComponent<IdComponent>().ID );
		}

		return e;
	}

	void Prefab::CreateScene()
	{
		m_Scene = nullptr;
		m_Scene = Ref<Scene>::Create();
	}

	Entity Prefab::CreateFromEntity( Entity srcEntity )
	{
		Entity result = m_Scene->CreateEntity();
		result.AddComponent<PrefabComponent>().AssetID = ID;
		
		auto& rc = srcEntity.GetComponent<RelationshipComponent>();

		CopyComponentIfExists( AllComponents{}, result, srcEntity, srcEntity.m_Scene->m_Registry, m_Scene->m_Registry );

		for( auto& childId : srcEntity.GetChildren() )
		{
			Entity child = CreateFromEntity( srcEntity.m_Scene->FindEntityByID( childId ) );

			auto& rc = result.GetComponent<RelationshipComponent>();

			child.SetParent( result.GetComponent<IdComponent>().ID );
			//rc.ChildrenID.push_back( child.GetComponent<IdComponent>().ID );

			SAT_CORE_INFO("{0}", rc.ChildrenID.size() );
		}

		SAT_CORE_INFO("{0}", result.GetComponent<RelationshipComponent>().ChildrenID.size() );

		return result;
	}

	Entity Prefab::CreateChildren( Entity parent, Ref<Scene> Scene )
	{
		// Create the child in the new scene.
		Entity child = Scene->CreateEntity();
		
		// Copy Components, from our child in the scene.
		CopyComponentIfExists( AllComponents{}, child, parent, m_Scene->m_Registry, Scene->m_Registry );

		// Check if this entity has any children.
		for( auto& childId : child.GetChildren() )
		{
			SAT_CORE_INFO( "Child in Child has {0} children", child.GetComponent<RelationshipComponent>().ChildrenID.size() );

			Entity c = CreateChildren( child, Scene );

			c.SetParent( child.GetComponent<IdComponent>().ID );
			child.GetComponent<RelationshipComponent>().ChildrenID.push_back( c.GetComponent<IdComponent>().ID );
		}

		return child;
	}

}