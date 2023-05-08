/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Jolt/Jolt.h>

namespace Saturn {

	namespace Auxiliary {
	
		static JPH::Vec3 GLMToJPH( const glm::vec3& rVec ) 
		{
			return JPH::Vec3( rVec.x, rVec.y, rVec.z );
		}

		static glm::vec3 JPHToGLM( const JPH::Vec3& rVec )
		{
			return glm::vec3( rVec.GetX(), rVec.GetY(), rVec.GetZ() );
		}
	
		static JPH::Quat GLMQuatToJPH( const glm::quat& rQuat )
		{
			return JPH::Quat( rQuat.x, rQuat.y, rQuat.z, rQuat.w );
		}

		static glm::quat JPHQuatToGLM( const JPH::Quat& rQuat )
		{
			// glm's quats work W,X,Y,Z and not X,Y,Z,W unless we define GLM_FORCE_QUAT_DATA_XYZW
			return glm::quat( rQuat.GetW(), rQuat.GetX(), rQuat.GetY(), rQuat.GetZ() );
		}
	}

}