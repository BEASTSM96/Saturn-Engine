/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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

namespace Saturn {

	class SClass;

	enum class SPropertyFlags
	{
		None,
		EditInEditor,
		ReadOnlyInEditor
	};

	enum class SPropertyType
	{
		Char,
		Float,
		Int,
		Double,
		Uint8,
		Uint16,
		Uint32,
		Uint64,
		Int8,
		Int16,
		Int32, /* same as int*/
		Int64,
		Object,
		AssetHandle,
		Class,
		Unknown
	};

	struct SProperty
	{
		std::string Name;
		SPropertyType Type;
		SPropertyFlags Flags;

		const void* pSetPropertyFunction;
		const void* pGetPropertyFunction;
	};

#define SAT_CREATE_FNP_FOR_TYPE( typename, x ) \
	typedef void( __stdcall* SetPropertyFn_##typename )( SClass*, x ); \
	typedef x( __stdcall* GetPropertyFn_##typename )( SClass* ); \

#define SAT_CALL_SET_FUNCTION_FOR_TYPE( typename, x, pClass, value ) \
	SetPropertyFn_##typename fnp = ( SetPropertyFn_##typename )x.pSetPropertyFunction; \
	( fnp ) ( pClass, value ) \

#define SAT_CALL_GET_FUNCTION_FOR_TYPE( typename, x, pClass ) \
	GetPropertyFn_##typename fnp = ( GetPropertyFn_##typename )x.pGetPropertyFunction; \
	return ( fnp ) ( pClass ) \


	//////////////////////////////////////////////////////////////////////////
	// Function types
	SAT_CREATE_FNP_FOR_TYPE( Char,   char );
	SAT_CREATE_FNP_FOR_TYPE( Float,  float );
	SAT_CREATE_FNP_FOR_TYPE( Int,    int );
	SAT_CREATE_FNP_FOR_TYPE( Double, double );
	SAT_CREATE_FNP_FOR_TYPE( Uint8,  unsigned char );
	SAT_CREATE_FNP_FOR_TYPE( Uint16, unsigned short );
	SAT_CREATE_FNP_FOR_TYPE( Uint32, unsigned int );
	SAT_CREATE_FNP_FOR_TYPE( Uint64, unsigned long long );
	SAT_CREATE_FNP_FOR_TYPE( Int8,   signed char );
	SAT_CREATE_FNP_FOR_TYPE( Int16,  short );
	SAT_CREATE_FNP_FOR_TYPE( Int32,  int );
	SAT_CREATE_FNP_FOR_TYPE( Int64,  long long );

	template<typename Ty>
	static void SetSProperty( const SProperty& rProperty, SClass* pClass, Ty value )
	{
		switch( rProperty.Type )
		{
			case SPropertyType::Char:   { SAT_CALL_SET_FUNCTION_FOR_TYPE( Char, rProperty, pClass, value );   } break;
			case SPropertyType::Float:  { SAT_CALL_SET_FUNCTION_FOR_TYPE( Float, rProperty, pClass, value );  } break;
			case SPropertyType::Int:    { SAT_CALL_SET_FUNCTION_FOR_TYPE( Int, rProperty, pClass, value );    } break;
			case SPropertyType::Double: { SAT_CALL_SET_FUNCTION_FOR_TYPE( Double, rProperty, pClass, value ); } break;
			case SPropertyType::Uint8:  { SAT_CALL_SET_FUNCTION_FOR_TYPE( Uint8, rProperty, pClass, value );  } break;
			case SPropertyType::Uint16: { SAT_CALL_SET_FUNCTION_FOR_TYPE( Uint16, rProperty, pClass, value ); } break;
			case SPropertyType::Uint32: { SAT_CALL_SET_FUNCTION_FOR_TYPE( Uint32, rProperty, pClass, value ); } break;
			case SPropertyType::Uint64: { SAT_CALL_SET_FUNCTION_FOR_TYPE( Uint64, rProperty, pClass, value ); } break;
			case SPropertyType::Int8:   { SAT_CALL_SET_FUNCTION_FOR_TYPE( Int8, rProperty, pClass, value );   } break;
			case SPropertyType::Int16:  { SAT_CALL_SET_FUNCTION_FOR_TYPE( Int16, rProperty, pClass, value );  } break;
			case SPropertyType::Int32:  { SAT_CALL_SET_FUNCTION_FOR_TYPE( Int32, rProperty, pClass, value );  } break;
			case SPropertyType::Int64:  { SAT_CALL_SET_FUNCTION_FOR_TYPE( Int16, rProperty, pClass, value );  } break;
		}
	}

	template<typename Ty>
	static Ty GetSProperty( const SProperty& rProperty, SClass* pClass )
	{
		switch( rProperty.Type )
		{
			case SPropertyType::Char:   { SAT_CALL_GET_FUNCTION_FOR_TYPE( Char, rProperty, pClass );   } break;
			case SPropertyType::Float:  { SAT_CALL_GET_FUNCTION_FOR_TYPE( Float, rProperty, pClass );  } break;
			case SPropertyType::Int:    { SAT_CALL_GET_FUNCTION_FOR_TYPE( Int, rProperty, pClass );    } break;
			case SPropertyType::Double: { SAT_CALL_GET_FUNCTION_FOR_TYPE( Double, rProperty, pClass ); } break;
			case SPropertyType::Uint8:  { SAT_CALL_GET_FUNCTION_FOR_TYPE( Uint8, rProperty, pClass );  } break;
			case SPropertyType::Uint16: { SAT_CALL_GET_FUNCTION_FOR_TYPE( Uint16, rProperty, pClass ); } break;
			case SPropertyType::Uint32: { SAT_CALL_GET_FUNCTION_FOR_TYPE( Uint32, rProperty, pClass ); } break;
			case SPropertyType::Uint64: { SAT_CALL_GET_FUNCTION_FOR_TYPE( Uint64, rProperty, pClass ); } break;
			case SPropertyType::Int8:   { SAT_CALL_GET_FUNCTION_FOR_TYPE( Int8, rProperty, pClass );   } break;
			case SPropertyType::Int16:  { SAT_CALL_GET_FUNCTION_FOR_TYPE( Int16, rProperty, pClass );  } break;
			case SPropertyType::Int32:  { SAT_CALL_GET_FUNCTION_FOR_TYPE( Int32, rProperty, pClass );  } break;
			case SPropertyType::Int64:  { SAT_CALL_GET_FUNCTION_FOR_TYPE( Int16, rProperty, pClass );  } break;
		}
	}
}