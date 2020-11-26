#pragma once

#ifdef SAT_DEBUG
	#define SAT_ENABLE_ASSERTS
#endif // SAT_DEBUG

// TODO: Make this macro able to take in no arguments except condition
#ifdef SAT_ENABLE_ASSERTS

#define SAT_ASSERTS_FIXED
#ifdef SAT_ASSERTS_FIXED
#define SAT_ASSERT_NO_MESSAGE(condition) { if(!(condition)) { SAT_CORE_ERROR("Assertion Failed"); __debugbreak(); } }
#define SAT_ASSERT_MESSAGE(condition, ...) { if(!(condition)) { SAT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }

#define SAT_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro
#define SAT_GET_ASSERT_MACRO(...) _VA_AGRS_(SAT_ASSERT_RESOLVE(__VA_ARGS__, SAT_ASSERT_MESSAGE, SAT_ASSERT_NO_MESSAGE))

#define SAT_ASSERT(...) _VA_AGRS_( SAT_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define SAT_CORE_ASSERT(...) _VA_AGRS_( SAT_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
#define SAT_ASSERT(x, ...) { if(!(x)) {/*SAT_ERROR("Assertion Failed: {0}", __VA_ARGS__);*/ SAT_DEBUGBREAK(); } }
#define SAT_CORE_ASSERT(x, ...) { if(!(x)) { /*SAT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);*/ SAT_DEBUGBREAK(); } }
#endif // SAT_ASSERTS_FIXED

#else
#ifdef SAT_ASSERTS_FIXED
#define SAT_ASSERT_NO_MESSAGE(condition)
#define SAT_ASSERT_MESSAGE(condition, ...)
#define SAT_ASSERT_RESOLVE(arg1, arg2, macro, ...)
#define SAT_GET_ASSERT_MACRO(...)
#define SAT_ASSERT(...)
#define SAT_CORE_ASSERT(...)
#endif

#define SAT_ASSERT(x, ...)
#define SAT_CORE_ASSERT(x, ...)
#endif