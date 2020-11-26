#pragma once

// TODO: Make this macro able to take in no arguments except condition
#ifdef SAT_ENABLE_ASSERTS
#define SAT_ASSERT(x, ...) { if(!(x)) {/*SAT_ERROR("Assertion Failed: {0}", __VA_ARGS__);*/ __debugbreak(); } }
#define SAT_CORE_ASSERT(x, ...) { if(!(x)) { /*SAT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);*/ __debugbreak(); } }
#else
#define SAT_CL_ASSERT(x, ...)
#define SAT_CORE_ASSERT(x, ...)
#endif