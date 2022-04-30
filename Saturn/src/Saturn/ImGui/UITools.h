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

#include <string>
#include <glm/glm.hpp>

namespace Saturn {

	extern bool DrawVec3Control( const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f );
	extern bool DrawVec2Control( const std::string& label, glm::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f );

	extern bool DrawColorVec3Control( const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f );

	extern bool DrawFloatControl( const std::string& label, float& values, float columnWidth = 125.0f );
	extern bool DrawIntControl( const std::string& label, int& values, float columnWidth = 125.0f );
	
	extern bool DrawBoolControl( const std::string& label, bool& value, float columnWidth = 125.0f );
}