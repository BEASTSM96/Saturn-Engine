#pragma once

/*
*					Sparky World
*********************************************************************************************************
*										Object (needed for serialisation) ?.
*/

/*
*					World is a "no data" class SATURN_API Level.h has all of the funcs in it.
*/

#include <string>
#include "Saturn/GameBase/GameObject.h"
#include "Saturn/GameBase/GameLayer.h"

namespace Saturn {

	//Should a world be a Object?
	class SATURN_API World
	{
	public:
		/** Default deconstructor */
		virtual ~World() = default;

		virtual void* GetLevel() = 0;
		virtual std::string GetLevelName() = 0;
		virtual uint64_t GetAllGameObjects() = 0;

		virtual void* GetGameLayer() = 0;
		virtual float GetID() = 0;

	private:

	};
}