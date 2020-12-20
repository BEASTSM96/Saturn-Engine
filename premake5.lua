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
IncludeDir["ReactPhysics3D"] = "Saturn/vendor/reactphysics3d/include"

-- Game
GameDir = {}
GameDir["Game"] = "Game/"

IncludeDir["yaml_cpp"] = "Saturn/vendor/yaml-cpp/include"
IncludeDir["json_cpp"] = "Saturn/vendor/jsoncpp/"
IncludeDir["Saturn-Serialisation"] = "Saturn/vendor/Saturn-Serialisation/"

group "sat/Dependencies"
	include "Saturn/vendor/GLFW"
	include "Saturn/vendor/Glad"
	include "Saturn/vendor/imgui"
	include "Saturn/vendor/assimp"
	include "Saturn/vendor/SPIRV_Cross"
	include "Saturn/vendor/reactphysics3d"
	group "sat/Dependencies/Serialisation"
			include "Saturn/vendor/jsoncpp"
			include "Saturn/vendor/yaml-cpp"
--			include "Saturn/vendor/Saturn-Serialisation"

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
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ReactPhysics3D}",
		"Saturn/vendor/yaml-cpp/include",
		"Saturn/vendor/SaturnLog/SaturnLogging/src"
	}


	disablewarnings 
	{ 
		"4001",
		"4002",
		"4003",
		"4005",
		"4006",
		"4007",
		"4008",
		"4010",
		"4013",
		"4015",
		"4018",
		"4019",
		"4020",
		"4022",
		"4023",
		"4024",
		"4025",
		"4026",
		"4027",
		"4028",
		"4029",
		"4030",
		"4031",
		"4032",
		"4033",
		"4034",
		"4035",
		"4036",
		"4038",
		"4041",
		"4042",
		"4045",
		"4047",
		"4048",
		"4049",
		"4051",
		"4052",
		"4053",
		"4055",
		"4056",
		"4057",
		"4060",
		"4061",
		"4062",
		"4063",
		"4064",
		"4065",
		"4066",
		"4067",
		"4068",
		"4069",
		"4073",
		"4074",
		"4075",
		"4076",
		"4077",
		"4079",
		"4080",
		"4081",
		"4083",
		"4085",
		"4086",
		"4087",
		"4088",
		"4089",
		"4090",
		"4091",
		"4092",
		"4094",
		"4096",
		"4097",
		"4098",
		"4099",
		"4100",
		"4101",
		"4102",
		"4103",
		"4109",
		"4112",
		"4113",
		"4114",
		"4115",
		"4116",
		"4117",
		"4119",
		"4120",
		"4121",
		"4122",
		"4123",
		"4124",
		"4125",
		"4127",
		"4129",
		"4130",
		"4131",
		"4132",
		"4133",
		"4137",
		"4138",
		"4141",
		"4142",
		"4143",
		"4144",
		"4145",
		"4146",
		"4150",
		"4152",
		"4153",
		"4154",
		"4155",
		"4156",
		"4157",
		"4158",
		"4159",
		"4160",
		"4161",
		"4162",
		"4163",
		"4164",
		"4165",
		"4166",
		"4167",
		"4168",
		"4172",
		"4174",
		"4175",
		"4176",
		"4177",
		"4178",
		"4179",
		"4180",
		"4181",
		"4182",
		"4183",
		"4185",
		"4186",
		"4187",
		"4188",
		"4189",
		"4190",
		"4191",
		"4192",
		"4193",
		"4194",
		"4195",
		"4196",
		"4197",
		"4200",
		"4201",
		"4202",
		"4203",
		"4204",
		"4205",
		"4206",
		"4207",
		"4208",
		"4210",
		"4211",
		"4212",
		"4213",
		"4214",
		"4215",
		"4216",
		"4218",
		"4220",
		"4221",
		"4223",
		"4224",
		"4226",
		"4227",
		"4228",
		"4229",
		"4230",
		"4232",
		"4233",
		"4234",
		"4235",
		"4237",
		"4238",
		"4239",
		"4240",
		"4242",
		"4243",
		"4244",
		"4244",
		"4245",
		"4250",
		"4251",
		"4254",
		"4255",
		"4256",
		"4258",
		"4263",
		"4264",
		"4265",
		"4266",
		"4267",
		"4268",
		"4269",
		"4272",
		"4273",
		"4274",
		"4275",
		"4276",
		"4277",
		"4278",
		"4279",
		"4280",
		"4281",
		"4282",
		"4283",
		"4285",
		"4286",
		"4287",
		"4288",
		"4289",
		"4290",
		"4291",
		"4293",
		"4295",
		"4296",
		"4297",
		"4298",
		"4299",
		"4301",
		"4302",
		"4303",
		"4305",
		"4306",
		"4307",
		"4308",
		"4309",
		"4310",
		"4311",
		"4312",
		"4313",
		"4314",
		"4315",
		"4316",
		"4317",
		"4318",
		"4319",
		"4321",
		"4322",
		"4323",
		"4324",
		"4325",
		"4326",
		"4327",
		"4328",
		"4329",
		"4330",
		"4333",
		"4334",
		"4335",
		"4336",
		"4337",
		"4338",
		"4339",
		"4340",
		"4342",
		"4343",
		"4344",
		"4346",
		"4348",
		"4350",
		"4352",
		"4353",
		"4355",
		"4356",
		"4357",
		"4358",
		"4359",
		"4362",
		"4364",
		"4365",
		"4366",
		"4367",
		"4368",
		"4369",
		"4370",
		"4371",
		"4373",
		"4374",
		"4375",
		"4376",
		"4377",
		"4378",
		"4379",
		"4380",
		"4381",
		"4382",
		"4383",
		"4384",
		"4387",
		"4388",
		"4389",
		"4390",
		"4391",
		"4392",
		"4393",
		"4394",
		"4395",
		"4396",
		"4397",
		"4398",
		"4399",
		"4600",
		"4400",
		"4401",
		"4402",
		"4403",
		"4404",
		"4405",
		"4406",
		"4407",
		"4408",
		"4409",
		"4410",
		"4411",
		"4412",
		"4413",
		"4414",
		"4415",
		"4416",
		"4417",
		"4418",
		"4419",
		"4420",
		"4421",
		"4423",
		"4424",
		"4425",
		"4426",
		"4427",
		"4429",
		"4430",
		"4431",
		"4434",
		"4435",
		"4436",
		"4437",
		"4438",
		"4439",
		"4440",
		"4441",
		"4442",
		"4443",
		"4444",
		"4445",
		"4446",
		"4447",
		"4448",
		"4449",
		"4450",
		"4451",
		"4452",
		"4453",
		"4454",
		"4455",
		"4456",
		"4457",
		"4458",
		"4459",
		"4460",
		"4461",
		"4462",
		"4463",
		"4464",
		"4470",
		"4471",
		"4472",
		"4473",
		"4474",
		"4475",
		"4476",
		"4477",
		"4478",
		"4480",
		"4481",
		"4482",
		"4483",
		"4484",
		"4485",
		"4486",
		"4487",
		"4488",
		"4489",
		"4490",
		"4491",
		"4492",
		"4493",
		"4494",
		"4495",
		"4496",
		"4497",
		"4498",
		"4499",
		"4502",
		"4503",
		"4505",
		"4506",
		"4508",
		"4509",
		"4510",
		"4511",
		"4512",
		"4513",
		"4514",
		"4515",
		"4516",
		"4517",
		"4518",
		"4519",
		"4521",
		"4522",
		"4523",
		"4526",
		"4530",
		"4531",
		"4532",
		"4533",
		"4534",
		"4535",
		"4536",
		"4537",
		"4538",
		"4540",
		"4541",
		"4542",
		"4543",
		"4544",
		"4545",
		"4546",
		"4547",
		"4548",
		"4549",
		"4550",
		"4551",
		"4552",
		"4553",
		"4554",
		"4555",
		"4556",
		"4557",
		"4558",
		"4559",
		"4561",
		"4562",
		"4564",
		"4565",
		"4566",
		"4568",
		"4569",
		"4570",
		"4571",
		"4572",
		"4573",
		"4574",
		"4575",
		"4576",
		"4577",
		"4578",
		"4580",
		"4581",
		"4582",
		"4583",
		"4584",
		"4585",
		"4586",
		"4587",
		"4588",
		"4591",
		"4592",
		"4593",
		"4594",
		"4595",
		"4596",
		"4597",
		"4598",
		"4599",
		"4600",
		"4602",
		"4603",
		"4604",
		"4605",
		"4606",
		"4608",
		"4609",
		"4610",
		"4611",
		"4612",
		"4613",
		"4615",
		"4616",
		"4618",
		"4619",
		"4620",
		"4621",
		"4622",
		"4623",
		"4624",
		"4625",
		"4626",
		"4627",
		"4628",
		"4629",
		"4630",
		"4631",
		"4632",
		"4633",
		"4634",
		"4635",
		"4636",
		"4637",
		"4638",
		"4639",
		"4640",
		"4641",
		"4645",
		"4646",
		"4647",
		"4648",
		"4649",
		"4650",
		"4651",
		"4652",
		"4653",
		"4654",
		"4655",
		"4656",
		"4657",
		"4658",
		"4659",
		"4661",
		"4662",
		"4667",
		"4668",
		"4669",
		"4670",
		"4671",
		"4672",
		"4673",
		"4674",
		"4676",
		"4677",
		"4678",
		"4679",
		"4680",
		"4681",
		"4682",
		"4683",
		"4684",
		"4685",
		"4686",
		"4687",
		"4688",
		"4689",
		"4690",
		"4691",
		"4692",
		"4693",
		"4694",
		"4695",
		"4696",
		"4700",
		"4701",
		"4702",
		"4703",
		"4706",
		"4709",
		"4710",
		"4711",
		"4714",
		"4715",
		"4716",
		"4717",
		"4718",
		"4719",
		"4720",
		"4721",
		"4722",
		"4723",
		"4724",
		"4725",
		"4727",
		"4728",
		"4729",
		"4730",
		"4731",
		"4732",
		"4733",
		"4738",
		"4739",
		"4740",
		"4742",
		"4743",
		"4744",
		"4746",
		"4747",
		"4749",
		"4750",
		"4751",
		"4752",
		"4754",
		"4755",
		"4756",
		"4757",
		"4764",
		"4767",
		"4768",
		"4770",
		"4771",
		"4772",
		"4774",
		"4775",
		"4776",
		"4777",
		"4778",
		"4788",
		"4789",
		"4792",
		"4793",
		"4794",
		"4799",
		"4800",
		"4803",
		"4804",
		"4805",
		"4806",
		"4807",
		"4808",
		"4809",
		"4810",
		"4811",
		"4812",
		"4813",
		"4816",
		"4817",
		"4819",
		"4820",
		"4821",
		"4822",
		"4823",
		"4826",
		"4827",
		"4829",
		"4835",
		"4837",
		"4838",
		"4839",
		"4840",
		"4841",
		"4842",
		"4843",
		"4844",
		"4845",
		"4846",
		"4847",
		"4848",
		"4854",
		"4855",
		"4856",
		"4857",
		"4866",
		"4867",
		"4868",
		"4872",
		"4880",
		"4881",
		"4882",
		"4900",
		"4905",
		"4906",
		"4910",
		"4912",
		"4913",
		"4916",
		"4917",
		"4918",
		"4920",
		"4921",
		"4925",
		"4926",
		"4927",
		"4928",
		"4929",
		"4930",
		"4931",
		"4932",
		"4934",
		"4935",
		"4936",
		"4937",
		"4938",
		"4939",
		"4944",
		"4945",
		"4946",
		"4947",
		"4948",
		"4949",
		"4950",
		"4951",
		"4952",
		"4953",
		"4954",
		"4955",
		"4956",
		"4957",
		"4958",
		"4959",
		"4960",
		"4961",
		"4962",
		"4963",
		"4964",
		"4965",
		"4966",
		"4970",
		"4971",
		"4972",
		"4973",
		"4974",
		"4981",
		"4984",
		"4985",
		"4986",
		"4987",
		"4988",
		"4989",
		"4990",
		"4991",
		"4992",
		"4995",
		"4996",
		"4997",
		"4998",
		"4999",
		"5022",
		"5023",
		"5024",
		"5025",
		"5026",
		"5027",
		"5028",
		"5029",
		"5030",
		"5031",
		"5032",
		"5033",
		"5034",
		"5035",
		"5036",
		"5037",
		"5038",
		"5039",
		"5040",
		"5041",
		"5042",
		"5043",
		"5044",
		"5045",
		"5046",
		"5047",
		"5048",
		"5049",
		"5050",
		"5051",
		"5052",
		"5053",
		"5054",
		"5055",
		"5056",
		"5057",
		"5058",
		"5059",
		"5060",
		"5061",
		"5062",
		"5063",
		"5100",
		"5101",
		"5102",
		"5103",
		"5104",
		"5105",
		"5106",
		"5107",
		"5108",
		"5200",
		"5201",
		"5202",
		"5203",
		"5204",
		"5205",
		"5206",
		"5207",
		"5208",
		"5209",
		"5210",
		"5212",
		"5213",
		"5214",
		"5215",
		"5216",
		"5217",
		"5218",
		"5219",
		"5220",
		"5221"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		"Jsoncpp",
		"ReactPhysics3D"
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
			"Saturn/vendor/yaml-cpp/bin/Debug/yaml-cpp.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "%{cfg.targetdir}"',
			'{COPY} "../Saturn/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll" "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Titan/"',
		}


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
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ReactPhysics3D}",
		"Saturn/vendor/yaml-cpp/include",
		"Saturn/vendor/glm/",
		"%{IncludeDir.SPIRV_Cross}"
	}

	links
	{
		"Saturn"
	}

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


group "Runtime"
	include "Game"