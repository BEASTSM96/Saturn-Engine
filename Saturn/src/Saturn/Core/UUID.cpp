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

#include "sppch.h"
#include "UUID.h"

#include "Random.h"
#include "Saturn/Serialisation/RawSerialisation.h"

namespace Saturn {

	UUID::UUID() : m_UUID( Random::RandomUUID() )
	{
	}

	UUID::UUID( uint64_t uuid ) : m_UUID( uuid )
	{
	}

	UUID::UUID( const UUID& other ) : m_UUID( other.m_UUID )
	{
	}

	void UUID::Serialise( const UUID& rObject, std::ofstream& rStream )
	{
		RawSerialisation::WriteObject( rObject.m_UUID, rStream );
	}

	void UUID::Deserialise( UUID& rObject, std::istream& rStream )
	{
		RawSerialisation::ReadObject( rObject.m_UUID, rStream );
	}
}