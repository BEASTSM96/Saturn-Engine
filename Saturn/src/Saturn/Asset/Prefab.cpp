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
#include "Prefab.h"

#include "Saturn/Scene/Scene.h"

namespace Saturn {

	template<typename... V>
	static void CopyComponentIfExists( entt::entity dst, entt::entity src, entt::registry& rRegistry, entt::registry& rDstRegistry )
	{
		( [&]()
			{
				SAT_CORE_INFO( "Trying Copying {0}", typeid( V ).name() );

				if( rRegistry.any_of<V>( src ) )
				{
					SAT_CORE_INFO( "Copied {0}", typeid( V ).name() );

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
		m_Scene = Ref<Scene>::Create();
	}

	Prefab::~Prefab()
	{
	}

	void Prefab::Create( Entity& srcEntity )
	{
		m_Scene = Ref<Scene>::Create();

		m_Scene->SetName( "Prefab scene" );

		CreateFromEntity( srcEntity );
	}

	Entity Prefab::PrefabToEntity( Ref<Scene> Scene )
	{
		Entity e;

		// TODO:

		return e;
	}

	void Prefab::CreateFromEntity( Entity& srcEntity )
	{
		m_Entity = m_Scene->CreateEntity( srcEntity.GetComponent<TagComponent>().Tag );

		CopyComponentIfExists( AllComponents{}, m_Entity, srcEntity, srcEntity.GetScene().m_Registry, m_Scene->m_Registry );
	}

}