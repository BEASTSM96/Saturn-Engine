
project "Game"
    architecture "x64"
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
    
    kind "StaticLib"
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
		"src/**.h",
        "src/**.cpp"
    }

    includedirs
	{
		"%{prj.name}/src",
        "../Saturn/src",
		--vendor
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
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ReactPhysics3D}",
		"../Saturn/vendor/yaml-cpp/include",
		"../Saturn/vendor/glm",
		"../Saturn/vendor/spdlog/include",
		"../Saturn/vendor/Glad/include",
		"../Saturn/vendor/entt/include",
		"../Saturn/vendor/GLFW/include",
		"../Saturn/vendor/reactphysics3d/include"
	}

    links 
	{ 
		--TODO: Maybe add the engine as a link?
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