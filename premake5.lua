workspace "Saturn"
	architecture "x64"
	startproject "Saturn-Editor"
	targetdir "build"
	warnings "Default"

	configurations { "Debug", "Release", "Dist" }

	flags { "MultiProcessorCompile" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["Ruby"] = "Saturn/vendor/Ruby/src"
IncludeDir["ImGui"] = "Saturn/vendor/imgui"
IncludeDir["glm"] = "Saturn/vendor/glm"
IncludeDir["entt"] = "Saturn/vendor/entt/include"
IncludeDir["assimp"] = "Saturn/vendor/assimp/include"
IncludeDir["shaderc"] = "Saturn/vendor/shaderc/libshaderc/include"
IncludeDir["glslc"] = "Saturn/vendor/shaderc/glslc/src"
IncludeDir["SPIRV_Cross"] = "Saturn/vendor/SPIRV-Cross/src/"
IncludeDir["vma"] = "Saturn/vendor/vma/src/"
IncludeDir["ImGuizmo"] = "Saturn/vendor/ImGuizmo/src/"
IncludeDir["yaml_cpp"] = "Saturn/vendor/yaml-cpp/include/"
IncludeDir["ImguiNodeEditor"] = "Saturn/vendor/imgui_node_editor"
IncludeDir["ImSpinner"] = "Saturn/vendor/imspinner/src"
IncludeDir["Tracy"] = "Saturn/vendor/tracy/src"
IncludeDir["Filewatch"] = "Saturn/vendor/Filewatch/src"
IncludeDir["MiniAudio"] = "Saturn/vendor/miniaudio/src"
IncludeDir["PhysX"] = "Saturn/vendor/physx/include"
IncludeDir["SharedStorage"] = "Saturn-SharedStorage/src"
IncludeDir["zlib"] = "Saturn/vendor/zlib"

group "Dependencies"
--	include "Saturn/vendor/Ruby"
	include "Saturn/vendor/imgui"
	include "Saturn/vendor/SPIRV-Cross"
	include "Saturn/vendor/yaml-cpp"
	include "Saturn/vendor/tracy"
	include "Saturn/vendor/zlib"

group "Engine"
project "Saturn"
	location "Saturn"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	warnings "Default"

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
		"%{prj.name}/vendor/vma/src/**.cpp",
		"%{prj.name}/vendor/vma/src/**.h",
		"%{prj.name}/vendor/vulkan/**.h",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/ImGuizmo/src/**.cpp",
		"%{prj.name}/vendor/ImGuizmo/src/**.h",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"PX_PHYSX_STATIC_LIB",
		"PX_GENERATE_STATIC_LIBRARIES",
		"AL_LIBTYPE_STATIC",
		"GLM_ENABLE_EXPERIMENTAL",
		"SATURN_SS_IMPORT",
		"TRACY_ENABLE",
		"SAT_RBY_INCLUDE_VULKAN"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/stb/",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/vulkan/include",
--		"%{IncludeDir.Ruby}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.glslc}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.SPIRV_Cross}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.ImguiNodeEditor}",
		"%{IncludeDir.ImSpinner}",
		"%{IncludeDir.Tracy}",
		"%{IncludeDir.MiniAudio}",
		"%{IncludeDir.Filewatch}",
		"%{IncludeDir.zlib}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/pxshared",
		"%{IncludeDir.PhysX}/physx",

		"%{IncludeDir.SharedStorage}"
	}

	links 
	{
--		"Ruby",
		"ImGui",
		"SPIRV-Cross",
		"yaml-cpp",
		"Tracy",
		"zlib",

		"Saturn-SharedStorage"
	}

	filter "files:Saturn/vendor/ImGuizmo/src/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:not windows"
		systemversion "latest"
		cppdialect "C++2a"
		
	filter "system:linux"
		systemversion "latest"
		cppdialect "C++2a"

		links 
		{
			"pthread",
			"dl",
			"m",
			"X11",
			"Xrandr",
			"vulkan",
			"vulkan-1"
		}

		defines
		{
			"SAT_PLATFORM_LINUX"
		}


	filter "system:windows"
		systemversion "latest"

		links 
		{
			"dwmapi",
			"Saturn/vendor/vulkan/bin/vulkan-1.lib"
		}

		defines
		{
			"SAT_PLATFORM_WINDOWS"
		}

		filter "configurations:Debug"
			defines "SAT_DEBUG"
			runtime "Debug"
			symbols "on"

			links 
			{
				"Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/shaderc.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/shaderc_util.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/glslangd.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/SPIRV-Tools.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/glslc.lib",

				-- PhysX
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
				"Saturn/vendor/physx/bin/Debug/SimulationController_static_64.lib",
			}

		filter "configurations:Release"
			defines "SAT_RELEASE"
			runtime "Release"
		--	optimize "on"
			links { "Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.lib" }

		filter "configurations:Dist"
			defines "SAT_DIST"
			runtime "Release"
			optimize "on"
			symbols "off"

			removelinks { "Tracy" }
			removedefines { "TRACY_ENABLE","SATURN_SS_IMPORT" }
			removefiles { "%{prj.name}/vendor/ImGuizmo/src/**.cpp", "%{prj.name}/vendor/ImGuizmo/src/**.h" }

			defines { "SATURN_SS_STATIC" }
			links { "Saturn-SharedStorage" }

		filter "configurations:Release or configurations:Dist"
			links 
			{
				"Saturn/vendor/shaderc/bin/Release-Windows/shaderc.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/shaderc_util.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/glslang.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/SPIRV-Tools.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/glslc.lib",

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

---------------------------------------------------------------------------------------------------------------------------

group "Editor"
project "Saturn-Editor"
	location "Saturn-Editor"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	warnings "Default"
	kind "ConsoleApp"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"SAT_HAS_EDITOR"
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
--		"%{IncludeDir.Ruby}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.glslc}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.SPIRV_Cross}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/pxshared",
		"%{IncludeDir.PhysX}/physx",
		"%{IncludeDir.Optick}",
		"Saturn/vendor/vulkan/include",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.ImSpinner}",
		"%{IncludeDir.Filewatch}",
		"%{IncludeDir.MiniAudio}",
		"%{IncludeDir.ImguiNodeEditor}",
		"%{IncludeDir.Tracy}",

		"%{IncludeDir.SharedStorage}"
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
			"SATURN_SS_IMPORT"
		}

	filter "configurations:Debug"
		defines "SAT_DEBUG"
		runtime "Debug"
		symbols "on"

		postbuildcommands 
		{ 
			'{COPYFILE} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../bin/Debug-windows-x86_64/Saturn-SharedStorage/Saturn-SharedStorage.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../Saturn/vendor/physx/bin/Debug/PhysXCommon_64.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../Saturn/vendor/physx/bin/Debug/PhysXFoundation_64.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../Saturn/vendor/physx/bin/Debug/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../Saturn/vendor/physx/bin/Debug/PhysX_64.dll" "%{cfg.targetdir}"',
		}

	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
	--	optimize "on"

		postbuildcommands 
		{ 
			'{COPYFILE} "../bin/Release-windows-x86_64/Saturn-SharedStorage/Saturn-SharedStorage.dll" "%{cfg.targetdir}"',
		}

	filter "configurations:Dist"
		defines "SAT_DIST"
		runtime "Release"
		optimize "on"
		symbols "Off"
		kind "WindowedApp"

		removedefines { "SATURN_SS_IMPORT" }
		defines { "SATURN_SS_STATIC" }

	filter "configurations:Dist or configurations:Release"
		postbuildcommands 
		{ 
			'{COPYFILE} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../Saturn/vendor/physx/bin/Release/PhysXCommon_64.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../Saturn/vendor/physx/bin/Release/PhysXFoundation_64.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../Saturn/vendor/physx/bin/Release/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPYFILE} "../Saturn/vendor/physx/bin/Release/PhysX_64.dll" "%{cfg.targetdir}"',
		}

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


group "Tools"
project "Saturn-ProjectBrowser"
	location "Saturn-ProjectBrowser"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	warnings "Default"
	kind "ConsoleApp"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"SATURN_SS_IMPORT"
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
--		"%{IncludeDir.Ruby}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.DiscordRPC}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.glslc}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.SPIRV_Cross}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.ImSpinner}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/pxshared",
		"%{IncludeDir.PhysX}/physx",
		"%{IncludeDir.Optick}",
		"Saturn/vendor/vulkan/include",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.Filewatch}",
		"%{IncludeDir.MiniAudio}",
		"%{IncludeDir.SharedStorage}"
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

			postbuildcommands 
			{
				'{COPYFILE} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"',
				
				'{COPYFILE} "../bin/Debug-windows-x86_64/Saturn-SharedStorage/Saturn-SharedStorage.dll" "%{cfg.targetdir}"'
			}

		filter "configurations:Release"
			defines "SAT_RELEASE"
			runtime "Release"
			optimize "on"

			postbuildcommands 
			{ 
				'{COPYFILE} "../bin/Release-windows-x86_64/Saturn-SharedStorage/Saturn-SharedStorage.dll" "%{cfg.targetdir}"'
			}

		filter "configurations:Dist"
			defines "SAT_DIST"
			runtime "Release"
			optimize "on"
			symbols "Off"
			kind "WindowedApp"

		filter "configurations:Release or configurations:Dist"
			postbuildcommands 
			{ 
				'{COPYFILE} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			}

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

group "Tools"
project "SaturnBuildTool"
	location "SaturnBuildTool"
	language "C#"
	kind "ConsoleApp"
	links { "System", "System.Xml" }

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.cs"
	}

	postbuildcommands
	{
		'{COPYFILE} "../../../SaturnBuildTool/RT" "RT"'
	}

	filter { "configurations:Debug" }
		symbols "On"

 	filter { "configurations:Release" }
		optimize "On"

	filter { "configurations:Dist" }
		optimize "On"

		
group "Tools"
project "Saturn-SharedStorage"
	location "Saturn-SharedStorage"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	warnings "Default"
		
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"Saturn/src",
--		"%{IncludeDir.Ruby}"
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
			kind "StaticLib"
			defines { "SATURN_SS_STATIC" }
