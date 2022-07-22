/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace Saturn {

	class Camera
	{
	public:
		Camera() = default;

		Camera( const glm::mat4& projectionMatrix )
			: m_Projection( projectionMatrix )
		{
			AllowEvents( true );
		}

		virtual ~Camera() = default;

		void AllowEvents( bool allow ) { m_CanRunEvents = allow; }
		bool HasEvents() { return m_CanRunEvents; }

		const glm::mat4& ProjectionMatrix() const { return m_Projection; }
		void SetProjectionMatrix( const glm::mat4& projection ) { m_Projection = projection; }
		
	protected:
		glm::mat4 m_Projection = glm::mat4( 1.0f );

		bool m_CanRunEvents = true;
	};
}