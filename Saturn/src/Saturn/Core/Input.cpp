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
#include "Input.h"

#include <vector>

namespace Saturn::Input {

	// TODO: Maybe think of something better?

	static std::pair<float, float> s_MouseOffsets;
	static std::pair<float, float> s_MousePos;

	static std::vector<std::function<void()>> s_ScrollEvents;

    std::pair<float, float> MouseOffset()
    {
		return s_MouseOffsets;
    }

	float MouseXOffset()
	{
		return s_MouseOffsets.first;
	}

	float MouseYOffset()
	{
		return s_MouseOffsets.second;
	}

	void SetOffset( float offx, float offy )
	{
		s_MouseOffsets.first = offx;
		s_MouseOffsets.second = offy;

		SAT_CORE_INFO( "Mouse Offset {0}, {1}", offx, offy );
	}

	void SetMousePos( int x, int y )
	{
		s_MousePos.first = x;
		s_MousePos.second = y;
	}

	std::pair<float, float> MousePos()
	{
		return s_MousePos;
	}

	int MouseY()
	{
		return s_MousePos.second;
	}

	int MouseX()
	{
		return s_MousePos.first;
	}

	void Subscribe( std::function<void()> func )
	{
		s_ScrollEvents.push_back( func );
	}

	void UpdateScrollEvents()
	{
		for ( auto& event : s_ScrollEvents )
		{
			event();
		}
	}

}