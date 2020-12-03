#include "sppch.h"
#include "Camera.h"

namespace Saturn {
	Camera::Camera( const glm::mat4& projectionMatrix ) : m_ProjectionMatrix(projectionMatrix)
	{
	}
}