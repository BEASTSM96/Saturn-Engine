project "SPIRV-Cross"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	includedirs
	{
		"src/"
	}

	files
	{
		"src/**.cpp",
		"src/**.h",
		"src/**.hpp"
	}

	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
