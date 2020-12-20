#pragma once

#include "Saturn/Renderer/Camera.h"
#include "Saturn/Core/Timestep.h"
#include "Saturn/Events/MouseEvent.h"

namespace Saturn {

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera( const glm::mat4& projectionMatrix );

		void Focus( void );
		void OnUpdate( Timestep ts );
		void OnEvent( Event& e );

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance( float distance ) { m_Distance = distance; }

		inline void SetViewportSize( uint32_t width, uint32_t height ) { m_ViewportWidth = width; m_ViewportHeight = height; }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

		glm::vec3 GetUpDirection( void );
		glm::vec3 GetRightDirection( void );
		glm::vec3 GetForwardDirection( void );
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation( void ) const;

		float GetExposure( void ) const { return m_Exposure; }
		float& GetExposure( void ) { return m_Exposure; }

		float GetPitch( void ) const { return m_Pitch; }
		float GetYaw( void ) const { return m_Yaw; }
	private:
		void UpdateCameraView( void );

		bool OnMouseScroll( MouseScrolledEvent& e );

		void MousePan( const glm::vec2& delta );
		void MouseRotate( const glm::vec2& delta );
		void MouseZoom( float delta );

		glm::vec3 CalculatePosition( void );

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed( void ) const;
		float ZoomSpeed( void ) const;
	private:
		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position, m_Rotation, m_FocalPoint;

		bool m_Panning, m_Rotating;
		glm::vec2 m_InitialMousePosition;
		glm::vec3 m_InitialFocalPoint, m_InitialRotation;

		float m_Distance;
		float m_Pitch, m_Yaw;

		float m_Exposure = 0.8f;

		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};

}