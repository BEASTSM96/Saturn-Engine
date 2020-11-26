#include "sppch.h"
#include "GameLayer.h"

#include <GLFW/glfw3.h>

#include "Saturn/Layer.h"

#include "Saturn/Input.h"

#include "Saturn/KeyCodes.h"

#include "Saturn/MouseButtons.h"

#include "Saturn/Core/Base.h"

#pragma warning(disable: 4101)

#ifdef SPARKY_GAME_BASE
/*  THIS class SATURN_API WILL PROB BE THE BATCH RENDERER!*/

namespace Saturn {


	GameLayer* GameLayer::s_Instance = nullptr;

	bool firstMouse = true;

	GameLayer::GameLayer() : Layer("GameLayer"), m_3DCamera(glm::vec3(0.0f, 0.0f, 3.0f)), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraController(1280.0f / 720.0f)
	{
		SAT_CORE_ASSERT(!s_Instance, "More than one Game Layer already exists!");

		s_Instance = this;

		lastX = Application::Get().GetWindow().GetWidth() / 2.0f;
		lastY = Application::Get().GetWindow().GetHeight() / 2.0f;

	}

	GameLayer::~GameLayer()
	{
	}

	void GameLayer::OnAttach()
	{
	}

	void GameLayer::OnUpdate(Timestep ts)
	{
		m_CameraController.OnUpdate(ts);

		//RendererAPI::SetClearColor(0.45f, 0.4f, 0.4f, 1.f);
		//RendererAPI::Clear(0.45f, 0.4f, 0.4f, 1.f);

		float velocity = 50 * 0.1f;
		if (Input::IsKeyPressed(SAT_KEY_W))
			m_3DCamera.ProcessKeyboard(Camera_Movement::FORWARD, ts);
		if (Input::IsKeyPressed(SAT_KEY_S))
			m_3DCamera.ProcessKeyboard(Camera_Movement::BACKWARD, ts);
		if (Input::IsKeyPressed(SAT_KEY_A))
			m_3DCamera.ProcessKeyboard(Camera_Movement::LEFT, ts);
		if (Input::IsKeyPressed(SAT_KEY_D))
			m_3DCamera.ProcessKeyboard(Camera_Movement::RIGHT, ts);


		if (Input::IsKeyPressed(SAT_KEY_LEFT_ALT) && Input::IsKeyPressed(SAT_KEY_J)) {
			// draw in wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else if (Input::IsKeyPressed(SAT_KEY_LEFT_ALT) && Input::IsKeyPressed(SAT_KEY_K))
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}


		if (Input::IsKeyPressed(SAT_KEY_LEFT_ALT) && Input::IsKeyPressed(SAT_KEY_M)) {

			GLFWwindow* win = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


		}
		else if (Input::IsKeyPressed(SAT_KEY_LEFT_ALT) && Input::IsKeyPressed(SAT_KEY_N))
		{
			GLFWwindow* win = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		for (GameObject gb : gameObjects)
		{
			gb.Render();
			gb.OnUpdate(ts);
		}
	}

	void GameLayer::OnEvent(Event& event)
	{
		m_CameraController.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(SAT_BIND_EVENT_FN(GameLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseScrolledEvent>(SAT_BIND_EVENT_FN(GameLayer::OnMouseScrolled));
		dispatcher.Dispatch<MouseMovedEvent>(SAT_BIND_EVENT_FN(GameLayer::OnMouseMoved));

	}


	bool GameLayer::OnKeyPressed(KeyPressedEvent& event)
	{

		for (GameObject i : gameObjects)
		{
			i.OnKeyInput(event);

			return true;
		}

		return false;

	}

	void GameLayer::OnGameObjectMove(glm::vec3& position)
	{
		m_Camera.SetPosition(position);
	}

	void GameLayer::SetCamPos(glm::vec3& position)
	{
		m_Camera.SetPosition(position);
	}

	bool GameLayer::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_3DCamera.ProcessMouseScroll(e.GetYOffset());

		return false;
	}

	bool GameLayer::OnMouseCliked(MouseButtonEvent& e)
	{
		//dispatcher.Dispatch<MouseMovedEvent>(SAT_BIND_EVENT_FN(GameLayer::OnMouseMoved));

		return false;
	}

	bool GameLayer::OnMouseMoved(MouseMovedEvent& e)
	{
		static bool CanMove = false;
		if (Input::IsKeyPressed(SAT_KEY_LEFT_ALT) && Input::IsKeyPressed(SAT_KEY_M))
		{
			CanMove = true;
		}
		if (!CanMove)
		{
			float xpos, ypos;


			if (Input::IsMouseButtonPressed(SAT_MOUSE_BUTTON_LEFT))
			{
				auto xpos = Input::GetMouseX();
				auto ypos = Input::GetMouseY();


				float xoffset = xpos - lastX;
				float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

				lastX = xpos;
				lastY = ypos;

				m_3DCamera.ProcessMouseMovement(float(xoffset), float(yoffset));

				return false;


			}

			//xpos = Input::GetMouseX();
			//ypos = Input::GetMouseY();


			//if (firstMouse)
			//{
			//	lastX = xpos;
			//	lastY = ypos;
			//	firstMouse = false;
			//}

			//float xoffset = xpos - lastX;
			//float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

			//lastX = xpos;
			//lastY = ypos;

			//m_3DCamera.ProcessMouseMovement(float(xoffset), float(yoffset));
		}
		if (Input::IsKeyPressed(SAT_KEY_LEFT_ALT) && Input::IsKeyPressed(SAT_KEY_N))
		{
			CanMove = false;
			return false;
		}

		return false;
	}

	void GameLayer::AddGameObjects(GameObject gameObject)
	{
		gameObjects.push_back(gameObject);
	}
}
#endif // SPARKY_GAME_BASE