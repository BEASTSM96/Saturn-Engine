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

#include "Core/Base.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <glm/glm.hpp>

#include "ImGui/ImGuiConsole.h"

namespace Saturn {

	class Log
	{
	public:
		static void Init( void );
		static void Clear( void );

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

	template<typename T>
	static void Info( const T& msg )
	{
		Log::GetCoreLogger()->info( msg );
	}

	template<typename T>
	static void Trace( const T& msg )
	{
		Log::GetCoreLogger()->trace( msg );
	}

	template<typename T>
	static void Warn( const T& msg )
	{
		Log::GetCoreLogger()->warn( msg );
	}

	template<typename T>
	static void Error( const T& msg )
	{
		Log::GetCoreLogger()->error( msg );
	}

	template<typename T>
	static void Fatal( const T& msg )
	{
		Log::GetCoreLogger()->critical( msg );
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
#define SAT_CORE_TRACE (...)				Saturn::Log::GetCoreLogger()->trace    (__VA_ARGS__)
#define SAT_CORE_INFO  (...)				Saturn::Log::GetCoreLogger()->info     (__VA_ARGS__)
#define SAT_CORE_WARN  (...)				Saturn::Log::GetCoreLogger()->warn     (__VA_ARGS__)
#define SAT_CORE_ERROR (...)				Saturn::Log::GetCoreLogger()->error    (__VA_ARGS__)
#define SAT_CORE_FATAL (...)				Saturn::Log::GetCoreLogger()->critical (__VA_ARGS__)

// Client log macros
#define SAT_TRACE      (...)                Saturn::Log::GetClientLogger()->trace    (__VA_ARGS__)
#define SAT_INFO       (...)				Saturn::Log::GetClientLogger()->info     (__VA_ARGS__)
#define SAT_WARN       (...)				Saturn::Log::GetClientLogger()->warn     (__VA_ARGS__)
#define SAT_ERROR      (...)				Saturn::Log::GetClientLogger()->error    (__VA_ARGS__)
#define SAT_FATAL      (...)				Saturn::Log::GetClientLogger()->critical (__VA_ARGS__)