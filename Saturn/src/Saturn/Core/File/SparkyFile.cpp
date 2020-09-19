#include "sppch.h"
#include "Saturn/Core/File/SparkyFile.h"

#include <fstream>

#include <vector>

#include <string>

namespace Saturn {
	void FileCreation::NewProjectFile()
	{

		std::string string = "good3dgame.sproject";

		std::ofstream file(string);

		std::string ProjectNameTemplate = R"({
	"FileVersion": 0,
	"EngineAssociation": "0.01",
	"Category": "",
	"Description": "",
	"Modules": [
		{
			"Name": "",
			"Type": "Runtime",
			"LoadingPhase": "Default",
			"AdditionalDependencies": [
				"Engine"
			]
		}
	],
	"TargetPlatforms": [
		"Windows"
	]

	testReader
}
)";
		if (file)
		{
		 file << ProjectNameTemplate << std::endl;

			file.open(string);
			if (file.is_open())
			{
				SAT_CORE_INFO("File Opened");

			}
			file.close();
		}

	}

	void FileReader::ReadFile(std::string fileName, std::string fileLine)
	{
		std::ifstream file (fileName);
		std::getline (file, fileLine);
		std::cout << fileLine << "\n";
	}

}