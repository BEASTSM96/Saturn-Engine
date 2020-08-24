#include "sppch.h"
#include "Layer.h"



namespace Sparky {

	Layer::Layer(const std::string& debugName)
		: m_DebugName(debugName), OBJ_NAME(debugName)
	{
		archive();
	}

	Layer::~Layer()
	{

	}
}

