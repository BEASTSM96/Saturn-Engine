#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Scene/Entity.h"
#include <string>

#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/object.h>

namespace Saturn {

	class ScriptRegistry 
	{
	public:
		static void RegisterAll();
	};

}