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

#include "sppch.h"
#include "JoltMeshColliderStream.h"

#include "Saturn/Project/Project.h"

namespace Saturn {

	void JoltMeshColliderWriter::WriteBytes( const void* inData, size_t inNumBytes )
	{
		size_t offset = m_Stream.size() + inNumBytes;

		m_Stream.resize( offset );

		memcpy( m_Stream.data(), inData, inNumBytes );
	}

	bool JoltMeshColliderWriter::IsFailed() const
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	void JoltMeshColliderReader::ReadBytes( void* outData, size_t inNumBytes )
	{
		// We want to read from where we are and not the start
		memcpy( outData, (uint8_t*)m_Buffer->Data + m_BytesCompleted, inNumBytes );
		m_BytesCompleted += inNumBytes;
	}

	bool JoltMeshColliderReader::IsEOF() const
	{
		return m_BytesCompleted >= m_Buffer->Size;
	}

	bool JoltMeshColliderReader::IsFailed() const
	{
		return m_BytesCompleted == 0 || m_Buffer->Size == 0;
	}

}