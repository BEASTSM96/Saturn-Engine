/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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

#include "Saturn/Core/Ref.h"
#include "Shader.h"

#include "StorageBuffer.h"

#include <unordered_map>

namespace Saturn {

	class StorageBufferSet : public CountedObj
	{
	public:
		StorageBufferSet( uint32_t size, uint32_t binding );
		~StorageBufferSet();

		void Create( uint32_t set, uint32_t binding );

		void Resize( uint32_t set, uint32_t binding, uint32_t frame, size_t newSize );
		void Resize( uint32_t set, uint32_t binding, size_t newSize );

		Ref<StorageBuffer> Get( uint32_t set, uint32_t binding, uint32_t frame );
		
	private:
		void Set( Ref<StorageBuffer>& rBuffer, uint32_t set, uint32_t binding );

	private:
		// Set, Binding, Frame, Buffer
		std::unordered_map< uint32_t, std::unordered_map<uint32_t, std::unordered_map< uint32_t, Ref<StorageBuffer> >>> m_Buffers;
	};
}