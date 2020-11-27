#pragma once

#include <memory>
#include "Ref.h"

#pragma warning(push, 0)

#if (__cplusplus >= 201402L)
#error C++ 14 is not supported
#else

#include "PlatformDetection.h"

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

namespace Saturn {
	void InitCore();
	void EndCore();
}

#include "Asserts.h"

#define BIT(x) (1 << x)

#define SAT_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#include "Types.h"

namespace Saturn {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using RefSR = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ... Args>
	constexpr RefSR<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

//from the vc++
#define SAT_MoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define SAT_CopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define SAT_FillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define SAT_ZeroMemory(Destination,Length) memset((Destination),0,(Length))

#define SAT_FILEOPENNAMEA OPENFILENAMEA
#define SAT_FILEOPENNAME OPENFILENAME

/*Can be used for macros just core*/
#define _VA_AGRS_(x) x

#endif

#pragma warning(pop)