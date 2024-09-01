/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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
#include "SceneFlyCamera.h"

#include "Saturn/Core/Input.h"
#include "Saturn/Core/Ruby/RubyCore.h"

#include <glm/glm.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

constexpr double M_PI = 3.14159265358979323846;

namespace Saturn {

	SceneFlyCamera::SceneFlyCamera( const float Fov, const float Width, const float Height, const float NearPlane, const float FarPlane )
		: Camera( Fov, Width, Height, NearPlane, FarPlane ), m_FocalPoint( 0.0f )
	{
		constexpr glm::vec3 position{ -5.0f, 5.0f, 5.0f };
		m_Distance = glm::distance( m_Position, m_FocalPoint );

		m_Yaw = 3.0f * ( float ) M_PI / 4.0f;
		m_Pitch = ( float ) M_PI / 4.0f;

		m_Position = CalculatePosition();
		const glm::quat orientation = GetOrientation();

		m_Rotation = glm::eulerAngles( orientation ) * ( 180.0f / ( float ) M_PI );
		m_ViewMatrix = glm::translate( glm::mat4( 1.0f ), m_Position ) * glm::toMat4( orientation );
		m_ViewMatrix = glm::inverse( m_ViewMatrix );
	}

	static void DisableMouse()
	{
		Input::Get().SetCursorMode( RubyCursorMode::Locked );
	}

	static void EnableMouse()
	{
		Input::Get().SetCursorMode( RubyCursorMode::Normal );
	}

	void SceneFlyCamera::OnUpdate( Timestep ts )
	{
		const glm::vec2& mouse{ Input::Get().MouseX(), Input::Get().MouseY() };
		const glm::vec2 delta = ( mouse - m_InitialMousePosition ) * 0.002f;

		if( !Input::Get().KeyPressed( RubyKey::Esc ) )
		{
			DisableMouse();
			const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

			const float speed = GetCameraSpeed();

			if( Input::Get().KeyPressed( RubyKey::Q ) )
				m_PositionDelta -= ts.Milliseconds() * speed * glm::vec3{ 0.f, yawSign, 0.f };
			if( Input::Get().KeyPressed( RubyKey::E ) )
				m_PositionDelta += ts.Milliseconds() * speed * glm::vec3{ 0.f, yawSign, 0.f };
			if( Input::Get().KeyPressed( RubyKey::S ) )
				m_PositionDelta -= ts.Milliseconds() * speed * m_Rotation;
			if( Input::Get().KeyPressed( RubyKey::W ) )
				m_PositionDelta += ts.Milliseconds() * speed * m_Rotation;
			if( Input::Get().KeyPressed( RubyKey::A ) )
				m_PositionDelta -= ts.Milliseconds() * speed * m_RightDirection;
			if( Input::Get().KeyPressed( RubyKey::D ) )
				m_PositionDelta += ts.Milliseconds() * speed * m_RightDirection;

			constexpr float maxRate{ 0.12f };
			m_YawDelta += glm::clamp( yawSign * delta.x, -maxRate, maxRate );
			m_PitchDelta += glm::clamp( delta.y, -maxRate, maxRate );

			m_RightDirection = glm::cross( m_Rotation, glm::vec3{ 0.f, yawSign, 0.f } );

			m_Rotation = glm::rotate( glm::normalize( glm::cross( glm::angleAxis( -m_PitchDelta, m_RightDirection ),
				glm::angleAxis( -m_YawDelta, glm::vec3{ 0.f, yawSign, 0.f } ) ) ), m_Rotation );

			const float distance = glm::distance( m_FocalPoint, m_Position );
			m_FocalPoint = m_Position + GetForwardDirection() * distance;
			m_Distance = distance;
		}
		else
		{
			EnableMouse();
		}

		m_InitialMousePosition = mouse;

		m_Position += m_PositionDelta;
		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		UpdateCameraView();
	}

	float SceneFlyCamera::GetCameraSpeed() const
	{
		float speed = m_NormalSpeed;
		if( Input::Get().KeyPressed( RubyKey::Ctrl ) )
			speed /= 2 - glm::log( m_NormalSpeed );
		if( Input::Get().KeyPressed( RubyKey::Shift ) )
			speed *= 2 - glm::log( m_NormalSpeed );

		return glm::clamp( speed, MIN_SPEED, MAX_SPEED );
	}

	void SceneFlyCamera::UpdateCameraView()
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

		// Extra step to handle the problem when the camera direction is the same as the up vector
		const float cosAngle = glm::dot( GetForwardDirection(), GetUpDirection() );
		if( cosAngle * yawSign > 0.99f )
			m_PitchDelta = 0.f;

		const glm::vec3 lookAt = m_Position + GetForwardDirection();
		m_Rotation = glm::normalize( lookAt - m_Position );
		m_Distance = glm::distance( m_Position, m_FocalPoint );
		m_ViewMatrix = glm::lookAt( m_Position, lookAt, glm::vec3{ 0.f, yawSign, 0.f } );

		//damping for smooth camera
		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	void SceneFlyCamera::OnEvent( RubyEvent& e )
	{
		if( e.Type == RubyEventType::MouseScroll )
			OnMouseScroll( ( RubyMouseScrollEvent& ) e );
	}

	bool SceneFlyCamera::OnMouseScroll( RubyMouseScrollEvent& e )
	{
		if( Input::Get().MouseButtonPressed( RubyMouseButton::Right ) )
		{
			m_NormalSpeed += e.GetOffsetY() * 0.3f * m_NormalSpeed;
			m_NormalSpeed = std::clamp( m_NormalSpeed, MIN_SPEED, MAX_SPEED );
		}
		else
		{
			MouseZoom( e.GetOffsetY() * 0.1f );
			UpdateCameraView();
		}

		return true;
	}

	glm::vec3 SceneFlyCamera::GetUpDirection() const
	{
		return glm::rotate( GetOrientation(), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	}

	glm::vec3 SceneFlyCamera::GetRightDirection() const
	{
		return glm::rotate( GetOrientation(), glm::vec3( 1.f, 0.f, 0.f ) );
	}

	glm::vec3 SceneFlyCamera::GetForwardDirection() const
	{
		return glm::rotate( GetOrientation(), glm::vec3( 0.0f, 0.0f, -1.0f ) );
	}

	glm::quat SceneFlyCamera::GetOrientation() const
	{
		return glm::quat( glm::vec3( -m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f ) );
	}

	void SceneFlyCamera::MousePan( const glm::vec2& delta )
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint -= GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void SceneFlyCamera::MouseRotate( const glm::vec2& delta )
	{
		const float yawSign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
		m_YawDelta += yawSign * delta.x * RotationSpeed();
		m_PitchDelta += delta.y * RotationSpeed();
	}

	void SceneFlyCamera::MouseZoom( float delta )
	{
		m_Distance -= delta * ZoomSpeed();
		const glm::vec3 forwardDir = GetForwardDirection();
		m_Position = m_FocalPoint - forwardDir * m_Distance;
		if( m_Distance < 1.0f )
		{
			m_FocalPoint += forwardDir * m_Distance;
			m_Distance = 1.0f;
		}
		m_PositionDelta += delta * ZoomSpeed() * forwardDir;
	}

	glm::vec3 SceneFlyCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance + m_PositionDelta;
	}

	std::pair<float, float> SceneFlyCamera::PanSpeed() const
	{
		const float x = glm::min( float( m_ViewportWidth ) / 1000.0f, 2.4f ); // max = 2.4f
		const float xFactor = 0.0366f * ( x * x ) - 0.1778f * x + 0.3021f;

		const float y = glm::min( float( m_ViewportHeight ) / 1000.0f, 2.4f ); // max = 2.4f
		const float yFactor = 0.0366f * ( y * y ) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float SceneFlyCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float SceneFlyCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = glm::max( distance, 0.0f );
		float speed = distance * distance;
		speed = glm::min( speed, 50.0f ); // max speed = 50
		return speed;
	}

}