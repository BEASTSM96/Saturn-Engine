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

#include "Saturn/Core/UUID.h"
#include "Saturn/Core/Timestep.h"

#include "entt.hpp"

namespace Saturn {

	class Entity;

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

	class PhysXRuntime;

	class Scene : public CountedObj
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity( const std::string& name =  "" );
		Entity CreateEntityWithID( UUID uuid, const std::string& name = "" );

		void DestroyEntity( Entity entity );

		void OnRenderEditor( const EditorCamera& Camera, Timestep ts );

		void DuplicateEntity( Entity entity );
		void DeleteEntity( Entity entity );

		template<typename T>
		auto GetAllEntitiesWith( void )
		{
			return m_Registry.view<T>();
		}

		void OnUpdate( Timestep ts );
		void OnUpdatePhysics( Timestep ts );

		void SetSelectedEntity( entt::entity entity ) { m_SelectedEntity = entity; }
		
		Entity FindEntityByTag( const std::string& tag );

		void CopyScene( Ref<Scene>& NewScene );

		void SetName( const std::string& name ) { m_Name = name; }

		std::string& Name() { return m_Name; }
		const std::string& Name() const { return m_Name; }

		const std::string& Filepath() { return m_Filepath; }

		bool m_RuntimeRunning = false;

		void OnRuntimeStart();
		void OnRuntimeEnd();

	private:

		UUID m_SceneID;

		std::string m_Name;
		std::string m_Filepath;

		EntityMap m_EntityIDMap;

		entt::registry m_Registry;

		entt::entity m_SceneEntity;
		entt::entity m_SelectedEntity;

		DirectionalLight m_DirectionalLight[ 4 ];

		PhysXRuntime* m_PhysXRuntime;

	private:

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerialiser;
		friend class SceneRenderer;
	};
}