#pragma once

/*
*					Sparky GameObject
* 1. This class SATURN_API for NOT for the objects, the objects are in the Entity class SATURN_API.
*  1.5.This class SATURN_API IS for all of the Entities, that the renderer can rendered on one layer (GameLayer.h).
* 
*********************************************************************************************************
*										Object (needed for serialisation).
*										Friend class SATURN_API GameLayer (needed for rendering on that layer).
*										Entity (for ECS)
*/

// Always include this 
#include "entt.hpp"
#include "Saturn/Scene/Scene.h"
#include "Saturn/Core/Serialisation/Object.h"
#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Entity.h"


#ifdef SPARKY_GAME_BASE

#include "Saturn/Renderer/3D/Mesh/Model.h"
#include "Saturn/Renderer/Texture.h"
#include "Saturn/Renderer/OrthographicCamera.h"
#include "Saturn/Renderer/VertexArray.h"
#include "Saturn/Renderer/Shader.h"
#include "Saturn/Renderer/Renderer.h"
#include "Saturn/Renderer/3D/3dShader.h"

#include "Saturn/Layer.h"

#include "Saturn/Input.h"

#include "Saturn/KeyCodes.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace Saturn {

	enum E_GameObjectState : SPint //uint64_t
	{
		Idle,
		Walking,
		Runing		
	};


	class SATURN_API SATURN_API GameObject : public Entity, public Object /* Sparky GameObject */
	{
	public:
		GameObject();
		GameObject(const std::string& objectname, Json::Value& reconstructionValue);
		//////////////////////////////////////////////////////////////////////////////

		GameObject(entt::entity handle, Scene* scene);
		GameObject(const GameObject& other) = default;
		//////////////////////////////////////////////////////////////////////////////////

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			SAT_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			SAT_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			SAT_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		operator bool() const { return m_EntityHandle != entt::null; }

		//////////////////////////////////////////////////////////////////////////////////
		virtual ~GameObject() = default;

		void Render();
		void Init();

		void OnPos();
		void OnKeyInput(KeyPressedEvent & InEvent);

		void OnUpdate(Timestep ts);

	private:
		void renderSphere();
		void renderCube();
		void OnPBRRender();
		void PBRirradiance_conversion();
	public:

		E_GameObjectState GetObjectState() { return GameObjectState; }

		SPint GetObjectStateInInt() { return GameObjectState; }
 
		E_GameObjectState SetObjectState(E_GameObjectState newstate) { if (newstate == GameObjectState) { SAT_CORE_WARN("You are setting a new State, when the old state is the same!"); return E_GameObjectState::Idle; GameObjectState = E_GameObjectState::Idle; } else return GameObjectState = newstate; }

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

		E_GameObjectState GameObjectState = E_GameObjectState::Idle;

		std::vector<DShader*> shaders;


		std::string test = "test";

		unsigned int cubeVAO = 0;
		unsigned int cubeVBO = 0;

		glm::mat4& transform = glm::mat4(1.0f);

		unsigned int loadTexture(char const* path);

	public:

		glm::mat4& GetTransform() {
			return GetComponent<TransformComponent>().Transform;
		}

		DShader *		ourShader;

		Model *			ourModel;

		DShader *		shader;

		DShader *		pbrShader;
		DShader *		equirectangularToCubemapShader;
		DShader *		backgroundShader;

		
		Ref<Shader> m_playerShader;
		Ref<VertexArray> m_playerVA;

		Ref<Texture2D> m_playerTexture;

		Ref<Shader> m_Shader;
		Ref<VertexArray> m_VertexArray;

		Ref<Shader> m_flatShader, m_TextureShader;
		Ref<VertexArray> m_SquareVA;

		Ref<Texture2D> m_Texture, m_beastlogo;

		float m_PlayerRotation = 0.0f;
		float m_PlayerRotationSpeed = 180.0f;

		glm::vec3 m_PlayerPosition;
		float m_PlayerMoveSpeed = 50.0f;

		bool m_ShadersDone = false;
		
		bool m_3D = true;
	protected:
		virtual void archive() override {}

	private:
		friend class SATURN_API GameLayer;

		friend class SATURN_API Scene;

		static GameObject* s_Instance;
	};
}

#endif