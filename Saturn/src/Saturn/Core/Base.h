/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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
#elif defined ( SAT_PLATFORM_LINUX )
#define SAT_LINUX 1
#include <signal.h>
#else
#define SAT_MAC 1
#endif 

#define SAT_ARRAYSIZE( x ) ( ( int ) ( sizeof( x ) / sizeof( *( x ) ) ) )

// Line Ending for shaders

#define __CR_LF__ "\r\n"
#define _LF__ "\n"
#define _CR__ "\r"

#define SAT_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define BIT( x ) (1 << x)

//////////////////////////////////////////////////////////////////////////
// CONCAT
#define SAT_CONTACT_INNER(x,y) x##y
#define SAT_CONTACT(x,y) SAT_CONTACT_INNER(x,y)

#define SAT_CONTACT_THREE_INNER(x,y,z) x##y##z
#define SAT_CONTACT_THREE(x,y,z) SAT_CONTACT_THREE_INNER(x,y,z)

//////////////////////////////////////////////////////////////////////////
// VERSION
#define SAT_MAKE_VERSION(major, minor, patch) ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

#define SAT_CURRENT_VERISON SAT_MAKE_VERSION( 0, 1, 0 )
#define SAT_CURRENT_VERISON_STRING "0.1.0"

#define SAT_DECODE_VERSION(source, major, minor, patch) \
patch = (source) & 0xFF; \
minor = ((source) >> 12) & 0x3FF;\
major = (source) >> 22;

#define SAT_DECODE_VER_STRING(source, string) \
uint32_t SAT_CONTACT_THREE(major,_,__LINE__), SAT_CONTACT_THREE(minor,_,__LINE__), SAT_CONTACT_THREE(patch,_,__LINE__); \
SAT_DECODE_VERSION(source, SAT_CONTACT_THREE(major,_,__LINE__), SAT_CONTACT_THREE(minor,_,__LINE__), SAT_CONTACT_THREE(patch,_,__LINE__)) \
string = std::format( "{0}.{1}.{2}", SAT_CONTACT_THREE(major,_,__LINE__), SAT_CONTACT_THREE(minor,_,__LINE__), SAT_CONTACT_THREE(patch,_,__LINE__) );

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