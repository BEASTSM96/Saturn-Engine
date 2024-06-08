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

#include "Saturn/Core/Ref.h"
#include "Saturn/Serialisation/RawSerialisation.h"

#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Saturn {

	class Link : public RefTarget
	{
	public:
		Link() = default;

		Link( ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId ) :
			ID( id ), StartPinID( startPinId ), EndPinID( endPinId ), Color( 255, 255, 255 )
		{
		}

		static void Serialise( const Ref<Link>& rObject, std::ofstream& rStream )
		{
			RawSerialisation::WriteObject( rObject->ID, rStream );
			RawSerialisation::WriteObject( rObject->StartPinID, rStream );
			RawSerialisation::WriteObject( rObject->EndPinID, rStream );

			RawSerialisation::WriteObject( rObject->Color, rStream );
		}

		static void Deserialise( Ref<Link>& rObject, std::ifstream& rStream )
		{
			RawSerialisation::ReadObject( rObject->ID, rStream );
			RawSerialisation::ReadObject( rObject->StartPinID, rStream );
			RawSerialisation::ReadObject( rObject->EndPinID, rStream );

			RawSerialisation::ReadObject( rObject->Color, rStream );
		}

	public:
		ed::LinkId ID;

		ed::PinId StartPinID;
		ed::PinId EndPinID;

		ImColor Color;
	};

}