project "PhysX"
	kind				"StaticLib"
	language			"C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"include/**.h",
        "include/*/**.h",
        "source/*/**.cpp",
        "source/*/**.h"
	}

	includedirs {
        "include/",
        "source/",
        "source/*/", --source/
        "source/*/src",--source/*/src
        "source/*/src/**.*",--source/*/src
        "source/*/src/*/**.*",--source/*/src/*/*.*
        "source/*/src/**.h",     
        "source/*/src/**.cpp",

        "source/*/include/**.*",--source/*/include
        "source/*/include/*/*.*",--source/*/include/*/*.*
        "source/*/include/**.h",     
        "source/*/include/**.cpp",
        "source/*/include/*.h",     
        "source/*/include/*.cpp"
    
    }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"