/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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

#include "Base.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <glm/glm.hpp>

namespace Saturn {

	class Log
	{
		SINGLETON( Log );

		Log() { Init(); }
		~Log() { Clear(); }

		void Init();
		void Clear();

	public:

		static inline std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static inline std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:

		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

	template<typename T>
	static void Info( const T& msg )
	{
		Log::Get().GetCoreLogger()->info( msg );
	}

	template<typename T>
	static void Trace( const T& msg )
	{
		Log::Get().GetCoreLogger()->trace( msg );
	}

	template<typename T>
	static void Warn( const T& msg )
	{
		Log::Get().GetCoreLogger()->warn( msg );
	}

	template<typename T>
	static void Error( const T& msg )
	{
		Log::Get().GetCoreLogger()->error( msg );
	}

	template<typename T>
	static void Fatal( const T& msg )
	{
		Log::Get().GetCoreLogger()->critical( msg );
	}
}

template<typename OStream>
OStream& operator<<( OStream& os, const glm::vec3& vec )
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ')';
}

template<typename OStream>
OStream& operator<<( OStream& os, const glm::vec4& vec )
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
}


// Core log macros
#define SAT_CORE_TRACE(...)				Saturn::Log::Get().GetCoreLogger()->trace(__VA_ARGS__)
#define SAT_CORE_INFO(...)				Saturn::Log::Get().GetCoreLogger()->info(__VA_ARGS__)
#define SAT_CORE_WARN(...)				Saturn::Log::Get().GetCoreLogger()->warn(__VA_ARGS__)
#define SAT_CORE_ERROR(...)				Saturn::Log::Get().GetCoreLogger()->error(__VA_ARGS__)
#define SAT_CORE_FATAL(...)				Saturn::Log::Get().GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SAT_TRACE(...)					Saturn::Log::Get().GetClientLogger()->trace(__VA_ARGS__)
#define SAT_INFO(...)					Saturn::Log::Get().GetClientLogger()->info(__VA_ARGS__)
#define SAT_WARN(...)					Saturn::Log::Get().GetClientLogger()->warn(__VA_ARGS__)
#define SAT_ERROR(...)					Saturn::Log::Get().GetClientLogger()->error(__VA_ARGS__)
#define SAT_FATAL(...)					Saturn::Log::Get().GetClientLogger()->critical(__VA_ARGS__)