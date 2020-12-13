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

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};


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
#define SAT_CORE_TRACE(...)				Saturn::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SAT_CORE_INFO(...)				Saturn::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SAT_CORE_WARN(...)				Saturn::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SAT_CORE_ERROR(...)				Saturn::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SAT_CORE_FATAL(...)				Saturn::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SAT_TRACE(...)					Saturn::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SAT_INFO(...)					Saturn::Log::GetClientLogger()->info(__VA_ARGS__)
#define SAT_WARN(...)					Saturn::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SAT_ERROR(...)					Saturn::Log::GetClientLogger()->error(__VA_ARGS__)
#define SAT_FATAL(...)					Saturn::Log::GetClientLogger()->critical(__VA_ARGS__)