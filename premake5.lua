workspace "Sparky"
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
IncludeDir["GLFW"] = "Spark/vendor/GLFW/include"
IncludeDir["Glad"] = "Spark/vendor/Glad/include"
IncludeDir["ImGui"] = "Spark/vendor/imgui"
IncludeDir["glm"] = "Spark/vendor/glm"
IncludeDir["stb_image"] = "Spark/vendor/stb/"
IncludeDir["json_cpp"] = "Spark/vendor/jsoncpp/"



group "sp/Dependencies"
	include "Spark/vendor/GLFW"
	include "Spark/vendor/Glad"
	include "Spark/vendor/imgui"
	include "Spark/vendor/jsoncpp"
group "sp/Dependencies/Audio"
	include "Spark/vendor/Audio/OpenAL-Soft"
	include "Spark/vendor/Audio/libogg"
	include "Spark/vendor/Audio/Vorbis"

group "sp/Core"
project "Spark"
	location "Spark"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "sppch.h"
	pchsource "Spark/src/sppch.cpp"

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
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.json_cpp}",
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
		"Vorbis"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SP_PLATFORM_WINDOWS",
			"SP_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"SPARKY_GAME_BASE"
		}

	filter "configurations:Debug"
		defines "SP_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "on"

group "sp/Core"
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
		"Spark/vendor/spdlog/include",
		"Spark/src",
		"%{IncludeDir.json_cpp}",
		"Spark/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Spark"
	}


	filter "system:Unix"
		defines
		{
			"SP_PLATFORM_LINUX"
		}

	filter "system:Mac"
		defines
		{
			"SP_PLATFORM_MACOSX"
		}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SP_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "SP_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "on"
