#pragma once

#include "OrthographicCamera.h"
#include "Saturn/Renderer/3DCamera.h"
#include "Saturn/Core/Timestep.h"

#include "Saturn/Events/ApplicationEvent.h"
#include "Saturn/Events/MouseEvent.h"

namespace Saturn {

	class SATURN_API OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		SCamera *  Get3DCameraTest() { return m_3DCameraTest; }
		const SCamera *  Get3DCameraTest() const { return m_3DCameraTest; }

		SCamera& Get3DCamera() { return m_3DCamera; }
		const SCamera& Get3DCamera() const { return m_3DCamera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

		bool CameraMove();

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		SCamera m_3DCamera;


		SCamera * m_3DCameraTest;

		bool m_Rotation;

		bool IsUsing3dCam;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};

}