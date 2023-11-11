/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2023 BEAST                                                           		*
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

#include <stdint.h>
#include <string>

#define RBY_BIT( x ) ( 1 << x )

#if defined( RBY_DLL )
#if defined( RBY_BUILD )
#define RBY_API __declspec(dllexport)
#else
#define RBY_API __declspec(dllimport)
#endif
#else
#define RBY_API 
#endif

enum class RubyGraphicsAPI
{
	OpenGL,
	Vulkan,
	DirectX11,
	DirectX12,
	None
};

enum class RubyStyle
{
	Default = RBY_BIT( 0 ),
	Borderless = RBY_BIT( 1 ),
};

enum class RubyCursor 
{
	Arrow,
	Hand,
	IBeam
};

struct RubyWindowSpecification
{
	std::string_view Name;
	uint32_t Width = 0;
	uint32_t Height = 0;
	RubyGraphicsAPI GraphicsAPI = RubyGraphicsAPI::None;
	RubyStyle Style = RubyStyle::Default;
	bool ShowNow = true;
};

struct RubyIVec2
{
	int x = 0;
	int y = 0;
};

struct RubyVec2
{
	float x = 0.0f;
	float y = 0.0f;
};