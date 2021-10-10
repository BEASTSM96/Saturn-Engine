workspace "Saturn"
	architecture "x64"
	startproject "Titan"
	targetdir "build"
	warnings "Off"

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
IncludeDir["entt"] = "Saturn/vendor/entt/include"
IncludeDir["assimp"] = "Saturn/vendor/assimp/include"

group "sat/Dependencies"
	include "Saturn/vendor/GLFW"
	include "Saturn/vendor/Glad"
	include "Saturn/vendor/imgui"

group "sat/Core"
project "Saturn"
	location "Saturn"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	warnings "Off"

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
		"PX_PHYSX_STATIC_LIB",
		"PX_GENERATE_STATIC_LIBRARIES",
		"AL_LIBTYPE_STATIC"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/stb/",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		links 
		{
			"dwmapi"	
		}

		defines
		{
			"SAT_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
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

	
	filter "system:linux"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_LINUX"
		}

		links 
		{
			"stdc++fs",
			"pthread",
			"dl",
			"GL",
			"X11",
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

	filter "system:macosx"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_MAC"
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

group "sat/Tools"
project "Titan"
	location "Titan"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	warnings "Off"

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
		"Saturn/vendor",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
	}

	links
	{
		"Saturn"
	}

	postbuildcommands 
	{
		--'{COPY} "../Titan/assets" "%{cfg.targetdir}/assets"',
		--'{COPY} "../Titan/imgui.ini" "%{cfg.targetdir}/imgui.ini"'
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
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


	filter "system:linux"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_LINUX"
		}

		links 
		{
			"stdc++fs",
			"pthread",
			"dl",
			"GL",
			"X11",
			"GLFW",
			"Glad",
			"ImGui"
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

	filter "system:macosx"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_MAC"
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