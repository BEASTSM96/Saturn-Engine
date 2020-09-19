project "Assimp"
	kind				"StaticLib"
	language			"C++"
	postbuildcommands	"{COPY} bin/assimp-vc142-mt.lib %{cfg.targetdir}"


	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/**.h",
		"include/**.hpp",
		"include/assimp/**.h",
		"include/assimp/**.hpp",
		"include/assimp/**.inl",
	}

	includedirs {
	
		"include/"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"