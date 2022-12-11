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

#include "sppch.h"
#include "StorageBufferSet.h"

namespace Saturn {

	StorageBufferSet::StorageBufferSet( uint32_t size, uint32_t binding )
	{

	}

	StorageBufferSet::~StorageBufferSet()
	{

	}

	void StorageBufferSet::Create( uint32_t set, uint32_t binding )
	{
		ShaderStorageBuffer ssb;
		ssb.Size = 1;
		ssb.Size = set;
		ssb.Binding = binding;
		ssb.Buffer = nullptr;

		Set( ssb, set, binding );
	}

	void StorageBufferSet::Set( const ShaderStorageBuffer& rBuffer, uint32_t set, uint32_t binding )
	{
		m_Buffers[ set ][ binding ] = rBuffer;
	}

	void StorageBufferSet::Resize( uint32_t set, uint32_t binding, size_t newSize )
	{
		m_Buffers.at( set ).at( binding ).ResizeStorageBuffer( newSize );
	}

	ShaderStorageBuffer& StorageBufferSet::Get( uint32_t set, uint32_t binding )
	{
		return m_Buffers.at( set ).at( binding );
	}

}