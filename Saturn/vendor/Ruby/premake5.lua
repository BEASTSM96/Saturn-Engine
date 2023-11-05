project "Ruby"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/%{prj.name}/**.h",
		"src/%{prj.name}/**.cpp"
	}

	defines
	{
		"RBY_INCLUDE_VULKAN"
	}

	includedirs
	{
		"src/",
		"../vulkan/include"
	}

	filter "system:windows"
		systemversion "latest"

		links 
		{
			"dwmapi",
			"opengl32"
		}

		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			runtime "Release"
			optimize "on"


	filter "system:linux"
		systemversion "latest"

		links 
		{
			"stdc++fs",
			"pthread",
			"dl",
			"xcb",
			"X11",
			"X11-xcb",
			"GL"
		}

		defines
		{
		}

		filter "configurations:Debug"
			defines "_DEBUG"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines "_RELEASE"
			runtime "Release"
			optimize "on"