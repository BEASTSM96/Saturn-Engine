#pragma once

/*
*					Sparky GameLayer
*********************************************************************************************************
*										Layer.
*/

#ifdef SPARKY_GAME_BASE

#include "Saturn/Layer.h"

#include "Saturn/Core/Serialisation/Object.h"

#include "Saturn/Renderer/OrthographicCamera.h"

#include "Saturn/Renderer/Renderer.h"

#include "Saturn/GameBase/GameObject.h"

#include "Saturn/Application.h"

#include "Saturn/Events/KeyEvent.h"
#include "Saturn/Events/Event.h"
#include "Saturn/Events/MouseEvent.h"
#include "Saturn/Events/ApplicationEvent.h"

#include "Saturn/Renderer/OrthographicCameraController.h"

#include<string>

namespace Saturn {
	class SATURN_API GameLayer : public Layer
	{
	public:

		friend class GameObject;

		GameLayer();
		~GameLayer();

		void OnAttach() override;

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& event) override;

		void Sumbit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, FTransform Intransform);

		void AddGameObjects(GameObject gameObject);

		bool OnKeyPressed(KeyPressedEvent& event);

		void OnGameObjectMove(glm::vec3& position);

		void SetCamPos(glm::vec3& position);

	public:
		const std::string& GetName() const { return m_DebugName; }

		const SCamera& Get3DLayerCamera() const { return m_3DCamera; }

		SCamera & Get3DCamera() { return m_3DCamera; }

		const OrthographicCamera& GetLayerCamera() const { return m_Camera; }

		const OrthographicCameraController& GetCameraController() const { return m_CameraController; }

		std::vector<GameObject>& GetGameObjects() { return gameObjects; }

		size_t GetGameObjectsSize() { return gameObjects.size(); }

		uint64_t GetGameObjectsSizeUint64() { return gameObjects.size(); }


	public:
		glm::vec3 m_CameraPosition;
		float m_CameraMoveSpeed = 5.0f;


		float m_CameraRotation = 0.0f;
		float m_CameraRotationSpeed = 180.0f;


	protected:
		std::string m_DebugName = "GameLayer";

		OrthographicCamera m_Camera;

		SCamera  m_3DCamera;

		OrthographicCameraController m_CameraController;

		float lastX, lastY;

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnMouseCliked(MouseButtonEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);

		std::vector<GameObject> gameObjects;

		Json::Value serialiser;

	private:
		static GameLayer* s_Instance;
	};
}
#endif // SPARKY_GAME_BASE