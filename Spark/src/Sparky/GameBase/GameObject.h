#pragma once

/*
*					Sparky GameObject
* 1. This class for NOT for the objects, the objects are in the Entity class.
*  1.5.This class IS for all of the Entities, that the renderer can rendered on one layer (GameLayer.h).
* 
*********************************************************************************************************
*										Object (needed for serialisation).
*										Friend Class GameLayer (needed for rendering on that layer).
*/

#ifdef SPARKY_GAME_BASE

#include "Sparky/Core/Serialisation/Object.h"
#include "Sparky/Renderer/Texture.h"

#include "Sparky/Renderer/OrthographicCamera.h"

#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/OrthographicCamera.h"

#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Shader.h"

#include "Sparky/Renderer/Renderer.h"

#include "Sparky/Layer.h"

#include "Sparky/Input.h"

#include "Sparky/KeyCodes.h"

//#include "Sparky/Application.h"

namespace Sparky {

	class SPARKY_API GameObject : public Object /* Sparky GameObject */
	{
	public:
		GameObject();
		GameObject(const std::string& objectname, Json::Value& reconstructionValue);
		virtual ~GameObject() = default;

		void Render();
		void Init();


		void OnKeyInput(KeyPressedEvent & InEvent);

		void OnUpdate(Timestep ts);


		std::string test = "test";


		Sparky::Ref<Sparky::Shader> m_playerShader;
		Sparky::Ref<Sparky::VertexArray> m_playerVA;

		Sparky::Ref<Sparky::Texture2D> m_playerTexture;

		Sparky::Ref<Sparky::Shader> m_Shader;
		Sparky::Ref<Sparky::VertexArray> m_VertexArray;

		Sparky::Ref<Sparky::Shader> m_flatShader, m_TextureShader;
		Sparky::Ref<Sparky::VertexArray> m_SquareVA;

		Sparky::Ref<Sparky::Texture2D> m_Texture, m_beastlogo;

		float m_PlayerRotation = 0.0f;
		float m_PlayerRotationSpeed = 180.0f;

		glm::vec3 m_PlayerPosition;
		float m_PlayerMoveSpeed = 50.0f;

		bool m_ShadersDone = false;

	protected:
		virtual void archive() override {}

	private:
		friend class GameLayer;

		static GameObject* s_Instance;
	};
}

#endif