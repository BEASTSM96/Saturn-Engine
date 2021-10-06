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
#include "EditorCamera.h"

#include "Saturn/Core/Input.h"

#include <imgui.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Saturn {

	// HACK: Fix static and input events
	EditorCamera* s_EditorCamera = nullptr;

	EditorCamera::EditorCamera( float fov, float aspectRatio, float nearClip, float farClip )
		: m_FOV( fov ), m_AspectRatio( aspectRatio ), m_NearClip( nearClip ), m_FarClip( farClip ), Camera( glm::perspective( glm::radians( fov ), aspectRatio, nearClip, farClip ) )
	{
		s_EditorCamera = this;

		std::function<void()> func = OnMouseScroll;
		Input::Subscribe( func );

		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective( glm::radians( m_FOV ), m_AspectRatio, m_NearClip, m_FarClip );
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		glm::quat orientation = Orientation();
		m_ViewMatrix = glm::translate( glm::mat4( 1.0f ), m_Position ) * glm::toMat4( orientation );
		m_ViewMatrix = glm::inverse( m_ViewMatrix );
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min( m_ViewportWidth / 1000.0f, 2.4f ); // max = 2.4f
		float xFactor = 0.0366f * ( x * x ) - 0.1778f * x + 0.3021f;

		float y = std::min( m_ViewportHeight / 1000.0f, 2.4f ); // max = 2.4f
		float yFactor = 0.0366f * ( y * y ) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max( distance, 0.0f );
		float speed = distance * distance;
		speed = std::min( speed, 100.0f ); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate( Timestep ts )
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_LEFT_ALT ) )
		{
			const glm::vec2& mouse{ Input::MouseX(), Input::MouseY() };
			glm::vec2 delta = ( mouse - m_InitialMousePosition ) * 0.003f;
			m_InitialMousePosition = mouse;

			if( ImGui::IsMouseClicked( ImGuiMouseButton_Middle ) )
				MousePan( delta );
			else if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
				MouseRotate( delta );
			else if( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
				MouseZoom( delta.y );
		}

		UpdateView();
	}

	void EditorCamera::OnMouseScroll()
	{
		float delta = Input::MouseYOffset() * 0.1f;
		s_EditorCamera->MouseZoom( delta );
		s_EditorCamera->UpdateView();
	}

	void EditorCamera::MousePan( const glm::vec2& delta )
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -RightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += UpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate( const glm::vec2& delta )
	{
		float yawSign = UpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom( float delta )
	{
		m_Distance -= delta * ZoomSpeed();
		if( m_Distance < 1.0f )
		{
			m_FocalPoint += ForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::UpDirection() const
	{
		return glm::rotate( Orientation(), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	}

	glm::vec3 EditorCamera::RightDirection() const
	{
		return glm::rotate( Orientation(), glm::vec3( 1.0f, 0.0f, 0.0f ) );
	}

	glm::vec3 EditorCamera::ForwardDirection() const
	{
		return glm::rotate( Orientation(), glm::vec3( 0.0f, 0.0f, -1.0f ) );
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - ForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::Orientation() const
	{
		return glm::quat( glm::vec3( -m_Pitch, -m_Yaw, 0.0f ) );
	}
}