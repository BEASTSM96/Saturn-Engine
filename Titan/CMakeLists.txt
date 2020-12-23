file( GLOB_RECURSE titan_source_files "src/*.cpp" )
file( GLOB_RECURSE titan_header_files "src/*.h" )

add_executable( Titan ${titan_source_files} ${titan_header_files} )

target_include_directories( Titan PRIVATE "src" )

set_property( TARGET Titan PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" )
set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Titan )

target_link_libraries( Titan PRIVATE Saturn )