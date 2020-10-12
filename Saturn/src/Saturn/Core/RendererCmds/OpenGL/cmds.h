#pragma once 

#include <glad/glad.h>
#include "Saturn/Renderer/Renderer.h"

namespace Saturn::Core {

	namespace OpenGLCommands {

		enum Depth
		{
			None = 0xDFFFF,
			On = 0xD1,
			Off = 0xD2
		};
	}
}