#pragma once

#include <memory>

#ifdef SP_PLATFORM_WINDOWS
#if SP_DYNAMIC_LINK
#ifdef SP_BUILD_DLL
#define SPARKY_API __declspec(dllexport)
#else
#define SPARKY_API __declspec(dllimport)
#endif
#else
#define SPARKY_API
#endif
#else
#error Sparkys only supports Windows!
#endif 

#define BIT(x) (1 << x)

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
#define SSP_CL_ASSERT(x, ...) { if(!(x)) { SP_CL_ERROR("Assertion Failed: {0}", __VA_ARGS__); SP_DEBUGBREAK(); } }
#define SP_CORE_ASSERT(x, ...) { if(!(x)) { SP_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); SP_DEBUGBREAK(); } }
#else
#define SP_ASSERT(x, ...)
#define SP_CORE_ASSERT(x, ...)
#endif

#define SP_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Sparky {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

}