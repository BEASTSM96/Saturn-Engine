#pragma once

#include "Sparky\Core.h"

#include <fstream>

#include <vector>

namespace Sparky {
	class FileCreation
	{
	public:
		FileCreation()  = default;
		virtual ~FileCreation() {};

		static void NewProjectFile();
		//TODO Add NewFile func
		//virtual void NewFile() {};
	};


	class FileReader
	{
	public:
		FileReader() = default;
		virtual ~FileReader() = default;

		static void ReadFile(std::string fileName, std::string fileLine);
	};
}

