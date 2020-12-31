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

#include "sppch.h"
#include "Collision.h"

namespace Saturn {

    bool Collision::TestAABB( glm::vec4 a, glm::vec4 b )
    {
       if ( a.x + b.w >= b.x && b.x + b.w >= a.x && a.y + a.b >= b.y && b.y + b.b >= a.y )
       {
		   SAT_CORE_INFO( "[AABB] a.x + b.w >= b.x && b.x + b.w >= a.x && a.y + a.b >= b.y && b.y + b.b >= a.y " );
           return true;
       }
       return false;
    }

	bool Collision::TestAABB( glm::vec3 a, glm::vec3 b )
	{
		if( a.z == b.z )
		{
			SAT_CORE_INFO( "[AABB] a.z == b.z " );
			return true;
		}
		if( a.x == b.x )
		{
			SAT_CORE_INFO( "[AABB] a.x == b.x " );
			return true;
		}
		if( a.z == b.z && a.x == b.x )
		{
			SAT_CORE_INFO( "[AABB] a.z == b.z & a.x == b.x " );
			return true;
		}
		return false;
	}

}
