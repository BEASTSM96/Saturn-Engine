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

#include "sppch.h"
#include "EditorCamera.h"

#include "Saturn/Core/Input.h"

#include <imgui.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#define M_PI 3.14159f

namespace Saturn {

	EditorCamera::EditorCamera( const glm::mat4& projectionMatrix )
		: Camera( projectionMatrix )
	{
	#if 1
		
		m_Yaw = 3.0f * ( float )M_PI / 4.0f;
		m_Pitch = M_PI / 4.0f;

		const glm::quat orientation = Orientation();
		m_WorldRotation = glm::eulerAngles( orientation ) * ( 180.0f / ( float )M_PI );

		glm::vec3 Translation = glm::vec3( 5.0f, 5.0f, 5.0f );
		m_Distance = glm::distance( Translation, glm::vec3( 0.0f, 0.0f, 0.0f ) );
		m_Position = CalculatePosition();
		
		if( m_FlipY )
			m_Position.y * -1.0f;

		glm::mat4 RotationM = glm::mat4( 1.0f );
		glm::mat4 TranslationM = glm::mat4( 1.0f );
		
		RotationM = glm::rotate( RotationM, glm::radians( m_WorldRotation.x * m_FlipY ? -1.0f : 1.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
		RotationM = glm::rotate( RotationM, glm::radians( m_WorldRotation.y ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
		RotationM = glm::rotate( RotationM, glm::radians( m_WorldRotation.z ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

		TranslationM = glm::translate( glm::mat4( 1.f ), Translation );
		
		m_ViewMatrix = TranslationM * RotationM;

	#else
		m_FocalPoint = glm::vec3( 0.0f );

		glm::vec3 position ={ 5, -1.0f, 5 };
		m_Distance = glm::distance( position, m_FocalPoint );

		m_Yaw = 3.0f * ( float )M_PI / 4.0f;
		m_Pitch = M_PI / 4.0f;

		m_Position = CalculatePosition();

		if( m_FlipY )
			m_Position.y *= -1.0f;

		const glm::quat orientation = Orientation();
		m_WorldRotation = glm::eulerAngles( orientation ) * ( 180.0f / ( float )M_PI );

		m_ViewMatrix = glm::translate( glm::mat4( 1.0f ), m_Position ) * glm::toMat4( orientation );
		m_ViewMatrix = glm::inverse( m_ViewMatrix );

		if( m_FlipY )
		{
			m_ViewMatrix[ 1 ][ 1 ] *= -1;
			//m_Projection[ 1 ][ 1 ] *= -1;
		}
	#endif
	}

	static void SetMouseEnabled( const bool enable )
	{
		if( enable )
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		else
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

	static void DisableMouse()
	{
		Input::Get().SetCursorMode( CursorMode::Locked );
		SetMouseEnabled( false );
	}

	static void EnableMouse()
	{
		Input::Get().SetCursorMode( CursorMode::Normal );
		SetMouseEnabled( true );
	}

	void EditorCamera::OnUpdate( const Timestep ts )
	{
		const glm::vec2& mouse{ Input::Get().MouseX(), Input::Get().MouseY() };
		const glm::vec2 delta = ( mouse - m_InitialMousePosition ) * 0.002f;

		if( m_IsActive )
		{
			if( Input::Get().MouseButtonPressed( Mouse::Right ) && !Input::Get().KeyPressed( Key::LeftAlt ) )
			{
				m_CameraMode = CameraMode::FLYCAM;
				DisableMouse();
				const float yawSign = UpDirection().y < 0 ? -1.0f : 1.0f;

				if( Input::Get().KeyPressed( Key::Q ) )
					m_PositionDelta -= ts.Milliseconds() * m_Speed * glm::vec3{ 0.f, yawSign, 0.f };
				if( Input::Get().KeyPressed( Key::E ) )
					m_PositionDelta += ts.Milliseconds() * m_Speed * glm::vec3{ 0.f, yawSign, 0.f };

				if( Input::Get().KeyPressed( Key::S ) )
					m_PositionDelta -= ts.Milliseconds() * m_Speed * m_WorldRotation;
				if( Input::Get().KeyPressed( Key::W ) )
					m_PositionDelta += ts.Milliseconds() * m_Speed * m_WorldRotation;

				if( Input::Get().KeyPressed( Key::A ) )
					m_PositionDelta -= ts.Milliseconds() * m_Speed * m_RightDirection;
				
				if( Input::Get().KeyPressed( Key::D ) )
					m_PositionDelta += ts.Milliseconds() * m_Speed * m_RightDirection;

				constexpr float maxRate{ 0.12f };
				m_YawDelta += glm::clamp( yawSign * delta.x, -maxRate, maxRate );
				m_PitchDelta += glm::clamp( delta.y, -maxRate, maxRate );

				m_RightDirection = glm::cross( m_WorldRotation, glm::vec3{ 0.f, yawSign, 0.f } );

				m_WorldRotation = glm::rotate( glm::normalize( glm::cross( glm::angleAxis( m_PitchDelta, m_RightDirection ),
					glm::angleAxis( -m_YawDelta, glm::vec3{ 0.f, yawSign, 0.f } ) ) ), m_WorldRotation );
			}
			else if( Input::Get().KeyPressed( Key::LeftAlt ) )
			{
				m_CameraMode = CameraMode::ARCBALL;


				if( Input::Get().MouseButtonPressed( Mouse::Middle ) )
				{
					DisableMouse();
					MousePan( delta );
				}
				else if( Input::Get().MouseButtonPressed( Mouse::Left ) )
				{
					DisableMouse();
					MouseRotate( delta );
				}
				else if( Input::Get().MouseButtonPressed( Mouse::Right ) )
				{
					DisableMouse();
					MouseZoom( delta.x + delta.y );
				}
				else
					EnableMouse();
			}
			else
			{
				EnableMouse();
			}
		}
		m_InitialMousePosition = mouse;

		m_Position += m_PositionDelta;

		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		if( m_CameraMode == CameraMode::ARCBALL )
			m_Position = CalculatePosition();

		UpdateCameraView();
	}

	void EditorCamera::UpdateCameraView()
	{
		const float yawSign = UpDirection().y < 0 ? -1.0f : 1.0f;

		// Extra step to handle the problem when the camera direction is the same as the up vector
		const float cosAngle = glm::dot( ForwardDirection(), UpDirection() );
		if( cosAngle * yawSign > 0.99f )
			m_PitchDelta = 0.f;

		const glm::vec3 lookAt = m_Position + ForwardDirection();
		m_WorldRotation = glm::normalize( m_FocalPoint - m_Position );
		m_FocalPoint = m_Position + ForwardDirection() * m_Distance;
		m_Distance = glm::distance( m_Position, m_FocalPoint );
		m_ViewMatrix = glm::lookAt( m_Position, lookAt, glm::vec3{ 0.f, m_FlipY ? -yawSign : yawSign, 0.f } );
		
		// print view matrix
		std::cout << m_ViewMatrix[ 0 ][ 0 ] << " " << m_ViewMatrix[ 0 ][ 1 ] << " " << m_ViewMatrix[ 0 ][ 2 ] << " " << m_ViewMatrix[ 0 ][ 3 ] << std::endl;
		std::cout << m_ViewMatrix[ 1 ][ 0 ] << " " << m_ViewMatrix[ 1 ][ 1 ] << " " << m_ViewMatrix[ 1 ][ 2 ] << " " << m_ViewMatrix[ 1 ][ 3 ] << std::endl;
		std::cout << m_ViewMatrix[ 2 ][ 0 ] << " " << m_ViewMatrix[ 2 ][ 1 ] << " " << m_ViewMatrix[ 2 ][ 2 ] << " " << m_ViewMatrix[ 2 ][ 3 ] << std::endl;
		std::cout << m_ViewMatrix[ 3 ][ 0 ] << " " << m_ViewMatrix[ 3 ][ 1 ] << " " << m_ViewMatrix[ 3 ][ 2 ] << " " << m_ViewMatrix[ 3 ][ 3 ] << std::endl;
		std::cout << std::endl;
			
		//damping for smooth camera
		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	void EditorCamera::Focus( const glm::vec3& focusPoint )
	{
		m_FocalPoint = focusPoint;
		if( m_Distance > m_MinFocusDistance )
		{
			const float distance = m_Distance - m_MinFocusDistance;
			MouseZoom( distance / ZoomSpeed() );
			m_CameraMode = CameraMode::ARCBALL;
		}
		m_Position = m_FocalPoint - ForwardDirection() * m_Distance;
		UpdateCameraView();
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		const float x = std::min( float( m_ViewportWidth ) / 1000.0f, 2.4f ); // max = 2.4f
		const float xFactor = 0.0366f * ( x * x ) - 0.1778f * x + 0.3021f;

		const float y = std::min( float( m_ViewportHeight ) / 1000.0f, 2.4f ); // max = 2.4f
		const float yFactor = 0.0366f * ( y * y ) - 0.1778f * y + 0.3021f;

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

	void EditorCamera::OnEvent( Event& event )
	{
		EventDispatcher dispatcher( event );
		dispatcher.Dispatch<MouseScrolledEvent>( [this]( MouseScrolledEvent& e ) { return OnMouseScroll( e ); } );
		dispatcher.Dispatch<KeyReleasedEvent>( [this]( KeyReleasedEvent& e ) { return OnKeyReleased( e ); } );
		dispatcher.Dispatch<KeyPressedEvent>( [this]( KeyPressedEvent& e ) { return OnKeyPressed( e ); } );
	}

	bool EditorCamera::OnMouseScroll( MouseScrolledEvent& e )
	{
		if( m_IsActive )
		{
			if( Input::Get().MouseButtonPressed( Mouse::Right ) )
			{
				e.YOffset() > 0 ? m_Speed += 0.3f * m_Speed : m_Speed -= 0.3f * m_Speed;
				m_Speed = std::clamp( m_Speed, 0.0005f, 2.f );
			}
			else
			{
				MouseZoom( e.YOffset() * 0.1f );
				UpdateCameraView();
			}
		}

		return false;
	}

	bool EditorCamera::OnKeyPressed( KeyPressedEvent& e )
	{
		if( m_LastSpeed == 0.0f )
		{
			if( e.KeyCode() == Key::LeftShift )
			{
				m_LastSpeed = m_Speed;
				m_Speed *= 2.0f - glm::log( m_Speed );
			}
			if( e.KeyCode() == Key::LeftControl )
			{
				m_LastSpeed = m_Speed;
				m_Speed /= 2.0f - glm::log( m_Speed );
			}

			m_Speed = glm::clamp( m_Speed, 0.0005f, 2.0f );
		}
		return true;
	}

	bool EditorCamera::OnKeyReleased( KeyReleasedEvent& e )
	{
		if( e.KeyCode() == Key::LeftShift || e.KeyCode() == Key::LeftControl )
		{
			if( m_LastSpeed != 0.0f )
			{
				m_Speed = m_LastSpeed;
				m_LastSpeed = 0.0f;
			}
			m_Speed = glm::clamp( m_Speed, 0.0005f, 2.0f );
		}
		return true;
	}

	void EditorCamera::MousePan( const glm::vec2& delta )
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -RightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += UpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate( const glm::vec2& delta )
	{
		const float yawSign = UpDirection().y < 0.0f ? -1.0f : 1.0f;
		m_YawDelta += yawSign * delta.x * RotationSpeed();
		m_PitchDelta += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom( float delta )
	{
		m_Distance -= delta * ZoomSpeed();
		m_Position = m_FocalPoint - ForwardDirection() * m_Distance;
		const glm::vec3 forwardDir = ForwardDirection();
		if( m_Distance < 1.0f )
		{
			m_FocalPoint += forwardDir;
			m_Distance = 1.0f;
		}
		m_PositionDelta += delta * ZoomSpeed() * forwardDir;
	}

	glm::vec3 EditorCamera::UpDirection() const
	{
		if( m_FlipY )
			return glm::rotate( Orientation(), glm::vec3( 0.0f, -1.0f, 0.0f ) );
		else
			return glm::rotate( Orientation(), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	}

	glm::vec3 EditorCamera::RightDirection() const
	{
		return glm::rotate( Orientation(), glm::vec3( 1.f, 0.f, 0.f ) );
	}

	glm::vec3 EditorCamera::ForwardDirection() const
	{
		return glm::rotate( Orientation(), glm::vec3( 0.0f, 0.0f, -1.0f ) );
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - ForwardDirection() * m_Distance + m_PositionDelta;
	}

	glm::quat EditorCamera::Orientation() const
	{
		return glm::quat( glm::vec3( -m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f ) );
	}
}