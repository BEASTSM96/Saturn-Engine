#pragma once

#include "Core/Base.h"

#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#pragma warning(pop)

#include "ImGui/ImGuiConsole.h"

#include "ImGui/ImGuiLayer.h"

namespace Saturn {

	class SATURN_API Log
	{
	public:
		static void Init();

		static RefSR< spdlog::logger >& GetCoreLogger() { return s_CoreLogger; }
		static RefSR< spdlog::logger >& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core log macros
#define SAT_CORE_TRACE(...)				Saturn::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SAT_CORE_INFO(...)				Saturn::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SAT_CORE_WARN(...)				Saturn::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SAT_CORE_ERROR(...)				Saturn::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SAT_CORE_FATAL(...)				Saturn::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SAT_CL_TRACE(...)				Saturn::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SAT_CL_INFO(...)				Saturn::Log::GetClientLogger()->info(__VA_ARGS__)
#define SAT_CL_WARN(...)				Saturn::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SAT_CL_ERROR(...)				Saturn::Log::GetClientLogger()->error(__VA_ARGS__)
#define SAT_CL_FATAL(...)				Saturn::Log::GetClientLogger()->critical(__VA_ARGS__)