#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Log.h"

#ifdef USE_NVIDIA

#include <pxshared/foundation/PxErrorCallback.h>

namespace Saturn {

	class UserErrorCallback : public physx::PxErrorCallback
	{
	public:
		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file,
			int line)
		{
			switch (code)
			{
			case physx::PxErrorCode::Enum::eDEBUG_INFO:
				SAT_CORE_INFO("{0}", message);
				break;

			case physx::PxErrorCode::Enum::eDEBUG_WARNING:
				SAT_CORE_WARN("{0}", message);
				break;

			case physx::PxErrorCode::Enum::eOUT_OF_MEMORY:
				SAT_CORE_FATAL("Out of Memory! {0}", message);
				break;

			case physx::PxErrorCode::Enum::eINVALID_PARAMETER:
				SAT_CORE_ERROR("Invalid parameter! {0}", message);
				break;

			case physx::PxErrorCode::Enum::eABORT:
				SAT_CORE_ERROR("{0}!", message);
				break;

			case physx::PxErrorCode::Enum::ePERF_WARNING:
				SAT_CORE_WARN("{0}!", message);
				break;

			case physx::PxErrorCode::Enum::eINTERNAL_ERROR:
				SAT_CORE_ERROR("{0}!", message);
				break;

			case physx::PxErrorCode::Enum::eINVALID_OPERATION:
				SAT_CORE_ERROR("{0}!", message);
				break;

			case physx::PxErrorCode::Enum::eMASK_ALL:
				SAT_CORE_INFO("{0}!", message);
				break;

			case physx::PxErrorCode::Enum::eNO_ERROR:
				//SAT_CORE_INFO("{0}!", message);
				break;

			default:
				break;
			}
		}
	};
}

#endif