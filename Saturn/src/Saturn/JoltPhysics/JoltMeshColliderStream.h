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

#include "JoltMeshCollider.h"

#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

namespace Saturn {

	class JoltMeshColliderWriter : public JPH::StreamOut
	{
	public:
		void WriteBytes( const void* inData, size_t inNumBytes ) override;

		bool IsFailed() const override;

		Buffer ToBuffer() const
		{
			return Buffer::Copy( m_Data.data(), m_Data.size() );
		}

	private:
		std::vector<uint8_t*> m_Data;
	};

	class JoltMeshColliderReader : public JPH::StreamIn
	{
	public:
		JoltMeshColliderReader( const Buffer& rBuffer )
			: m_Buffer( &rBuffer ) 
		{
		}

		~JoltMeshColliderReader()
		{
			m_Buffer = nullptr;
		}

		void ReadBytes( void* outData, size_t inNumBytes ) override;

		bool IsEOF() const override;

		bool IsFailed() const override;

	private:
		const Buffer* m_Buffer = nullptr;

		size_t m_BytesCompleted = 0;
	};
}