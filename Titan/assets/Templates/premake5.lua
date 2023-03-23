-- Game Project premake template.
-- __PROJECT_NAME__

workspace "__PROJECT_NAME__"
	architecture "x64"
	startproject "__PROJECT_NAME__"
	warnings "Off"

	configurations { "Debug", "Release", "Dist", "Dist-Debug", "Dist-Release", "Dist-Full" }

	flags { "MultiProcessorCompile" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}

group "Game"
project "__PROJECT_NAME__"
	kind "Makefile"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Scripts/**.h",
		"Scripts/**.cpp"
	}

	removefiles 
	{ 
		"**.Gen.cpp", 
		"**.Gen.h" 
	}

	-- Right now we are always going to link from the debug, when we build the engine in Dist maybe with change to be Dist.
	links
	{
		"__SATURN_BIN_DIR__/Saturn.lib"
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"

			buildcommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /BUILD /%{prj.name} /Win64 /Debug /%{prj.location}"
			}

			rebuildcommands 
			{
				"__SATURN_BT_DIR__/RT/Run.bat /REBUILD /%{prj.name} /Win64 /Debug /%{prj.location}"
			}

			cleancommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /CLEAN /%{prj.name} /Win64 /Debug /%{prj.location}"
			}

		filter "configurations:Release"
			runtime "Release"
			optimize "on"

			buildcommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /BUILD /%{prj.name} /Win64 /Release /%{prj.location}"
			}

			rebuildcommands 
			{
				"__SATURN_BT_DIR__/RT/Run.bat /REBUILD /%{prj.name} /Win64 /Release /%{prj.location}"
			}

			cleancommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /CLEAN /%{prj.name} /Win64 /Release /%{prj.location}"
			}

		filter "configurations:Dist"
			runtime "Release"
			symbols "on"

			buildcommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /BUILD /%{prj.name} /Win64 /Dist /%{prj.location}"
			}

			rebuildcommands 
			{
				"__SATURN_BT_DIR__/RT/Run.bat /REBUILD /%{prj.name} /Win64 /Dist /%{prj.location}"
			}

			cleancommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /CLEAN /%{prj.name} /Win64 /Dist /%{prj.location}"
			}

		filter "configurations:Dist-Debug"
			runtime "Debug"
			optimize "on"
			symbols "Off"

			buildcommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /BUILD /%{prj.name} /Win64 /DDebug /%{prj.location}"
			}

			rebuildcommands 
			{
				"__SATURN_BT_DIR__/RT/Run.bat /REBUILD /%{prj.name} /Win64 /DDebug /%{prj.location}"
			}

			cleancommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /CLEAN /%{prj.name} /Win64 /DDebug /%{prj.location}"
			}

		filter "configurations:Dist-Release"
			runtime "Release"
			optimize "on"

			buildcommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /BUILD /%{prj.name} /Win64 /DRelease /%{prj.location}"
			}

			rebuildcommands 
			{
				"__SATURN_BT_DIR__/RT/Run.bat /REBUILD /%{prj.name} /Win64 /DRelease /%{prj.location}"
			}

			cleancommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /CLEAN /%{prj.name} /Win64 /DRelease /%{prj.location}"
			}

		filter "configurations:Dist-Full"
			runtime "Release"
			optimize "on"
			symbols "on"

			buildcommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /BUILD /%{prj.name} /Win64 /DF /%{prj.location}"
			}

			rebuildcommands 
			{
				"__SATURN_BT_DIR__/RT/Run.bat /REBUILD /%{prj.name} /Win64 /DF /%{prj.location}"
			}

			cleancommands
			{
				"__SATURN_BT_DIR__/RT/Run.bat /CLEAN /%{prj.name} /Win64 /DF /%{prj.location}"
			}