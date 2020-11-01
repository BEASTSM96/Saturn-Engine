#pragma once

#include <glm/glm.hpp>

#include <glad/glad.h>

#include "Saturn/Core/Serialisation/Object.h"
#include "Saturn/Core/Serialisation/Serialiser.h"

namespace Saturn {

    class SATURN_API EmptyCamera
    {
    public:
        //Just to clean to up
        //This is dumb
        using openglbool = GLboolean;
	};

}