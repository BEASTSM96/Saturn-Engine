-- Game Project premake template.
-- __PROJECT_NAME__

workspace "__PROJECT_NAME__"
	architecture "x64"
	startproject "__PROJECT_NAME__"
	targetdir "build"
	warnings "Off"

	configurations { "Debug", "Release", "Dist" }

	flags { "MultiProcessorCompile" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["Saturn"] = "__SATURN_DIR__"

group "Game"
project "__PROJECT_NAME__"
	location "__PROJECT_NAME__"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
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
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.Saturn}"
	}

	links 
	{
		-- Maybe link the engine??
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			runtime "Release"
			optimize "on"
			symbols "Off"