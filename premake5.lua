
workspace "Saturn"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Saturn/vendor/GLFW/include"
IncludeDir["Glad"] = "Saturn/vendor/Glad/include"
IncludeDir["ImGui"] = "Saturn/vendor/imgui"
IncludeDir["glm"] = "Saturn/vendor/glm"
IncludeDir["stb_image"] = "Saturn/vendor/stb/"
IncludeDir["json_cpp"] = "Saturn/vendor/jsoncpp/"
IncludeDir["Assimp"] = "Saturn/vendor/assimp/"
IncludeDir["entt"] = "Saturn/vendor/entt/include"
IncludeDir["Box2D"] = "Saturn/vendor/box2d/include"

group "sat/Dependencies"
	include "Saturn/vendor/GLFW"
	include "Saturn/vendor/Glad"
	include "Saturn/vendor/imgui"
	include "Saturn/vendor/jsoncpp"
	include "Saturn/vendor/assimp"
	include "Saturn/vendor/box2d"
group "sat/Dependencies/Audio"
	include "Saturn/vendor/Audio/OpenAL-Soft"
	include "Saturn/vendor/Audio/libogg"
	include "Saturn/vendor/Audio/Vorbis"

group "sat/Core"
project "Saturn"
	location "Saturn"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "sppch.h"
	pchsource "Saturn/src/sppch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb/**.cpp",
		"%{prj.name}/vendor/stb/**.h",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"AL_LIBTYPE_STATIC"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
			"%{prj.name}/vendor/imgui/ImGuizmo/",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.json_cpp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.assimp}",
				"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/ImguiFileDialog/ImguiFileDialog",
		"%{prj.name}/vendor/dirent/include",
		"%{prj.name}/vendor/Audio/OpenAL-Soft/include",
		"%{prj.name}/vendor/Audio/OpenAL-Soft/src",
		"%{prj.name}/vendor/Audio/OpenAL-Soft/src/common",
		"%{prj.name}/vendor/Audio/libogg/include",
		"%{prj.name}/vendor/Audio/Vorbis/include",
		"%{prj.name}/vendor/Audio/minimp3"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		"OpenAL-Soft",
		"Jsoncpp",
		"Vorbis",
		"Saturn/vendor/assimp/bin/assimp-vc142-mt.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_WINDOWS",
			"SAT_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"SPARKY_GAME_BASE"
		}

	filter "configurations:Debug"
		defines "SAT_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SAT_DIST"
		runtime "Release"
		optimize "on"
---------------------------------------------------------------------------------------------------------------------------

project "Saturn2D"
	location "Saturn/2D"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"AL_LIBTYPE_STATIC"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%Saturn/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
			"Saturn/vendor/imgui/ImGuizmo/",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.json_cpp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.assimp}",
				"Saturn/vendor/assimp/include",
		"Saturn/vendor/ImguiFileDialog/ImguiFileDialog",
		"Saturn/vendor/dirent/include",
		"Saturn/vendor/Audio/OpenAL-Soft/include",
		"Saturn/vendor/Audio/OpenAL-Soft/src",
		"Saturn/vendor/Audio/OpenAL-Soft/src/common",
		"Saturn/vendor/Audio/libogg/include",
		"Saturn/vendor/Audio/Vorbis/include",
		"Saturn/vendor/Audio/minimp3"
	}

	links 
	{ 
		"Saturn"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_WINDOWS",
			"SAT_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"SPARKY_GAME_BASE"
		}

	filter "configurations:Debug"
		defines "SAT_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SAT_DIST"
		runtime "Release"
		optimize "on"

group "sat/Core"
project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}


	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Saturn/vendor/spdlog/include",
		"Saturn/src",
		"%{IncludeDir.json_cpp}",
		"Saturn/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.Assimp}"
	}

	links
	{
		"Saturn",
		"Saturn2D"
	}


	filter "system:Unix"
		defines
		{
			"SAT_PLATFORM_LINUX"
		}

	filter "system:Mac"
		defines
		{
			"SAT_PLATFORM_MACOS"
		}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "SAT_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SAT_DIST"
		runtime "Release"
		optimize "on"