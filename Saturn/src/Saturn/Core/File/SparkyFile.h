#pragma once

#include "Saturn\Core.h"

#include <fstream>

#include <vector>

namespace Saturn {
	class SATURN_API FileCreation
	{
	public:
		FileCreation()  = default;
		virtual ~FileCreation() {};

		static void NewProjectFile();
		//TODO Add NewFile func
		//virtual void NewFile() {};
	};


	class SATURN_API FileReader
	{
	public:
		FileReader() = default;
		virtual ~FileReader() = default;

		static void ReadFile(std::string fileName, std::string fileLine);
	};
}

