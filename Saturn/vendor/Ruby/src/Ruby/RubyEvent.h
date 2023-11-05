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

#include "RubyEventType.h"

class RubyEvent
{
public:
	RubyEvent() = default;
	RubyEvent( RubyEventType newType ) : Type( newType ) {}

	virtual ~RubyEvent() = default;

public:
	bool Handled = false;
	RubyEventType Type;
};

class RubyWindowResizeEvent : public RubyEvent
{
public:
	RubyWindowResizeEvent() = default;
	RubyWindowResizeEvent( RubyEventType type, uint32_t width, uint32_t height ) 
		: RubyEvent( type ), m_Width( width ), m_Height( height )
	{
	}

	~RubyWindowResizeEvent() {}

	uint32_t GetWidth() { return m_Width; }
	uint32_t GetHeight() { return m_Height; }

	const uint32_t GetWidth() const { return m_Width; }
	const uint32_t GetHeight() const { return m_Height; }

private:
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
};

class RubyMouseMoveEvent : public RubyEvent
{
public:
	RubyMouseMoveEvent() = default;
	RubyMouseMoveEvent( RubyEventType type, float x, float y )
		: RubyEvent( type ), X( x ), Y( y )
	{
	}

	~RubyMouseMoveEvent() {}

	float GetX() const { return X; }
	float GetY() const { return Y; }

private:
	float X, Y;
};

class RubyMouseEvent : public RubyEvent
{
public:
	RubyMouseEvent() = default;
	RubyMouseEvent( RubyEventType type, int ButtonCode ) : RubyEvent( type ), m_ButtonCode( ButtonCode ) {}
	~RubyMouseEvent() {}

	int GetButton() const { return m_ButtonCode; }

private:
	int m_ButtonCode = 0;
};

class RubyKeyEvent : public RubyEvent
{
public:
	RubyKeyEvent() = default;
	RubyKeyEvent( RubyEventType Type, int scanCode ) : RubyEvent( Type ), m_ScanCode( scanCode ) {}

	~RubyKeyEvent() = default;

	int GetScancode() const { return m_ScanCode; }

private:
	int m_ScanCode = 0;
};

class RubyMaximizeEvent : public RubyEvent
{
public:
	RubyMaximizeEvent() = default;
	RubyMaximizeEvent( RubyEventType Type, bool state ) : RubyEvent( Type ), m_State( state ) {}

	~RubyMaximizeEvent() = default;

	bool GetState() const { return m_State; }

private:
	// State meaning: true is maximized, false no longer maximized.
	bool m_State = false;
};

class RubyMinimizeEvent : public RubyEvent
{
public:
	RubyMinimizeEvent() = default;
	RubyMinimizeEvent( RubyEventType Type, bool state ) : RubyEvent( Type ), m_State( state ) {}

	~RubyMinimizeEvent() = default;

	bool GetState() const { return m_State; }

private:
	bool m_State = false;
};

class RubyEventTarget
{
public:
	virtual ~RubyEventTarget() = default;

	virtual bool OnEvent( const RubyEvent& rEvent ) = 0;

public:
};
