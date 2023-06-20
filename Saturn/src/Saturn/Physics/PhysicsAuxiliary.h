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

#include "PxPhysicsAPI.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#define PHYSX_TERMINATE_ITEM( x ) if(x) x->release(); x = nullptr

namespace Saturn::Auxiliary {

	inline physx::PxVec3 GLMToPx( const glm::vec3& vec ) 
	{
		return *( physx::PxVec3* )&vec;
	}

	inline physx::PxQuat QGLMToPx( const glm::quat& quat )
	{
		// X, Y, Z, W
		return physx::PxQuat( quat.x, quat.y, quat.z, quat.w );
	}

	inline glm::vec3 PxToGLM( const physx::PxVec3& vec )
	{
		return *( glm::vec3* ) &vec;
	}

	// We don't use quat's atm.
	inline glm::quat QPxToGLM( const physx::PxQuat& quat )
	{
		return *( glm::quat* ) &quat;
	}

	inline physx::PxTransform GLMTransformToPx( const glm::mat4& mat )
	{
		physx::PxQuat r = QGLMToPx( glm::normalize( glm::quat( mat ) ) );
		physx::PxVec3 p = GLMToPx( glm::vec3( mat[ 3 ] ) );

		return physx::PxTransform( p, r );
	}
}