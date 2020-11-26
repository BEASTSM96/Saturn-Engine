#pragma once

#include "Saturn/Core/Base.h"

namespace Saturn {

	classGraphicsContext {

	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}