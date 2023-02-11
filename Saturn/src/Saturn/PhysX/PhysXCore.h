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

#include "PxPhysicsAPI.h"

#include <stdint.h>

namespace Saturn {

	enum ForceMode
	{
		Force,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum BroadphaseType
	{
		SaP, // Sweep and Prune
		AbP, // Auto Box Prune 
		MbP // Multi-Box Prune
	};

	enum FrictionType
	{
		Patch,
		OneDirectinal,
		TwoDirectinal
	};

	struct RaycastResult
	{
		uint64_t EntityID;
		float Distance;
		glm::vec3 Position;
		glm::vec3 Normal;
	};

	static physx::PxQuat GLMToPhysXQuat( const glm::quat& quat )
	{
		return physx::PxQuat( quat.x, quat.y, quat.z, quat.w );
	}

	static physx::PxVec3 GLMToPhysXVec( const glm::vec3& vec )
	{
		return *( physx::PxVec3* ) &vec;
	}

	static glm::quat PxQuatToGLM( physx::PxQuat quat )
	{
		return *( glm::quat* ) &quat;
	}

	static physx::PxTransform glmTransformToPx( const glm::mat4& mat )
	{
		physx::PxQuat r = GLMToPhysXQuat( glm::normalize( glm::quat( mat ) ) );
		physx::PxVec3 p = GLMToPhysXVec( glm::vec3( mat[ 3 ] ) );

		return physx::PxTransform( p, r );
	}

}