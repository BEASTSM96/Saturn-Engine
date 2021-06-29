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

#ifdef USE_NVIDIA

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <physx/PxPhysicsAPI.h>

#define SAT_PHYSX_RELEASE(obj) obj->release(); obj = nullptr

namespace Saturn {

	static glm::vec3 PxVec3ToGLM( physx::PxVec3 vec ) 
	{
		glm::vec3 glmvec3;
		glmvec3.x = vec.x;
		glmvec3.y = vec.y;
		glmvec3.z = vec.z;
		return glmvec3;
	}

	static physx::PxVec3 glmVec3ToPx( glm::vec3 vec )
	{
		physx::PxVec3 pxvec3;
		pxvec3.x = vec.x;
		pxvec3.y = vec.y;
		pxvec3.z = vec.z;
		return pxvec3;
	}

	static physx::PxQuat GLMToPhysXQuat( glm::quat& quat )
	{
		return physx::PxQuat( quat.x, quat.y, quat.z, quat.w );
	}

	static physx::PxVec3 GLMToPhysXVec( glm::vec3& vec )
	{
		return *( physx::PxVec3* )&vec;
	}

	static physx::PxTransform glmTransformToPx( glm::mat4& mat )
	{
		physx::PxQuat r = GLMToPhysXQuat( glm::normalize( glm::quat( mat ) ) );
		physx::PxVec3 p = GLMToPhysXVec( glm::vec3( mat[ 3 ] ) );

		return physx::PxTransform( p, r );
	}

	static physx::PxTransform glmTransformToPx( glm::vec3& pos, glm::quat rot )
	{
		physx::PxQuat r = GLMToPhysXQuat( rot );
		physx::PxVec3 p = GLMToPhysXVec( pos );

		return physx::PxTransform( p, r );
	}

	physx::PxFilterFlags CollisionFilterShader( physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize );

}


#endif