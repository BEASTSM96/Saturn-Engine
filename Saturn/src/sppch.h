#pragma once

#pragma warning(push, 0)

#ifdef SAT_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "Saturn/Log.h"
#include "Saturn/Core/Math/Math.h"
#include "Saturn/Debug/Instrumentor.h"

#ifdef SAT_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // SAT_PLATFORM_WINDOWS
#pragma warning(pop)