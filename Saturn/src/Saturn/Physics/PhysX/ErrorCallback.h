/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Log.h"

#ifdef USE_NVIDIA

#include <pxshared/foundation/PxErrorCallback.h>

namespace Saturn {

	class PhysXErrorCallback : public physx::PxErrorCallback
	{
	public:
		virtual void reportError( physx::PxErrorCode::Enum code, const char* message, const char* file,
			int line )
		{
			switch( code )
			{
				case physx::PxErrorCode::Enum::eDEBUG_INFO:
					SAT_CORE_INFO( "{0} {1} {2}", message, file, line );
					break;

				case physx::PxErrorCode::Enum::eDEBUG_WARNING:
					SAT_CORE_WARN( "{0} {1} {2}", message, file, line );
					break;

				case physx::PxErrorCode::Enum::eOUT_OF_MEMORY:
					SAT_CORE_FATAL( "Out of Memory! {0}", message );
					break;

				case physx::PxErrorCode::Enum::eINVALID_PARAMETER:
					SAT_CORE_ERROR( "Invalid parameter! {0}", message );
					break;

				case physx::PxErrorCode::Enum::eABORT:
					SAT_CORE_ERROR( "{0} {1} {2}", message, file, line );
					break;

				case physx::PxErrorCode::Enum::ePERF_WARNING:
					SAT_CORE_WARN( "{0} {1} {2}", message, file, line );
					break;

				case physx::PxErrorCode::Enum::eINTERNAL_ERROR:
					SAT_CORE_ERROR( "{0} {1} {2}", message, file, line );
					break;

				case physx::PxErrorCode::Enum::eINVALID_OPERATION:
					SAT_CORE_ERROR( "{0} {1} {2}", message, file, line );
					break;

				case physx::PxErrorCode::Enum::eMASK_ALL:
					SAT_CORE_INFO( "{0} {1} {2}", message, file, line );
					break;

				case physx::PxErrorCode::Enum::eNO_ERROR:
					SAT_CORE_INFO( "{0} {1} {2}", message, file, line );
					break;


				default:
					SAT_CORE_INFO( "{0} {1} {2}", message, file, line );
					break;
			}
		}
	};
}

#endif