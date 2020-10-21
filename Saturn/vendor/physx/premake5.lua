project "PhysX"
	kind				"StaticLib"
	language			"C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    warnings "Off"
    characterset "MBCS"

    defines {
		"PX_PHYSX_STATIC_LIB",
		"PX_PHYSX_STATIC_LIB",
    	"PX_COOKING"
    }

	files {
		"include/**.h",
        "include/*/**.h",
        "source/*/**.cpp",
        "source/*/**.h"
	}

	includedirs {
        "include/physx/",
        "include/pxshared/",
        "source/**/*/",
    }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"
		removefiles {
			"source/foundation/src/unix/**",
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"