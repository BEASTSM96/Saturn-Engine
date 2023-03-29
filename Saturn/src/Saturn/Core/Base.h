/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#pragma once

#define SAT_DONT_USE_GL
#define SAT_DONT_USE_DX

// Short Macros
#if defined ( SAT_PLATFORM_WINDOWS )
#define SAT_WINDOWS 1
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined ( SAT_PLATFORM_LINUX )
#define SAT_LINUX 1
#define GLFW_EXPOSE_NATIVE_X11
#include <signal.h>
#else
#define SAT_MAC 1
#endif 

#define GLFW_RESIZE_NESW_CURSOR 0x00036008

#define SAT_ARRAYSIZE( x ) ( ( int ) ( sizeof( x ) / sizeof( *( x ) ) ) )

// Line Ending for shaders

#define __CR_LF__ "\r\n"
#define _LF__ "\n"
#define _CR__ "\r"

#define SAT_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

// Lazy load... TODO: Change this.
#define SINGLETON( x )                    \
public:                                   \
static x& Get() { static x _; return _; } \
x( const x& ) = delete;                   \
x( x&& ) = delete;                        \
x& operator=( x&& ) = delete;             \
x& operator=( const x& ) = delete

#define BIT( x ) (1 << x)

#define SAT_CURRENT_VERISON 0.1.0
#define SAT_CURRENT_VERISON_STRING "0.1.0"
#define SAT_FULL_BUILD_PATH "/saturn/build/++alpha/mvp/++0.1"

namespace Saturn::Core {

	inline void BreakDebug()
	{
	#if defined( _WIN32 )
		__debugbreak();
	#else
		raise( SIGTRAP );
	#endif // _MSC_VER
	}

}

const int MAX_FRAMES_IN_FLIGHT = 3;

// Inject asserts
#define __CORE_INCLUDED__
#include "Asserts.h"
// Common includes
#include "Timestep.h"
#include "Ref.h"
#include "SingletonStorage.h"