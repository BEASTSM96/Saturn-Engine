/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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

#include "Saturn/Input.h"

#include <glfw/glfw3.h>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define M_PI 3.14159f

namespace Saturn {

	EditorCamera::EditorCamera( const glm::mat4& projectionMatrix )
	{
		m_Rotation = glm::vec3( 90.0f, 0.0f, 0.0f );
		m_FocalPoint = glm::vec3( 0.0f );

		glm::vec3 position ={ -5, 5, 5 };
		m_Distance = glm::distance( position, m_FocalPoint );

		m_Yaw = 3.0f * ( float )M_PI / 4.0f;
		m_Pitch = M_PI / 4.0f;

		UpdateCameraView();
	}

	void EditorCamera::UpdateCameraView()
	{
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_Rotation = glm::eulerAngles( orientation ) * ( 180.0f / ( float )M_PI );
		m_ViewMatrix = glm::translate( glm::mat4( 1.0f ), m_Position ) * glm::toMat4( orientation );
		m_ViewMatrix = glm::inverse( m_ViewMatrix );
	}

	void EditorCamera::Focus()
	{
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

		const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		glm::vec2 delta = ( mouse - m_InitialMousePosition ) * 0.003f;
		m_InitialMousePosition = mouse;

		if( Input::IsMouseButtonPressed( GLFW_MOUSE_BUTTON_MIDDLE ) )
			MousePan( delta );
		else if( Input::IsMouseButtonPressed( SAT_MOUSE_BUTTON_RIGHT ) )
			MouseRotate( delta );
		else if( Input::IsKeyPressed( GLFW_KEY_LEFT_ALT ) )
			MouseZoom( delta.y );

		UpdateCameraView();
	}

	void EditorCamera::OnEvent( Event& e )
	{
		EventDispatcher dispatcher( e );
		dispatcher.Dispatch<MouseScrolledEvent>( SAT_BIND_EVENT_FN( EditorCamera::OnMouseScroll ) );
	}

	bool EditorCamera::OnMouseScroll( MouseScrolledEvent& e )
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom( delta );
		UpdateCameraView();
		return false;
	}

	void EditorCamera::MousePan( const glm::vec2& delta )
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate( const glm::vec2& delta )
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom( float delta )
	{
		m_Distance -= delta * ZoomSpeed();
		if( m_Distance < 1.0f )
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::GetUpDirection()
	{
		return glm::rotate( GetOrientation(), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	}

	glm::vec3 EditorCamera::GetRightDirection()
	{
		return glm::rotate( GetOrientation(), glm::vec3( 1.0f, 0.0f, 0.0f ) );
	}

	glm::vec3 EditorCamera::GetForwardDirection()
	{
		return glm::rotate( GetOrientation(), glm::vec3( 0.0f, 0.0f, -1.0f ) );
	}

	glm::vec3 EditorCamera::CalculatePosition()
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat( glm::vec3( -m_Pitch, -m_Yaw, 0.0f ) );
	}

}