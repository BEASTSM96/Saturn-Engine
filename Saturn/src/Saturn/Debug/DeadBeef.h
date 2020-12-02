#pragma once

#include "Saturn/Core/Base.h"

namespace Saturn {
	namespace deadbeef {
		void Throw( void ) 
		{
			//just throw it already
			int val = 0xDEADBEEF;

			//gee this is dumb
			SAT_CORE_ASSERT(!val == 0xDEADBEEF, "Value was 0xDEADBEEF!");
		}
	}
}