#pragma once

#include "Saturn/Core/Timestep.h"
#include "Saturn/GameFramework/GameScript.h"

class __FILE_NAME__ : public Saturn::SClass
{
public:
	__FILE_NAME__();
	~__FILE_NAME__();

	virtual void BeginPlay() override;
	virtual void OnUpdate() override;

private:

};

SATURN_REGISTER_SCRIPT( __FILE_NAME__ );