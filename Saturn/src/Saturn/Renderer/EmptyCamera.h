#pragma once

#include <glm/glm.hpp>

#include <glad/glad.h>

#include "Saturn/Core/Serialisation/Object.h"
#include "Saturn/Core/Serialisation/Serialiser.h"

namespace Saturn {

    //Just to clean to up
    //This is dumb
    using openglbool = GLboolean;

    class EmptyCamera
    {
    public:
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        void SetProjectionMatrix(const glm::mat4& projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }

    protected:
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
	};

}