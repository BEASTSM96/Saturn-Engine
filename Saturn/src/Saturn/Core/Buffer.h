/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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

#include "Saturn/Core/Base.h"

namespace Saturn {

	struct Buffer
	{
		
		uint8_t* Data;
		uint32_t Size;

		Buffer()
			: Data( nullptr ), Size( 0 )
		{
		}

		Buffer( uint8_t* data, uint32_t size )
			: Data( data ), Size( size )
		{
		}

		static Buffer Copy( void* data, uint32_t size )
		{
			Buffer buffer;
			buffer.Allocate( size );
			memcpy( buffer.Data, data, size );
			return buffer;
		}

		void Allocate( uint32_t size )
		{
			delete[] Data;
			Data = nullptr;

			if( size == 0 )
			{
				SAT_CORE_ASSERT( !size == 0, "The size that was given was 0!" );
				return;
			}

			Data = new uint8_t[ size ];
			Size = size;
		}

		void ZeroInitialize( void )
		{
			if( Data )
				memset( Data, 0, Size );
		}

		template<typename T>
		T& Read( uint32_t offset = 0 )
		{
			return *( T* )( Data + offset );
		}

		void Write( void* data, uint32_t size, uint32_t offset = 0 )
		{
			SAT_CORE_ASSERT( offset + size <= Size, "Buffer overflow!" );
			memcpy( Data + offset, data, size );
		}

		operator bool() const
		{
			return Data;
		}

		uint8_t& operator[]( int index )
		{
			return Data[ index ];
		}

		uint8_t operator[]( int index ) const
		{
			return Data[ index ];
		}

		template<typename T>
		T* As()
		{
			return ( T* )Data;
		}

		inline uint32_t GetSize() const { return Size; }

	};
}