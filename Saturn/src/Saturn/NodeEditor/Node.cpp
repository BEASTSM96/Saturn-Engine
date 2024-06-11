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
#include "Node.h"

#include "UI/NodeEditor.h"

#include "Saturn/Serialisation/RawSerialisation.h"

#include "builders.h"

namespace Saturn {

	static void SerialiseImColor( const ImColor& rColor, std::ofstream& rStream )
	{
		RawSerialisation::WriteObject( rColor.Value, rStream );
	}

	static void DeserialiseImColor( ImColor& rColor, std::ifstream& rStream )
	{
		RawSerialisation::ReadObject( rColor.Value, rStream );
	}

	static void SerialiseImVec2( const ImVec2& rVector, std::ofstream& rStream )
	{
		RawSerialisation::WriteObject( rVector.x, rStream );
		RawSerialisation::WriteObject( rVector.y, rStream );
	}

	static void DeserialiseImVec2( ImVec2& rVector, std::ifstream& rStream )
	{
		RawSerialisation::ReadObject( rVector.x, rStream );
		RawSerialisation::ReadObject( rVector.y, rStream );
	}

	void Node::Serialise( const Ref<Node>& rObject, std::ofstream& rStream )
	{
		UUID::Serialise( rObject->ID, rStream );
		RawSerialisation::WriteString( rObject->Name, rStream );
		RawSerialisation::WriteObject( rObject->Color, rStream );

		RawSerialisation::WriteObject( rObject->Type, rStream );
		RawSerialisation::WriteObject( rObject->Size, rStream );

		RawSerialisation::WriteObject( rObject->Position, rStream );

		RawSerialisation::WriteString( rObject->ActiveState, rStream );
		RawSerialisation::WriteString( rObject->SavedState, rStream );

		RawSerialisation::WriteSaturnBuffer( rObject->ExtraData, rStream );

		size_t mapSize = rObject->Inputs.size();
		RawSerialisation::WriteObject( mapSize, rStream );

		for( const auto& rInput : rObject->Inputs )
		{
			Pin::Serialise( rInput, rStream );
		}

		mapSize = rObject->Outputs.size();
		RawSerialisation::WriteObject( mapSize, rStream );

		for( const auto& rOutput : rObject->Outputs )
		{
			Pin::Serialise( rOutput, rStream );
		}
	}

	void Node::Deserialise( Ref<Node>& rObject, std::ifstream& rStream )
	{
		UUID::Deserialise( rObject->ID, rStream );
		rObject->Name = RawSerialisation::ReadString( rStream );
		RawSerialisation::ReadObject( rObject->Color, rStream );

		RawSerialisation::ReadObject( rObject->Type, rStream );
		RawSerialisation::ReadObject( rObject->Size, rStream );
		RawSerialisation::ReadObject( rObject->Position, rStream );

		rObject->ActiveState = RawSerialisation::ReadString( rStream );
		rObject->SavedState = RawSerialisation::ReadString( rStream );

		RawSerialisation::ReadSaturnBuffer( rObject->ExtraData, rStream );

		size_t mapSize = 0;
		RawSerialisation::ReadObject( mapSize, rStream );

		rObject->Inputs.resize( mapSize );

		for( size_t i = 0; i < mapSize; i++ )
		{
			Ref<Pin> pin = Ref<Pin>::Create();

			Pin::Deserialise( pin, rStream );

			rObject->Inputs[ i ] = pin;
		}

		mapSize = 0;
		RawSerialisation::ReadObject( mapSize, rStream );

		rObject->Outputs.resize( mapSize );

		for( size_t i = 0; i < mapSize; i++ )
		{
			Ref<Pin> pin = Ref<Pin>::Create();

			Pin::Deserialise( pin, rStream );

			rObject->Outputs[ i ] = pin;
		}

		ed::SetNodePosition( ed::NodeId( rObject->ID ), rObject->Position );
	}

	void Node::Render( ax::NodeEditor::Utilities::BlueprintNodeBuilder& rBuilder, NodeEditorBase* pBase )
	{
		rBuilder.Begin( ed::NodeId( ID ) );

		rBuilder.Header( Color );

		ImGui::Spring( 0 );
		ImGui::TextUnformatted( Name.c_str() );
		ImGui::Spring( 1 );
		ImGui::Dummy( ImVec2( 0, 28 ) );
		ImGui::Spring( 0 );

		rBuilder.EndHeader();

		uint32_t pinIndex = 0;
		for( auto& rInput : Inputs )
		{
			rInput->Render( rBuilder, pBase->IsLinked( rInput->ID ), pinIndex );
			pinIndex++;
		}

		for( auto& rOutput : Outputs )
		{
			if( rOutput->Type == PinType::Delegate )
				continue;

			rOutput->Render( rBuilder, pBase->IsLinked( rOutput->ID ), 0 );
		}

		rBuilder.End();
	}
}