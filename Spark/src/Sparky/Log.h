#pragma once



#include "Core.h"
#include "spdlog\spdlog.h"
#include "spdlog\fmt\ostr.h"

#include "ImGui/ImGuiLayer.h"

namespace Sparky {

	class SPARKY_API Log
	{
	public:
		static void Init();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core log macros
#define SP_CORE_TRACE(...)    ::Sparky::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SP_CORE_INFO(...)     ::Sparky::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SP_CORE_WARN(...)     ::Sparky::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SP_CORE_ERROR(...)    ::Sparky::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SP_CORE_FATAL(...)    ::Sparky::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SP_CL_TRACE(...)	      ::Sparky::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SP_CL_INFO(...)	      ::Sparky::Log::GetClientLogger()->info(__VA_ARGS__)
#define SP_CL_WARN(...)	      ::Sparky::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SP_CL_ERROR(...)	      ::Sparky::Log::GetClientLogger()->error(__VA_ARGS__)
#define SP_CL_FATAL(...)	      ::Sparky::Log::GetClientLogger()->critical(__VA_ARGS__) 