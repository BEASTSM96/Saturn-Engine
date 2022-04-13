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

#include "Saturn/Core/Timestep.h"
#include "Saturn/Core/Input.h"
#include "Saturn/Core/Events.h"
#include "Camera.h"

namespace Saturn {


	enum class CameraMode
	{
		NONE, FLYCAM, ARCBALL
	};

	class EditorCamera : public Camera
	{
	public:

		EditorCamera() = default;
		EditorCamera( const glm::mat4& projectionMatrix );

		void Focus( const glm::vec3& focusPoint );
		void OnUpdate( Timestep ts );
		void OnEvent( Event& e );

		bool Active() const { return m_IsActive; }
		void SetActive( bool active ) { m_IsActive = active; }

		inline float Distance() const { return m_Distance; }
		inline void SetDistance( float distance ) { m_Distance = distance; }

		const glm::vec3& FocalPoint() const { return m_FocalPoint; }

		inline void SetViewportSize( uint32_t width, uint32_t height ) { m_ViewportWidth = width; m_ViewportHeight = height; }

		const glm::mat4& ViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 ViewProjection() const { return m_Projection * m_ViewMatrix; }

		glm::vec3 UpDirection() const;
		glm::vec3 RightDirection() const;
		glm::vec3 ForwardDirection() const;

		const glm::vec3& Position() const { return m_Position; }

		glm::quat Orientation() const;

		float Pitch() const { return m_Pitch; }
		float Yaw() const { return m_Yaw; }
		float& CameraSpeed() { return m_Speed; }
		float CameraSpeed() const { return m_Speed; }

	private:

		void UpdateCameraView();

		bool OnMouseScroll( MouseScrolledEvent& e );
		bool OnKeyPressed( KeyPressedEvent& e );
		bool OnKeyReleased( KeyReleasedEvent& e );

		void MousePan( const glm::vec2& delta );
		void MouseRotate( const glm::vec2& delta );
		void MouseZoom( float delta );

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position, m_WorldRotation, m_FocalPoint;

		bool m_IsActive = false;
		bool m_Panning, m_Rotating;
		glm::vec2 m_InitialMousePosition{};
		glm::vec3 m_InitialFocalPoint, m_InitialRotation;

		float m_Distance;
		float m_Speed{ 0.002f };
		float m_LastSpeed = 0.f;

		float m_Pitch, m_Yaw;
		float m_PitchDelta{}, m_YawDelta{};
		glm::vec3 m_PositionDelta{};
		glm::vec3 m_RightDirection{};

		CameraMode m_CameraMode{ CameraMode::ARCBALL };

		float m_MinFocusDistance = 100.0f;

		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

	private:

		friend class EditorLayer;
	};
	
#endif
}