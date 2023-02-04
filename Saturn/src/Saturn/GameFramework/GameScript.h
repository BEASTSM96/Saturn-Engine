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

#include "Saturn/Scene/Entity.h"

#define SCLASS(...)

#define BODY_MACRO_COMBINE_INNER(A,B,C,D) A##B##C##D
#define BODY_MACRO_COMBINE(A,B,C,D) BODY_MACRO_COMBINE_INNER(A,B,C,D)

// Include a empty macro, this will be defined when SHT has been ran.
#define CURRENT_FILE_ID
#define GENERATED_BODY(...) BODY_MACRO_COMBINE(CURRENT_FILE_ID,_,__LINE__,_GENERATED_BODY);

#define DECLARE_CLASS( x, BaseClass ) \
private: \
	x& operator=(x&&); \
	x& operator=(const x&); \
	static Saturn::SClass* _PrvStatic() {} \
public: \
	typedef x ThisClass; \
	typedef BaseClass Super; \
	inline static Saturn::SClass* StaticClass() \
	{ \
		return nullptr; \
	} \
	extern "C" { \
	__declspec(dllexport) static x* Spawn() \
	{ \
		return new x(); \
	}\
	}

#define SATURN_REG_FUNC_NAME(x, a) x##a
#define SATURN_REG_FUNC(x, a) SATURN_REG_FUNC_NAME( x, a )

//-- Base script class
#define SATURN_REGISTER_SCRIPT(x) extern "C" { __declspec(dllexport) Saturn::SClass* SATURN_REG_FUNC(CreateScriptClass, x##_ )() { return new x(); }  }
//-- Entity
#define SATURN_REGISTER_ENTITY(x) extern "C" { __declspec(dllexport) Saturn::Entity* SATURN_REG_FUNC(CreateScriptClass, x##_ )() { return new x(); }  }