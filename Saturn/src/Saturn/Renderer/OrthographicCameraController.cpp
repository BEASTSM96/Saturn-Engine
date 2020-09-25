#include "sppch.h"
#include "OrthographicCameraController.h"


#include "Saturn/Input.h"
#include "Saturn/KeyCodes.h"
#include "Saturn/MouseButtons.h"



namespace Saturn {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation) : m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation), m_3DCamera((glm::vec3(0.0f, 0.0f, 3.0f)))
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		float velocity = 50 * 0.1f;
		if (Input::IsMouseButtonPressed(SAT_MOUSE_BUTTON_LEFT))
			CameraMove();
		if (Input::IsKeyPressed(SAT_KEY_S))
			m_3DCamera.ProcessKeyboard(Camera_Movement::BACKWARD, ts);
		if (Input::IsKeyPressed(SAT_KEY_A))
			m_3DCamera.ProcessKeyboard(Camera_Movement::LEFT, ts);
		if (Input::IsKeyPressed(SAT_KEY_D))
			m_3DCamera.ProcessKeyboard(Camera_Movement::RIGHT, ts);

		/*
		else if (Input::IsKeyPressed(SAT_KEY_D))
			m_CameraPosition.x += m_CameraTranslationSpeed * ts;

		if (Input::IsKeyPressed(SAT_KEY_W))
			m_CameraPosition.y -= m_CameraTranslationSpeed * ts;
		else if (Input::IsKeyPressed(SAT_KEY_S))
			m_CameraPosition.y += m_CameraTranslationSpeed * ts;

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(SAT_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;
			if (Input::IsKeyPressed(SAT_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			m_Camera.SetRotation(m_CameraRotation);
		}

		m_Camera.SetPosition(m_CameraPosition);

		m_CameraTranslationSpeed = m_ZoomLevel;*/
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(SAT_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(SAT_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
	/*	m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);*/

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthographicCameraController::CameraMove()
	{
		if (Input::IsMouseButtonPressed(SAT_MOUSE_BUTTON_LEFT))
		{


			auto xpos = Input::GetMouseX();
			auto ypos = Input::GetMouseY();

			float lastX = 0.0f;
			float lastY = 0.0f;

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

			lastX = xpos;
			lastY = ypos;

			m_3DCamera.ProcessMouseMovement(float(xoffset), float(yoffset));
			
			return false;

		}

		return false;
	}

	void OrthographicCameraController::OnResize(float width, float height)
	{
		m_AspectRatio = width / height;
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}
	
}