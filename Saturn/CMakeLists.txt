file( GLOB_RECURSE saturn_src_files "src/Saturn/*.cpp" )
file( GLOB_RECURSE saturn_header_files "src/Saturn/*.h" )

set( SAT_ALL_PLARFORM_FILES "src/Saturn.h" "src/sppch.h" "src/sppch.cpp" ${saturn_src_files} %{saturn_header_files} "vendor/stb/stb_image.h" "vendor/stb/stb_image.cpp" )

file( GLOB_RECURSE opengl_src_files "src/Plaform/OpenGL/*.cpp" )
file( GLOB_RECURSE opengl_header_files "src/Plaform/OpenGL/*.h" )

file( GLOB_RECURSE win_src_files "src/Plaform/Windows/*.cpp" )
file( GLOB_RECURSE win_header_files "src/Plaform/Windows/*.h" )

#TEMP!
#if( Win32 )

set( SAT_OS_FILES ${win_src_files} ${win_header_files} )

#endif()

add_library( Saturn STATIC ${SAT_ALL_PLARFORM_FILES} ${SAT_OS_FILES} 
	${opengl_src_files} 
	%{opengl_header_files} 
	"src/Saturn/ImGui/ImGuizmo.h" 
	"src/Saturn/ImGui/ImGuizmo.cpp" 
)

target_precompile_headers( Saturn PUBLIC "src/sppch.h" )

target_include_directories( Saturn PRIVATE "vendor/GLFW/include" "vendor/Glad/include" "vendor/stb/stb_image" )

target_include_directories( Saturn PUBLIC "src" "vendor" "vendor/spdlog/include" "vendor/imgui" "src/Saturn/ImGui/ImGuizmo.h" "vendor/glm" "vendor/SPIRV_Cross" "vendor/reactphysics3d/include" "vendor/SaturnLog/SaturnLogging/src" "vendor/entt/include" "vendor/assimp/include" "vendor/yaml-cpp/include" )