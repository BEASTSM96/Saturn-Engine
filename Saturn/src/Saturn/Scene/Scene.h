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

#include "Saturn/Core/Base.h"

#include "Saturn/Core/Renderer/EditorCamera.h"

#if defined ( SAT_LINUX )

#include "Entity.h"

#endif

#include "Saturn/Core/UUID.h"
#include "Saturn/Core/Timestep.h"

#include "entt.hpp"

namespace Saturn {

#if defined ( SAT_LINUX )
	using EntityMap = std::unordered_map<UUID, Entity>;
#else

	class Entity;

	using EntityMap = std::unordered_map<UUID, Entity>;

#endif

	struct SceneComponent
	{
		UUID SceneID;
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity( const std::string& name =  "" );
		Entity CreateEntityWithID( UUID uuid, const std::string& name = "" );

		void DestroyEntity( Entity entity );

		void OnRenderEditor( Timestep ts );

		template<typename T>
		auto GetAllEntitiesWith( void )
		{
			return m_Registry.view<T>();
		}

		void OnUpdate( Timestep ts );
		void SetSelectedEntity( entt::entity entity ) { m_SelectedEntity = entity; }
		Entity FindEntityByTag( const std::string& tag );
		void CopyScene( Ref<Scene>& NewScene );

		void SetName( const std::string& name ) { m_Name = name; }

		std::string& Name() { return m_Name; }
		const std::string& Name() const { return m_Name; }

		Entity& LightEntity();
		std::vector<Entity>& VisableEntities();

	private:

		UUID m_SceneID;

		std::string m_Name;

		EntityMap m_EntityIDMap;

		entt::entity m_SceneEntity;
		entt::registry m_Registry;

		entt::entity m_SelectedEntity;

	private:

		friend class Entity;
		friend class SceneHierarchyPanel;
	};
}