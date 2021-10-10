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

#pragma once

#include "Saturn/Core/Timestep.h"
#include "Saturn/Core/Input.h"
#include "Saturn/Core/Events.h"
#include "Camera.h"

namespace Saturn {

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera( float fov, float aspectRatio, float nearClip, float farClip );
		EditorCamera( const glm::mat4& projectionMatrix );

		~EditorCamera() = default;

		void OnUpdate( Timestep ts );
	public:

		inline float Distance() const { return m_Distance; }
		inline void SetDistance( float distance ) { m_Distance = distance; }

		inline void SetViewportSize( float width, float height ) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const glm::mat4& ViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 ViewProjection() const { return m_Projection * m_ViewMatrix; }

		glm::vec3 UpDirection() const;
		glm::vec3 RightDirection() const;
		glm::vec3 ForwardDirection() const;
		const glm::vec3& Position() const { return m_Position; }
		glm::quat Orientation() const;

		float Pitch() const { return m_Pitch; }
		float Yaw() const { return m_Yaw; }

		void SetProjectionMatrix( const glm::mat4& projectionMatrix ) { m_Projection = projectionMatrix; }

	public:
		// Events

		void OnEvent( Event& e );
		bool OnMouseScroll( MouseScrolledEvent& e );
	private:

		void UpdateProjection();
		void UpdateView();

		void MousePan( const glm::vec2& delta );
		void MouseRotate( const glm::vec2& delta );
		void MouseZoom( float delta );

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:

		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position ={ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint ={ 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition ={ 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};
}