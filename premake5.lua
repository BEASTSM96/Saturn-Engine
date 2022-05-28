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
IncludeDir["ImGui"] = "Saturn/vendor/imgui"
IncludeDir["glm"] = "Saturn/vendor/glm"
IncludeDir["entt"] = "Saturn/vendor/entt/include"
IncludeDir["assimp"] = "Saturn/vendor/assimp/include"
IncludeDir["shaderc"] = "Saturn/vendor/shaderc/libshaderc/include"
IncludeDir["glslc"] = "Saturn/vendor/shaderc/glslc/src"
IncludeDir["SPIRV_Cross"] = "Saturn/vendor/SPIRV-Cross/src/"
IncludeDir["SPIRV_Reflect"] = "Saturn/vendor/SPIRV-Reflect/src/"
IncludeDir["vma"] = "Saturn/vendor/vma/src/"
IncludeDir["ImGuizmo"] = "Saturn/vendor/ImGuizmo/src/"

group "Dependencies"
	include "Saturn/vendor/GLFW"
	include "Saturn/vendor/imgui"
	include "Saturn/vendor/SPIRV-Cross"
	include "Saturn/vendor/SPIRV-Reflect"

group "Engine"
project "Saturn"
	location "Saturn"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
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
		"%{prj.name}/vendor/vma/src/**.cpp",
		"%{prj.name}/vendor/vma/src/**.h",
		"%{prj.name}/vendor/d3d12/**.h",
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
		"GLM_FORCE_LEFT_HANDED",
		"GLM_FORCE_RADIANS"
		--"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		--"GLM_FORCE_SWIZZLE",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/stb/",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/d3d12/",
		"%{prj.name}/vendor/vulkan/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.DiscordRPC}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.glslc}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.SPIRV_Cross}",
		"%{IncludeDir.SPIRV_Reflect}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.ImGuizmo}"
	}

	links 
	{ 
		"GLFW",
		"ImGui",
		"SPIRV-Cross",
		"SPIRV-Reflect"
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
			"SAT_PLATFORM_LINUX",
			"GLFW_INCLUDE_NONE"
		}


	filter "system:windows"
		systemversion "latest"

		links 
		{
			"dwmapi",
			"opengl32.lib",
			"Saturn/vendor/vulkan/bin/vulkan-1.lib"
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

			links 
			{
				"Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/shaderc.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/shaderc_util.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/glslangd.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/SPIRV-Tools.lib",
				"Saturn/vendor/shaderc/bin/Debug-Windows/glslc.lib"
			}

			postbuildcommands 
			{
				'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"'
			}

		filter "configurations:Release"
			defines "SAT_RELEASE"
			runtime "Release"
			optimize "on"

			links 
			{
				"Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/shaderc.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/shaderc_util.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/glslang.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/SPIRV-Tools.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/glslc.lib"
			}

			postbuildcommands 
			{
				'{COPY} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			}

		filter "configurations:Dist"
			defines "SAT_DIST"
			runtime "Release"
			optimize "on"

			links
			{ 
				"Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/shaderc.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/shaderc_util.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/glslang.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/SPIRV-Tools.lib",
				"Saturn/vendor/shaderc/bin/Release-Windows/glslc.lib"
			}

			postbuildcommands 
			{
				'{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			}

---------------------------------------------------------------------------------------------------------------------------

group "Editor"
project "Titan"
	location "Titan"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	warnings "Off"

	filter "configurations:not Debug"
		kind "WindowedApp"
	
	filter "configurations:Debug"
		kind "ConsoleApp"

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
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.DiscordRPC}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.glslc}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.SPIRV_Cross}",
		"%{IncludeDir.SPIRV_Reflect}",
		"Saturn/vendor/vulkan/include"
	}

	links
	{
		"Saturn"
	}

	postbuildcommands 
	{
	--	'{COPY} "../Titan/assets" "%{cfg.targetdir}/assets"',
	--	'{COPY} "../Titan/imgui.ini" "%{cfg.targetdir}/imgui.ini"'
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

		postbuildcommands { '{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"' }

	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

		postbuildcommands { '{COPY} "../Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "%{cfg.targetdir}"' }

	filter "configurations:Dist"
		defines "SAT_DIST"
		runtime "Release"
		optimize "on"

		postbuildcommands { '{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "%{cfg.targetdir}"' }

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