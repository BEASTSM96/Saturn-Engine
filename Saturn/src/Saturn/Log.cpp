#include "sppch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/sinks/basic_file_sink.h>

#include "ImGui/ImGuiConsoleSink.h"

namespace Saturn {

	std::shared_ptr< spdlog::logger > Log::s_CoreLogger;
	std::shared_ptr< spdlog::logger > Log::s_ClientLogger;

	void Log::Init( void )
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back( std::make_shared< spdlog::sinks::stdout_color_sink_mt >() );
		logSinks.emplace_back( std::make_shared< spdlog::sinks::basic_file_sink_mt >( "Saturn.log", true ) );
		logSinks.emplace_back( std::make_shared< ImGuiConsoleSink_mt >( true ) );

		logSinks[0]->set_pattern( "%^[%T] %n: %v%$" );
		logSinks[1]->set_pattern( "[%T] [%l] %n: %v" );
		logSinks[2]->set_pattern( "%^[%T] [%l] %n: %v%$" );

		s_CoreLogger = std::make_shared< spdlog::logger >( "SATURN", begin( logSinks ), end( logSinks ) );
		spdlog::register_logger( s_CoreLogger );

		s_ClientLogger = std::make_shared< spdlog::logger >( "APP", begin( logSinks ), end( logSinks ) );
		spdlog::register_logger( s_ClientLogger );

		// configure the loggers
		spdlog::set_pattern( "%^[%T] %n: %v%$" );
		s_CoreLogger->set_level( spdlog::level::trace );
		s_ClientLogger->set_level( spdlog::level::trace );
		//s_ClientLogger->flush_on(spdlog::level::trace);
		//s_CoreLogger->flush_on(spdlog::level::trace);
	}

}