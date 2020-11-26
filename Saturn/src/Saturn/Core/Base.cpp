#include "sppch.h"
#include "Base.h"

//#include "Saturn/Log.h"

namespace Saturn {

	void InitializeCore()
	{
		Saturn::Log::Init();

		//SAT_CORE_TRACE("Saturn Engine");
		//SAT_CORE_TRACE("Initializing...");
	}

	void ShutdownCore()
	{
		//SAT_CORE_TRACE("Shutting down...");
	}

}