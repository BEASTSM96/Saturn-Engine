#pragma once

#ifdef SAT_PLATFORM_WINDOWS
	#include <Windows.h>
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <memory>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <fstream>

#include <Saturn/Core/Base.h>
#include <Saturn/Log.h>
#include <Saturn/Debug/Instrumentor.h>
