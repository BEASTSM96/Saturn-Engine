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

#include "sppch.h"
#include "Scene.h"

#include "Entity.h"
#include "Components.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	static const std::string DefaultEntityName = "Empty Entity";
	std::unordered_map<UUID, Scene*> s_ActiveScenes;

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
		s_ActiveScenes[ m_SceneID ] = this;
		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>( m_SceneEntity, m_SceneID );
	}

	Scene::~Scene()
	{
		s_ActiveScenes.erase( m_SceneID );

		m_Registry.clear();
	}

	void Scene::OnUpdate( Timestep ts )
	{

	}

	void Scene::OnRenderEditor( Timestep ts, const EditorCamera& editorCamera )
	{

	}

	Entity Scene::CreateEntity( const std::string& name /*= "" */ )
	{
		Entity entity ={ m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned Entity" : name;

		auto& IDcomponent = entity.AddComponent<IdComponent>();
		entity.GetComponent<IdComponent>().ID ={};

		m_EntityIDMap[ IDcomponent.ID ] = entity;
		return entity;
	}

	Entity Scene::CreateEntityWithID( UUID uuid, const std::string& name /*= "" */ )
	{
		auto entity = Entity{ m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<IdComponent>();
		idComponent.ID = uuid;

		entity.AddComponent<TransformComponent>();
		if( !name.empty() )
			entity.AddComponent<TagComponent>( name );

		SAT_CORE_ASSERT( m_EntityIDMap.find( uuid ) == m_EntityIDMap.end(), "Entity has the same name!" );
		m_EntityIDMap[ uuid ] = entity;
		return entity;
	}

	Entity Scene::FindEntityByTag( const std::string& tag )
	{
		auto view = m_Registry.view<TagComponent>();
		for( auto entity : view )
		{
			const auto& canditate = view.get<TagComponent>( entity ).Tag;
			if( canditate == tag )
				return Entity( entity, this );
		}

		return Entity{};
	}

	void Scene::DestroyEntity( Entity entity )
	{
		m_Registry.destroy( entity.m_EntityHandle );
	}

	void Scene::CopyScene( Ref<Scene>& NewScene )
	{

	}

}