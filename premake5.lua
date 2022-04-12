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
IncludeDir["DiscordRPC"] = "Saturn/vendor/discord-rpc/include"
IncludeDir["discord-rpc"] = "Saturn/vendor/discord-rpc/include"
IncludeDir["rapidjson"] = "Saturn/vendor/rapidjson/include"
IncludeDir["shaderc"] = "Saturn/vendor/shaderc/libshaderc/include"
IncludeDir["glslc"] = "Saturn/vendor/shaderc/glslc/src"

group "sat/Dependencies"
	include "Saturn/vendor/GLFW"
	include "Saturn/vendor/imgui"
	include "Saturn/vendor/discord-rpc"
	include "Saturn/vendor/SPIRV-Cross"

group "sat/Core"
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
		"%{prj.name}/vendor/d3d12/**.h",
		"%{prj.name}/vendor/vulkan/**.h",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"PX_PHYSX_STATIC_LIB",
		"PX_GENERATE_STATIC_LIBRARIES",
		"AL_LIBTYPE_STATIC",
		"GLM_ENABLE_EXPERIMENTAL",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_SWIZZLE",
		"GLM_FORCE_LEFT_HANDED"
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
		"%{IncludeDir.shaderc}"
	}

	links 
	{ 
		"GLFW",
		"ImGui",
		"SPIRV-Cross",
		"discord-rpc"
	}

	filter "system:windows"
		systemversion "latest"

		links 
		{
			"dwmapi",
			"opengl32.lib",
			"d3d12.lib",
			"dxgi.lib",
			"d3dcompiler.lib",
			"%{prj.name}/vendor/vulkan/bin/vulkan-1.lib"
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

group "sat/Tools"
project "Titan"
	location "Titan"
	-- kind "WindowedApp"
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
		"%{IncludeDir.DiscordRPC}"
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
