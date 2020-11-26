#pragma once

#include "Saturn/Core/Base.h"

namespace Saturn {

	class SATURN_API GraphicsContext {

	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}