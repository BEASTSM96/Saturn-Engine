workspace "Saturn"
	architecture "x64"
	startproject "Titan"
	targetdir "build"

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
IncludeDir["PhysX"] = "Saturn/vendor/PhysX/include"

IncludeDir["yaml_cpp"] = "Saturn/vendor/yaml-cpp/include"
IncludeDir["json_cpp"] = "Saturn/vendor/jsoncpp/"
IncludeDir["Saturn-Serialisation"] = "Saturn/vendor/Saturn-Serialisation/"

group "sat/Dependencies"
	include "Saturn/vendor/GLFW"
	include "Saturn/vendor/Glad"
	include "Saturn/vendor/imgui"
	include "Saturn/vendor/assimp"
	include "Saturn/vendor/SPIRV_Cross"
<<<<<<< HEAD
	include "Saturn/vendor/PhysX"
	
=======
		group "sat/Dependencies/Serialisation"
			include "Saturn/vendor/jsoncpp"
			include "Saturn/vendor/yaml-cpp"
--			include "Saturn/vendor/Saturn-Serialisation"

>>>>>>> 644c83827fd721a0e3c31e9f46d48e0dc38bf366
group "sat/Core"
project "Saturn"
	location "Saturn"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"%{IncludeDir.json_cpp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
			"%{prj.name}/vendor/assimp/include/",
		"%{IncludeDir.SPIRV_Cross}",
<<<<<<< HEAD
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/pxshared",
		"%{IncludeDir.PhysX}/physx"
=======
		"%{IncludeDir.yaml_cpp}"
>>>>>>> 644c83827fd721a0e3c31e9f46d48e0dc38bf366
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		"Jsoncpp"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SAT_PLATFORM_WINDOWS",
			"SAT_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"SPARKY_GAME_BASE"
		}

	filter "configurations:Debug"
		defines "SAT_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.lib",
<<<<<<< HEAD
			"Saturn/vendor/physx/bin/Debug/PhysX_64.lib"
		}

=======
			"Saturn/vendor/yaml-cpp/bin/Debug/yaml-cpp.lib",
		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}


>>>>>>> 644c83827fd721a0e3c31e9f46d48e0dc38bf366
	filter "configurations:Release"
		defines "SAT_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.lib"
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
			"Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}

---------------------------------------------------------------------------------------------------------------------------

group "sat/Core"
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
		"Saturn/vendor/spdlog/include",
		"Saturn/src",
		"%{IncludeDir.json_cpp}",
		"Saturn/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.Assimp}"
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

group "sat/Tools"
project "Titan"
	location "Titan"
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
		"Saturn/vendor/spdlog/include",
		"Saturn/src",
		"Saturn/vendor",
		"%{IncludeDir.json_cpp}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/pxshared",
		"%{IncludeDir.PhysX}/physx"
	}

	links
	{
		"Saturn"
	}

<<<<<<< HEAD
=======
	postbuildcommands 
	{
		'{COPY} "../%{prj.name}/assets/" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/assets/"'
	}

	filter "configurations:Dist"
		postbuildcommands 
		{
			'{COPY} "Saturn/vendor/assimp/bin/Dist/assimp-vc142-mt.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/"'
		}
	filter "configurations:Release"
		postbuildcommands 
		{
			'{COPY} "Saturn/vendor/assimp/bin/Release/assimp-vc142-mt.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/"'
		}
	filter "configurations:Debug"
		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/"'
		}

>>>>>>> 644c83827fd721a0e3c31e9f46d48e0dc38bf366
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