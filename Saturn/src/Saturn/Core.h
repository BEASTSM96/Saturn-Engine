#pragma once

#include <memory>

#if (__cplusplus >= 201402L)
#error C++ 14 is not supported
#else
#ifdef _WIN32
/* Windows x64/x86 */
#ifdef _WIN64
	/* Windows x64  */
#define SAT_PLATFORM_WINDOWS
#ifdef SAT_DLL && SAT_PLATFORM_WINDOWS
#define SATURN_API 
#else
#define SATURN_API
#endif
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
#define SAT_PLATFORM_IOS
#error "IOS is not supported!"
#elif TARGET_OS_MAC == 1
#define SAT_PLATFORM_MACOS
#error "MacOS is not supported!"
#else
#error "Unknown Apple platform!"
#endif
 /* We also have to check __ANDROID__ before __linux__
  * since android is based on the linux kernel
  * it has __linux__ defined */
#elif defined(__ANDROID__)
#define SAT_PLATFORM_ANDROID
#error "Android is not supported!"
#elif defined(__linux__)
#define SAT_PLATFORM_LINUX
#error "Linux is not supported!"
#else
/* Unknown compiler/platform */
#error "Unknown platform!"
#endif // End of platform detection

#ifdef SAT_DLL

#define SAT_DLL_IMPORT __declspec(dllimport)
#define SAT_DLL_EXPORT __declspec(dllexport)

#else
#define SAT_DLL_IMPORT
#define SAT_DLL_EXPORT

#endif // SAT_DLL


#ifdef SAT_DEBUG
	#if defined(SAT_PLATFORM_WINDOWS)
		#define SAT_DEBUGBREAK() __debugbreak()
	#elif defined(SAT_PLATFORM_LINUX)
			#include <signal.h>
			#define SAT_DEBUGBREAK() raise(SIGTRAP)
	#else
			#error "Platform doesn't support debugbreak yet!"
	#endif
	#define SAT_ENABLE_ASSERTS
#else
	#define SAT_DEBUGBREAK()
#endif

// TODO: Make this macro able to take in no arguments except condition
#ifdef SAT_ENABLE_ASSERTS
	#define SAT_ASSERT(x, ...) { if(!(x)) {/*SAT_ERROR("Assertion Failed: {0}", __VA_ARGS__);*/ SAT_DEBUGBREAK(); } }
	#define SAT_CORE_ASSERT(x, ...) { if(!(x)) { /*SAT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);*/ SAT_DEBUGBREAK(); } }
#else
	#define SAT_CL_ASSERT(x, ...)
	#define SAT_CORE_ASSERT(x, ...)
#endif


#define BIT(x) (1 << x)

#define SAT_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

/** For testing the app (aka just mem) */
#define SAT_TEST

#define SAT_class(name) class SATURN_API(name)

#define SAT_FORCEINLINE inline

#define SAT_FORCE_INLINE __forceinline

namespace Saturn {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using Refprt = T*;

	using SPint = uint64_t;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}


	typedef struct fIO //base engine file system
	{
		float			SavingRate;                     // = 5.0f
		const char*		IniFilename;                    // = "{engver}Saturn.ini"
		const char*		LogFilename;                    // = "Saturn.log"

		//////////////////////////////////////////////////////////////////////////////////////////////////////

		float			ScSavingRate;							// = 5.0f
		const char*		ScFilename;						       // = "{gamename ? enginegame ? edname}.sats"
		const char*		ScLogFilename;                        // = "{scname}.log"

		//////////////////////////////////////////////////////////////////////////////////////////////////////

		float			MapSavingRate;							// = 5.0f
		const char*		MapFilename;						       // = "{mapname}.smap"
		const char*		MapLogFilename;                        // = "{mapname}.log"


	};
	typedef fIO FileIO;

#ifndef SAT_VERSON 1.0
#define SAT_VERSON 1.0
#endif //!SAT_VERSON 1.0


}


/* def's core stuff*/
#if defined (SAT_PLATFORM_WINDOWS)
#define SAT_CORE_DELAY(...) ::std::this_thread::sleep_for(__VA_ARGS__)
#endif


#endif
