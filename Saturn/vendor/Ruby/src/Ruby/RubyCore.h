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

enum class RubyCursorType 
{
	Arrow,
	Hand,
	IBeam,
	ResizeEW,
	ResizeNS,
	ResizeNESW,
	ResizeNWSE,
	NotAllowed,
	ImageFile
};

enum class RubyCursorMode
{
	Normal,
	Hidden,
	Locked
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
	RubyIVec2() = default;
	RubyIVec2( int _x, int _y ) : x( _x ), y( _y ) {}

	int x = 0;
	int y = 0;

	RubyIVec2 operator+( const RubyIVec2& other ) const { return RubyIVec2( x + other.x, y + other.y ); }
	RubyIVec2 operator-( const RubyIVec2& other ) const { return RubyIVec2( x - other.x, y - other.y ); }
	RubyIVec2 operator/( const RubyIVec2& other ) const { return RubyIVec2( x / other.x, y / other.y ); }
	RubyIVec2 operator*( const RubyIVec2& other ) const { return RubyIVec2( x * other.x, y * other.y ); }

	RubyIVec2& operator+=( const RubyIVec2& other ) { x += other.x; y += other.y; return *this; }
	RubyIVec2& operator-=( const RubyIVec2& other ) { x -= other.x; y -= other.y; return *this; }
	RubyIVec2& operator*=( int scalar )          { x *= scalar; y *= scalar; return *this;   }
	RubyIVec2& operator/=( int divisor )         { x /= divisor; y /= divisor; return *this; }
};

struct RubyVec2
{
	RubyVec2() = default;
	RubyVec2( float _x, float _y ) : x( _x ), y( _y ) {}

	float x = 0.0f;
	float y = 0.0f;

	RubyVec2 operator+( const RubyVec2& other ) const { return RubyVec2( x + other.x, y + other.y ); }
	RubyVec2 operator-( const RubyVec2& other ) const { return RubyVec2( x - other.x, y - other.y ); }
	RubyVec2 operator/( const RubyVec2& other ) const { return RubyVec2( x / other.x, y / other.y ); }
	RubyVec2 operator*( const RubyVec2& other ) const { return RubyVec2( x * other.x, y * other.y ); }

	RubyVec2& operator+=( const RubyVec2& other ) { x += other.x; y += other.y; return *this; }
	RubyVec2& operator-=( const RubyVec2& other ) { x -= other.x; y -= other.y; return *this; }
	RubyVec2& operator*=( float scalar ) { x *= scalar; y *= scalar; return *this; }
	RubyVec2& operator/=( float divisor ) { x /= divisor; y /= divisor; return *this; }
};