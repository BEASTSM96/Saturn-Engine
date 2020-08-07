workspace "Sparky"
	architecture "x64"

	configurations
	{ 
		"Debug", 
		"Release", 
		"Dist" 
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

IncludeDir = {}
IncludeDir["GLFW"] = "Spark/vendor/GLFW/include"
IncludeDir["Glad"] = "Spark/vendor/Glad/include"
IncludeDir["ImGui"] = "Spark/vendor/imgui/"
IncludeDir["glm"] = "Spark/vendor/glm/"

include "Spark/vendor/GLFW"
include "Spark/vendor/Glad"
include "Spark/vendor/imgui"


project "Spark"
	location "Spark"
	kind "StaticLib"
	cppdialect "C++17"
	language "C++"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "sppch.h"
	pchsource "spphc.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/**.hpp",
		"%{prj.name}/vendor/glm/**.inl"
	}

	includedirs 
	{
		"Spark/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"Spark/src"
	}
	links 
	{
		"GLFW",
		"Glad",
		"ImGui"
		--"opengl123.lib"

	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SP_PLATFORM_WINDOWS",
			"SP_BUILD_DLL",
			"_SCL_SECURE_NO_WARNINGS",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
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

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	cppdialect "C++17"
	language "C++"
	staticruntime "On"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Spark/vendor/spdlog/include",
		"%{IncludeDir.glm}",
		"Spark/src;"
	}

	links 
	{
		"Spark"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SP_PLATFORM_WINDOWS",
			"_SCL_SECURE_NO_WARNINGS"
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