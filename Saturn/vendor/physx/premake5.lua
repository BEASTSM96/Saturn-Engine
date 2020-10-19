-- workspace "PhysX"
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

	local function declSDKModule( name )
		project( name )
		kind "StaticLib"
		location ""
		characterset "MBCS"
	
		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		flags {
			"MultiProcessorCompile",
		}
	
		defines {
			"PX_PHYSX_STATIC_LIB",
			"PX_COOKING",
			"PX_SUPPORT_PVD",
		}

		includedirs { 
			
			"source/", 

			"include/" ,
			"include/**",
			"include/characterkinematic/**.h" ,
			"include/collision/**.h" ,
			"include/common/**.h" ,
			"include/common/**/**.h" ,
			"include/cooking/**.h" ,
			"include/cudamanager/**.h" ,
			"include/extensions/**.h" ,
			"include/filebuf/**.h" ,
			"include/foundation/**.h" ,
			"include/geometry/**.h" ,
			"include/geomutils/**.h" ,
			"include/gpu/**.h" ,
			"include/pvd/**.h" ,
			"include/solver/**.h" ,
			"include/task/**.h" ,
			"include/vehicle/**.h",

			------------------------------
			"source/common/include/windows/**.h", 
			"source/common/src/**.cpp", 
			"source/common/src/**/**.cpp", 

			"source/fastxml/include/**.h", 
			"source/fastxml/src/**.cpp", 

			"source/filebuf/include/**.h", 
			"source/filebuf/include/**.inl", 

			"source/foundation/include/**/**.h", 
			"source/foundation/include/**.h", 
			"source/foundation/src/**.cpp", 
			"source/foundation/src/**/**.cpp", 

			"source/geomutils/include/**.h", 
			"source/geomutils/src/**.cpp", 
			"source/geomutils/src/**.h", 
			"source/geomutils/src/**/**.cpp", 
			"source/geomutils/src/**/**.h",

			"source/immediatemode/src/**.cpp", 

			"source/lowlevel/api/include/**.h", 
			"source/lowlevel/api/**.cpp", 
			"source/lowlevel/common/include/**/**.h", 
			"source/lowlevel/common/src/**.cpp", 
			"source/lowlevel/software/include/**.h", 
			"source/lowlevel/software/src/**.cpp", 

			"source/lowlevelaabb/src/**.cpp", 
			"source/lowlevelaabb/include/**.h", 

			"source/lowleveldynamics/include/**.h", 
			"source/lowleveldynamics/src/**.cpp", 

			"source/physx/src/**.cpp", 
			"source/physx/src/**.h", 
			"source/physx/src/**/**.cpp", 
			"source/physx/src/**/**.h", 

			"source/physxcharacterkinematic/src/**.cpp", 
			"source/physxcharacterkinematic/src/**.h", 

			"source/physxcooking/src/**.cpp", 
			"source/physxcooking/src/**.h", 
			"source/physxcooking/src/**/**.cpp", 
			"source/physxcooking/src/**/**.h", 

			"source/physxextensions/src/**.cpp", 
			"source/physxextensions/src/**.h", 
			"source/physxextensions/src/**/**.cpp", 
			"source/physxextensions/src/**/**.h", 

			"source/physxgpu/include/**.h", 

			"source/physxmetadata/core/include/**.h", 
			"source/physxmetadata/core/src/**.cpp", 
			"source/physxmetadata/extensions/include/**.h", 
			"source/physxmetadata/extensions/src/**.cpp", 

			"source/physxvehicle/src/**.cpp", 
			"source/physxvehicle/src/**/**.cpp", 
			"source/physxvehicle/src/**.h", 
			"source/physxvehicle/src/**/**.h", 

			"source/pvd/include/**.h", 
			"source/pvd/src/**.cpp", 
			"source/pvd/src/**.h", 

			"source/scenequery/include/**.h", 
			"source/scenequery/src/**.cpp", 
			"source/scenequery/src/**.h", 

			"source/simulationcontroller/include/**.h", 
			"source/simulationcontroller/src/**.cpp", 
			"source/simulationcontroller/src/**.h", 

			"source/task/src/**.cpp", 
			"source/task/src/**.h", 
		}

		filter { "configurations:Debug" }
			optimize "Off"
			symbols "On"
			defines {
				"PX_CHECKED",
				"SAT_DEBUG",
			}
	
		filter { "configurations:Release" }
			optimize "Full"
			symbols "Off"
			defines {
                "NDEBUG",
                "SAT_RELEASE"
			}
	
		filter { "toolset:msc*" }
			defines {
				"_CRT_SECURE_NO_WARNINGS",
			}
	
		filter { }
	end
	
	declSDKModule "FastXml"
		files { "source/fastxml/**/*.cpp" }
	declSDKModule "LowLevel"
		files { "source/lowlevel/**/*.cpp" }
	declSDKModule "LowLevelAABB"
		files { "source/lowlevelaabb/**/*.cpp" }
	declSDKModule "LowLevelDynamics"
		files { "source/lowleveldynamics/**/*.cpp" }
	declSDKModule "PhysX"
		files { "source/physx/src/**.cpp", "source/physx/src/**.h", "source/physx/src/**/**.cpp", "source/physx/src/**/**.h" }
		removefiles { "source/physx/source/physx/src/**/linux/*" }
	declSDKModule "PhysXCharacterKinematic"
		files { "source/physxcharacterkinematic/**/*.cpp" }
	declSDKModule "PhysXCommon"
		files { "source/physxcommon/**/*.cpp" }
	declSDKModule "PhysXCooking"
		files { "source/physxcooking/**/*.cpp" }
	declSDKModule "PhysXExtensions"
		files { "source/physxextensions/**/*.cpp" }
	declSDKModule "PhysXFoundation"
		files { "source/physxfoundation/**/*.cpp" }
	declSDKModule "PhysXPvdSDK"
		files { "source/physxpvdsdk/**/*.cpp" }
	declSDKModule "PhysXTask"
		files { "source/physxtask/**/*.cpp" }
	declSDKModule "PhysXVehicle"
		files { "source/physxvehicle/**/*.cpp" }
	declSDKModule "SceneQuery"
		files { "source/scenequery/**/*.cpp" }
	declSDKModule "SimulationController"
		files { "source/simulationcontroller/**/*.cpp" }
	