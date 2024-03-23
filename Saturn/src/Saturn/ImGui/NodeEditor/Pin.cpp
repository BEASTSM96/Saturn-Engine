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
#include "Pin.h"

#include "Node.h"

namespace Saturn {

	void Pin::Serialise( const Ref<Pin>& rObject, std::ofstream& rStream )
	{
		RawSerialisation::WriteObject( rObject->ID, rStream );
		RawSerialisation::WriteObject( rObject->NodeID, rStream );

		RawSerialisation::WriteString( rObject->Name, rStream );
		RawSerialisation::WriteObject( rObject->Type, rStream );
		RawSerialisation::WriteObject( rObject->Kind, rStream );

		RawSerialisation::WriteSaturnBuffer( rObject->ExtraData, rStream );
	}

	void Pin::Deserialise( Ref<Pin>& rObject, std::ifstream& rStream )
	{
		RawSerialisation::ReadObject( rObject->ID, rStream );
		RawSerialisation::ReadObject( rObject->NodeID, rStream );

		rObject->Name = RawSerialisation::ReadString( rStream );
		RawSerialisation::ReadObject( rObject->Type, rStream );
		RawSerialisation::ReadObject( rObject->Kind, rStream );

		RawSerialisation::ReadSaturnBuffer( rObject->ExtraData, rStream );
	}

}