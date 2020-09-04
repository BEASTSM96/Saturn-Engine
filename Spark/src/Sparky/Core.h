#pragma once

#include <memory>

#if (__cplusplus >= 201402L)
#error C++ 14 is not supported
#else
#ifdef _WIN32
/* Windows x64/x86 */
#ifdef _WIN64
	/* Windows x64  */
#define SP_PLATFORM_WINDOWS
#define SPARKY_API
#else
	/* Windows x86 */
#error "x86 Builds are not supported!"
#endif
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
/* TARGET_OS_MAC exists on all the platforms
 * so we must check all of them (in this order)
 * to ensure that we're running on MAC
 * and not some other Apple platform */
#if TARGET_IPHONE_SIMULATOR == 1
#error "IOS simulator is not supported!"
#elif TARGET_OS_IPHONE == 1
#define SP_PLATFORM_IOS
#error "IOS is not supported!"
#elif TARGET_OS_MAC == 1
#define SP_PLATFORM_MACOS
#error "MacOS is not supported!"
#else
#error "Unknown Apple platform!"
#endif
 /* We also have to check __ANDROID__ before __linux__
  * since android is based on the linux kernel
  * it has __linux__ defined */
#elif defined(__ANDROID__)
#define SP_PLATFORM_ANDROID
#error "Android is not supported!"
#elif defined(__linux__)
#define SP_PLATFORM_LINUX
#error "Linux is not supported!"
#else
/* Unknown compiler/platform */
#error "Unknown platform!"
#endif // End of platform detection


#ifdef SP_DEBUG
	#if defined(SP_PLATFORM_WINDOWS)
		#define SP_DEBUGBREAK() __debugbreak()
	#elif defined(SP_PLATFORM_LINUX)
			#include <signal.h>
			#define SP_DEBUGBREAK() raise(SIGTRAP)
	#else
			#error "Platform doesn't support debugbreak yet!"
	#endif
	#define SP_ENABLE_ASSERTS
#else
	#define SP_DEBUGBREAK()
#endif

// TODO: Make this macro able to take in no arguments except condition
#ifdef SP_ENABLE_ASSERTS
	#define SP_CL_ASSERT(x, ...) { if(!(x)) { SP_CL_ERROR("Assertion Failed: {0}", __VA_ARGS__); SP_DEBUGBREAK(); } }
	#define SP_CORE_ASSERT(x, ...) { if(!(x)) { SP_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); SP_DEBUGBREAK(); } }
#else
	#define SP_CL_ASSERT(x, ...)
	#define SP_CORE_ASSERT(x, ...)
#endif


#define BIT(x) (1 << x)

#define SP_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#define SPARKY_NAMESPACE namespace Sparky {
#define SPARKY_NAMESPACE_END }

#define SP_CLASS(name) class(name)

SPARKY_NAMESPACE  

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using Refprt = T*;

	using SPint = uint64_t;
SPARKY_NAMESPACE_END


/* def's core stuff*/
#if defined (SP_PLATFORM_WINDOWS)
#define SP_CORE_DELAY(...) ::std::this_thread::sleep_for(__VA_ARGS__)
#endif



#endif
