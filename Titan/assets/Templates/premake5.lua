-- Game Project premake template.
-- __PROJECT_NAME__

workspace "__PROJECT_NAME__"
	architecture "x64"
	startproject "__PROJECT_NAME__"
	warnings "Off"

	configurations { "Debug", "Release", "Dist" }

	flags { "MultiProcessorCompile" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["Saturn"] = "__SATURN_DIR__"
--IncludeDir["glm"] = "__SATURN_VENDOR__glm"
IncludeDir["spdlog"] = "__SATURN_VENDOR__spdlog/include"
IncludeDir["vulkan"] = "__SATURN_VENDOR__vulkan/include"
--IncludeDir["vma"] = "__SATURN_VENDOR__vma/src/"
--IncludeDir["PhysX"] = "__SATURN_VENDOR__physx/include" -- Really?
--IncludeDir["entt"] = "__SATURN_VENDOR__entt/include"

IncludeDir["GLFW"] = "__SATURN_VENDOR__GLFW/include"
IncludeDir["ImGui"] = "__SATURN_VENDOR__imgui"
IncludeDir["glm"] = "__SATURN_VENDOR__glm"
IncludeDir["entt"] = "__SATURN_VENDOR__entt/include"
IncludeDir["assimp"] = "__SATURN_VENDOR__assimp/include"
IncludeDir["shaderc"] = "__SATURN_VENDOR__shaderc/libshaderc/include"
IncludeDir["glslc"] = "__SATURN_VENDOR__shaderc/glslc/src"
IncludeDir["SPIRV_Cross"] = "__SATURN_VENDOR__SPIRV-Cross/src/"
IncludeDir["vma"] = "__SATURN_VENDOR__vma/src/"
IncludeDir["ImGuizmo"] = "__SATURN_VENDOR__ImGuizmo/src/"
IncludeDir["yaml_cpp"] = "__SATURN_VENDOR__yaml-cpp/include/"
IncludeDir["PhysX"] = "__SATURN_VENDOR__physx/include"
IncludeDir["ImguiNodeEditor"] = "__SATURN_VENDOR__imgui_node_editor"
IncludeDir["SingletonStorage"] = "__SATURN_SINGLETON_DIR__"

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
		"%{prj.name}/Scripts/**.h",
		"%{prj.name}/Scripts/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/Scripts",
		"%{IncludeDir.Saturn}",
		"%{IncludeDir.vulkan}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/pxshared",
		"%{IncludeDir.PhysX}/physx"
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
			optimize "on"
			symbols "Off"

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