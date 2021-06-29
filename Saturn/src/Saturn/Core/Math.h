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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	static bool DecomposeTransform( const glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale )
	{
		using namespace glm;
		using T = float;

		mat4 LocalMatrix( transform );

		// Normalize the matrix.
		if( epsilonEqual( LocalMatrix[ 3 ][ 3 ], static_cast< float >( 0 ), epsilon<T>() ) )
			return false;

		// First, isolate perspective.  This is the messiest.
		if(
			epsilonNotEqual( LocalMatrix[ 0 ][ 3 ], static_cast< T >( 0 ), epsilon<T>() ) ||
			epsilonNotEqual( LocalMatrix[ 1 ][ 3 ], static_cast< T >( 0 ), epsilon<T>() ) ||
			epsilonNotEqual( LocalMatrix[ 2 ][ 3 ], static_cast< T >( 0 ), epsilon<T>() ) )
		{
			// Clear the perspective partition
			LocalMatrix[ 0 ][ 3 ] = LocalMatrix[ 1 ][ 3 ] = LocalMatrix[ 2 ][ 3 ] = static_cast< T >( 0 );
			LocalMatrix[ 3 ][ 3 ] = static_cast< T >( 1 );
		}

		// Next take care of translation (easy).
		translation = vec3( LocalMatrix[ 3 ] );
		LocalMatrix[ 3 ] = vec4( 0, 0, 0, LocalMatrix[ 3 ].w );

		vec3 Row[ 3 ];

		// Now get scale and shear.
		for( length_t i = 0; i < 3; ++i )
			for( length_t j = 0; j < 3; ++j )
				Row[ i ][ j ] = LocalMatrix[ i ][ j ];

		// Compute X scale factor and normalize first row.
		scale.x = length( Row[ 0 ] );
		Row[ 0 ] = detail::scale( Row[ 0 ], static_cast< T >( 1 ) );
		scale.y = length( Row[ 1 ] );
		Row[ 1 ] = detail::scale( Row[ 1 ], static_cast< T >( 1 ) );
		scale.z = length( Row[ 2 ] );
		Row[ 2 ] = detail::scale( Row[ 2 ], static_cast< T >( 1 ) );

		rotation.y = asin( -Row[ 0 ][ 2 ] );
		if( cos( rotation.y ) != 0 )
		{
			rotation.x = atan2( Row[ 1 ][ 2 ], Row[ 2 ][ 2 ] );
			rotation.z = atan2( Row[ 0 ][ 1 ], Row[ 0 ][ 0 ] );
		}
		else
		{
			rotation.x = atan2( -Row[ 2 ][ 0 ], Row[ 1 ][ 1 ] );
			rotation.z = 0;
		}
		return true;
	}

}