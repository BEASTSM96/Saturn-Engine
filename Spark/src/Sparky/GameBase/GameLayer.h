#pragma once

/*
*					Sparky GameLayer
*********************************************************************************************************
*										Layer.
*/

#ifdef SPARKY_GAME_BASE

#include "Sparky/Layer.h"

#include "Sparky/Core/Serialisation/Object.h"

#include "Sparky/Renderer/OrthographicCamera.h"

#include "Sparky/Renderer/Renderer.h"

#include "Sparky/GameBase/GameObject.h"

#include "Sparky/Application.h"

#include "Sparky/Events/KeyEvent.h"
#include "Sparky/Events/Event.h"
#include "Sparky/Events/MouseEvent.h"
#include "Sparky/Events/ApplicationEvent.h"

#include<string>

namespace Sparky {
	class SPARKY_API GameLayer : public Layer
	{
	public:

		friend class GameObject;

		GameLayer();
		~GameLayer();

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& event) override;

		void Sumbit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, FTransform Intransform);

		void AddGameObjects(GameObject gameObject);

		bool OnKeyPressed(KeyPressedEvent& event);

		void OnGameObjectMove(glm::vec3& position);

		inline const std::string& GetName() const { return m_DebugName; }

		inline std::vector<GameObject> GetGameObjects() { return gameObjects; }

		inline size_t GetGameObjectsSize() { return gameObjects.size(); }

		inline uint64_t GetGameObjectsSizeUint64() { return gameObjects.size(); }

		glm::vec3 m_CameraPosition;
		float m_CameraMoveSpeed = 5.0f;


		float m_CameraRotation = 0.0f;
		float m_CameraRotationSpeed = 180.0f;


	protected:
		std::string m_DebugName = "GameLayer";

		OrthographicCamera m_Camera;

	private:
		static GameLayer* s_Instance;

		std::vector<GameObject> gameObjects;

		Json::Value serialiser;

	};
}

//#else
//#error Error!, you are trying to call Player func's while 'SPARKY_GAME_BASE' is not defined!
#endif // SPARKY_GAME_BASE