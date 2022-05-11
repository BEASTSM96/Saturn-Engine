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
		
		void Flip( bool flip ) {  m_FlipY = flip; }

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

		//EditorCamera& operator=( const EditorCamera& other ) = default;
		
		/*
		// Copy assignment.
		EditorCamera& operator=( const EditorCamera& other ) 
		{
			m_Position = other.m_Position;
			m_WorldRotation = other.m_WorldRotation;
			m_FocalPoint = other.m_FocalPoint;
			m_Pitch = other.m_Pitch;
			m_Yaw = other.m_Yaw;
			m_Distance = other.m_Distance;
			m_Speed = other.m_Speed;
			m_IsActive = other.m_IsActive;
			m_ViewportWidth = other.m_ViewportWidth;
			m_ViewportHeight = other.m_ViewportHeight;

			m_Panning = other.m_Panning;

			m_ViewMatrix = other.m_ViewMatrix;
			m_Projection = other.m_Projection;
			
			m_CameraMode = other.m_CameraMode;
			
			m_PositionDelta = other.m_PositionDelta;
			m_PitchDelta = other.m_PitchDelta;
			m_YawDelta = other.m_YawDelta;

			m_RightDirection = other.m_RightDirection;

			m_FlipY = other.m_FlipY;

			m_MinFocusDistance = other.m_MinFocusDistance;

			m_LastSpeed = other.m_LastSpeed;

			m_InitialFocalPoint = other.m_InitialFocalPoint;
			m_InitialRotation = other.m_InitialRotation;

			return *this;
		}
		
		// Move assignment.
		EditorCamera& operator=( EditorCamera&& other ) 
		{
			m_Position = std::move( other.m_Position );
			m_WorldRotation = std::move( other.m_WorldRotation );
			m_FocalPoint = std::move( other.m_FocalPoint );
			m_Pitch = std::move( other.m_Pitch );
			m_Yaw = std::move( other.m_Yaw );
			m_Distance = std::move( other.m_Distance );
			m_Speed = std::move( other.m_Speed );
			m_IsActive = std::move( other.m_IsActive );
			m_ViewportWidth = std::move( other.m_ViewportWidth );
			m_ViewportHeight = std::move( other.m_ViewportHeight );
			
			m_Panning = std::move( other.m_Panning );
			
			m_ViewMatrix = std::move( other.m_ViewMatrix );
			
			m_Projection = std::move( other.m_Projection );

			m_CameraMode = std::move( other.m_CameraMode );
			
			m_PositionDelta = std::move( other.m_PositionDelta );
			
			m_PitchDelta = std::move( other.m_PitchDelta );

			m_YawDelta = std::move( other.m_YawDelta );
			
			m_RightDirection = std::move( other.m_RightDirection );

			m_FlipY = std::move( other.m_FlipY );
			
			m_MinFocusDistance = std::move( other.m_MinFocusDistance );

			m_LastSpeed = std::move( other.m_LastSpeed );

			m_InitialFocalPoint = std::move( other.m_InitialFocalPoint );
			m_InitialRotation = std::move( other.m_InitialRotation );
			
			return *this;
		}
		*/
		
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

		glm::vec4 m_ViewPos;

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

		bool m_FlipY = true;

		CameraMode m_CameraMode{ CameraMode::ARCBALL };

		float m_MinFocusDistance = 100.0f;

		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

	private:

		friend class EditorLayer;
	};
}