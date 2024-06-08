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

#include "Saturn/Serialisation/RawSerialisation.h"
#include "Node.h"

#include "Saturn/Vendor/Drawing.h"

namespace Saturn {

	ax::Drawing::IconType Pin::GetIconType() const
	{
		switch( Type )
		{
			case PinType::Flow:				  return ax::Drawing::IconType::Flow;
			case PinType::Bool:				  return ax::Drawing::IconType::Circle;
			case PinType::Int:				  return ax::Drawing::IconType::Circle;
			case PinType::Float:			  return ax::Drawing::IconType::Circle;
			case PinType::String:			  return ax::Drawing::IconType::Circle;
			case PinType::Object:			  return ax::Drawing::IconType::Circle;
			case PinType::Function:			  return ax::Drawing::IconType::Circle;
			case PinType::Material_Sampler2D: return ax::Drawing::IconType::Circle;
			case PinType::AssetHandle:        return ax::Drawing::IconType::Circle;
			case PinType::Delegate:           return ax::Drawing::IconType::Square;
		}

		return ax::Drawing::IconType::Circle;
	}

	ImColor Pin::GetPinColor() const
	{
		switch( Type )
		{
			default:
			case PinType::Flow:     return ImColor( 255, 255, 255 );
			case PinType::Bool:     return ImColor( 220, 48, 48 );
			case PinType::Int:      return ImColor( 68, 201, 156 );
			case PinType::Float:    return ImColor( 147, 226, 74 );
			case PinType::String:   return ImColor( 124, 21, 153 );
			case PinType::Object:   return ImColor( 51, 150, 215 );
			case PinType::Function: return ImColor( 218, 0, 183 );
			case PinType::Delegate: return ImColor( 255, 48, 48 );
			case PinType::AssetHandle: return ImColor( 0, 0, 255 );
		}

		return ImColor( 0, 0, 255 );
	}

	void Pin::DrawIcon( bool connected, int alpha )
	{
		auto nativeType = GetIconType();
		ImColor color = GetPinColor();
		color.Value.w = alpha / 255.0f;

		constexpr float PIN_ICON_SIZE = 24.0f;
		auto size = ImVec2( PIN_ICON_SIZE, PIN_ICON_SIZE );

		if( ImGui::IsRectVisible( size ) )
		{
			auto cursorPos = ImGui::GetCursorScreenPos();
			auto drawList = ImGui::GetWindowDrawList();

			ax::Drawing::DrawIcon( drawList,
				cursorPos,
				cursorPos + size,
				nativeType,
				connected,
				color, ImColor( 32, 32, 32, alpha ) );
		}

		ImGui::Dummy( size );
	}

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