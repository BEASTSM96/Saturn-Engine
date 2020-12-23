cmake_minimum_required( VERSION 3.16 )

if( ${CMAKE_VERSION} VERSION_LESS 3.12 )
	cmake_policy( VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION } )
endif()

set( CMAKE_CXX_STANDARD 17 )

project( Saturn VERSION 0.0.1 DESCRIPTION "Saturn Engine." LANGUAGES C CXX )


add_subdirectory( Saturn/vendor/GLFW )
add_subdirectory( Saturn/vendor/Glad )
add_subdirectory( Saturn/vendor/imgui )
add_subdirectory( Saturn/vendor/yaml-cpp )


add_subdirectory( Saturn )
add_subdirectory( Titan )
add_subdirectory( Sandbox )

if(WIN32)
set_property(  DIRECTORY PROPERTY VS_STARTUP_PROJECT "Titan" )
endif()

