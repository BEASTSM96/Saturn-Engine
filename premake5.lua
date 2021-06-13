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
IncludeDir["stb_image"] = "Saturn/vendor/stb/"
IncludeDir["Assimp"] = "Saturn/vendor/assimp/include"
IncludeDir["entt"] = "Saturn/vendor/entt/include"
IncludeDir["SPIRV_Cross"] = "Saturn/vendor/SPIRV-Cross/"
IncludeDir["PhysX"] = "Saturn/vendor/physx/include"
IncludeDir["yaml_cpp"] = "Saturn/vendor/yaml-cpp/include"
IncludeDir["Saturn-Serialisation"] = "Saturn/vendor/Saturn-Serialisation/"
IncludeDir["mono"] = "Saturn/vendor/mono/include"
IncludeDir["FontAwesome"] = "Saturn/vendor/FontAwseome/"

group "sat/Dependencies"
	include "Saturn/vendor/GLFW"
	include "Saturn/vendor/Glad"
	include "Saturn/vendor/imgui"
	include "Saturn/vendor/assimp"
	include "Saturn/vendor/SPIRV_Cross"
	include "Saturn/vendor/physx"
	group "sat/Dependencies/Serialisation"
			include "Saturn/vendor/yaml-cpp"

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
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/pxshared",
		"%{IncludeDir.PhysX}/physx",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
			"%{prj.name}/vendor/assimp/include/",
		"%{IncludeDir.SPIRV_Cross}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.FontAwesome}",
		"Saturn/vendor/yaml-cpp/include",
		"Saturn/vendor/SaturnLog/SaturnLogging/src"
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

		defines
		{
			"SAT_PLATFORM_WINDOWS",
			"SAT_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "SAT_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.lib",
			"Saturn/vendor/physx/bin/Debug/PhysX.lib",
			"Saturn/vendor/mono/lib/mono-2.0-sgen.lib",
			"Saturn/vendor/yaml-cpp/bin/Debug/yaml-cpp.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}


	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.lib",
			"Saturn/vendor/physx/bin/Release/PhysX.lib",
			"Saturn/vendor/yaml-cpp/bin/Release/yaml-cpp.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}

	filter "configurations:Dist"
		defines "SAT_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.lib",
			"Saturn/vendor/physx/bin/Dist/PhysX_64.lib",
		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}

---------------------------------------------------------------------------------------------------------------------------

group "sat/Core"

---------------------------------------------------------------------------------------------------------------------------

group "sat/Core"
project "SaturnScript"
	location "SaturnScript"
	kind "SharedLib"
	language "C#"
	warnings "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.cs"
	}

	postbuildcommands 
	{
		--'{COPY} "../%{cfg.targetdir}/SaturnScript.dll" "../SaturnScript/build/SaturnScript.dll"',
	}

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
		"%{IncludeDir.json_cpp}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/pxshared",
		"%{IncludeDir.PhysX}/physx",
		"%{IncludeDir.ReactPhysics3D}",
		"Saturn/vendor/yaml-cpp/include",
		"Saturn/vendor/glm/",
		"%{IncludeDir.mono}",
		"%{IncludeDir.SPIRV_Cross}"
	}

	links
	{
		"Saturn"
	}

	postbuildcommands 
	{
		'{COPY} "../Titan/assets" "%{cfg.targetdir}/assets"',
		'{COPY} "../Titan/imgui.ini" "%{cfg.targetdir}/imgui.ini"'
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

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/mono/lib/mono-2.0-sgen.lib" "%{cfg.targetdir}/assets/mono/lib"',
			'{COPY} "../Saturn/vendor/mono/lib/eglib.lib" "%{cfg.targetdir}/assets/mono/lib"',
			'{COPY} "../Saturn/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
			'{COPY} "../SaturnScript/build/SaturnScript.dll" "%{cfg.targetdir}/assets/assembly/SaturnRuntime.dll"'
		}

	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}

	filter "configurations:Dist"
		defines "SAT_DIST"
		runtime "Release"
		optimize "on"

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/mono/bin/Dist/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/mono/lib/mono-2.0-sgen.lib" "%{cfg.targetdir}/assets/mono/lib"',
			'{COPY} "../Titan/imgui.ini" "%{cfg.targetdir}/imgui.ini"',
		}

group "sat/Runtime"
project "ExampleApp"
	location "ExampleApp"
	kind "SharedLib"
	language "C#"
	warnings "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.cs"
	}

	links 
	{
		"SaturnScript"
	}

	postbuildcommands 
	{
		
	}
