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
IncludeDir["PhysX"] = "Saturn/vendor/physx/include"
IncludeDir["mono"] = "Saturn/vendor/mono/include"

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
		"Saturn/vendor/SaturnLog/SaturnLogging/src",
		"ProjectBrowser/src"
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
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "SAT_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.lib",
			"Saturn/vendor/mono/lib/mono-2.0-sgen.lib",
			"Saturn/vendor/yaml-cpp/bin/Debug/yaml-cpp.lib",

			-- Link PhysX
			"Saturn/vendor/physx/bin/Debug/LowLevel_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/LowLevelAABB_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/LowLevelDynamics_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysX_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysXCharacterKinematic_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysXCommon_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysXCooking_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysXExtensions_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysXFoundation_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysXPvdSDK_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysXTask_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/PhysXVehicle_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/SceneQuery_static_64.lib",
			"Saturn/vendor/physx/bin/Debug/SimulationController_static_64.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Debug/PhysX_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Debug/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Debug/PhysXCommon_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Debug/PhysXFoundation_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}


	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.lib",
			"Saturn/vendor/yaml-cpp/bin/Release/yaml-cpp.lib",

			-- Link PhysX
			"Saturn/vendor/physx/bin/Release/LowLevel_static_64.lib",
			"Saturn/vendor/physx/bin/Release/LowLevelAABB_static_64.lib",
			"Saturn/vendor/physx/bin/Release/LowLevelDynamics_static_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysX_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysXCharacterKinematic_static_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysXCommon_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysXCooking_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysXExtensions_static_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysXFoundation_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysXPvdSDK_static_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysXTask_static_64.lib",
			"Saturn/vendor/physx/bin/Release/PhysXVehicle_static_64.lib",
			"Saturn/vendor/physx/bin/Release/SceneQuery_static_64.lib",
			"Saturn/vendor/physx/bin/Release/SimulationController_static_64.lib"

		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysX_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXCommon_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXFoundation_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}

	filter "configurations:Dist"
		defines "SAT_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.lib",
			-- Link PhysX
			"Saturn/vendor/PhysX/bin/Release/LowLevel_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/LowLevelAABB_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/LowLevelDynamics_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysX_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysXCharacterKinematic_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysXCommon_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysXCooking_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysXExtensions_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysXFoundation_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysXPvdSDK_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysXTask_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/PhysXVehicle_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/SceneQuery_static_64.lib",
			"Saturn/vendor/PhysX/bin/Release/SimulationController_static_64.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}

---------------------------------------------------------------------------------------------------------------------------

group "sat/Core"

project "ProjectBrowser"
location "ProjectBrowser"
	kind "StaticLib"
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
		"%{prj.name}/src",
		"Saturn/src",
		"Saturn/vendor/spdlog/include",
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
			"Saturn/vendor/assimp/include/",
		"%{IncludeDir.SPIRV_Cross}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.FontAwesome}",
		"Saturn/vendor/yaml-cpp/include",
		"Saturn/vendor/SaturnLog/SaturnLogging/src"
	}

	links
	{
		"Saturn"
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
		"%{IncludeDir.SPIRV_Cross}",
		"ProjectBrowser/src"
	}

	links
	{
		"Saturn",
		"ProjectBrowser"
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
			'{COPY} "../SaturnScript/build/SaturnScript.dll" "%{cfg.targetdir}/assets/assembly/SaturnRuntime.dll"',

			-- Copy PhysX
			'{COPY} "../Saturn/vendor/physx/bin/Debug/PhysX_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Debug/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Debug/PhysXCommon_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Debug/PhysXFoundation_64.dll" "%{cfg.targetdir}"'
		}

	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"',

			-- Copy PhysX
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysX_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXCommon_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXFoundation_64.dll" "%{cfg.targetdir}"'
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

			-- Copy PhysX
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysX_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXCommon_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/physx/bin/Release/PhysXFoundation_64.dll" "%{cfg.targetdir}"'
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
