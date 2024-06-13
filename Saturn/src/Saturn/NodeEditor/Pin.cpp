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

#include "Saturn/ImGui/ImGuiAuxiliary.h"

#include "NodeEditorAux.h"
#include "builders.h"

namespace Saturn {

	PinIconType Pin::GetIconType() const
	{
		switch( Type )
		{
			case PinType::Flow:				  return PinIconType::Flow;
			case PinType::Bool:				  return PinIconType::Circle;
			case PinType::Int:				  return PinIconType::Circle;
			case PinType::Float:			  return PinIconType::Circle;
			case PinType::String:			  return PinIconType::Circle;
			case PinType::Object:			  return PinIconType::Circle;
			case PinType::Function:			  return PinIconType::Circle;
			case PinType::Material_Sampler2D: return PinIconType::Circle;
			case PinType::AssetHandle:        return PinIconType::Circle;
			case PinType::Delegate:           return PinIconType::Square;
		}

		return PinIconType::Circle;
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
		auto rendererIcon = GetIconType();
		ImColor color = GetPinColor();
		color.Value.w = alpha / 255.0f;

		constexpr float PIN_ICON_SIZE = 24.0f;
		constexpr ImVec2 size = ImVec2( PIN_ICON_SIZE, PIN_ICON_SIZE );

		if( ImGui::IsRectVisible( size ) )
		{
			auto cursorPos = ImGui::GetCursorScreenPos();
			auto drawList = ImGui::GetWindowDrawList();

			Auxiliary::DrawPinIconInternal( drawList,
				cursorPos,
				cursorPos + size,
				rendererIcon,
				connected,
				color, ImColor( 32, 32, 32, alpha ) );
		}

		ImGui::Dummy( size );
	}

	void Pin::Render( ax::NodeEditor::Utilities::BlueprintNodeBuilder& rBuilder, bool linked, uint32_t pinIndex )
	{
		switch( Kind )
		{
			case PinKind::Output: 
			{
				RenderOutput( rBuilder, linked );
			} break;

			case PinKind::Input:
			{
				RenderInput( rBuilder, linked, pinIndex );
			} break;
		}
	}

	void Pin::RenderInput( ax::NodeEditor::Utilities::BlueprintNodeBuilder& rBuilder, bool linked, uint32_t pinIndex )
	{
		auto alpha = ImGui::GetStyle().Alpha;

		rBuilder.Input( ed::PinId( ID ) );

		ImGui::PushStyleVar( ImGuiStyleVar_Alpha, alpha );

		DrawIcon( linked, ( int ) ( alpha * 255 ) );

		ImGui::Spring( 0 );

		if( !Name.empty() )
		{
			ImGui::TextUnformatted( Name.c_str() );
			ImGui::Spring( 0 );
		}

		if( Type == PinType::Bool && !linked )
		{
			ImGui::Button( "Hello" );
			ImGui::Spring( 0 );
		}

		if( Type == PinType::Float && !linked )
		{
			float value = ExtraData.Read<float>( pinIndex * sizeof( float ) );

			ImGui::SetNextItemWidth( 25.0f );

			ImGui::PushID( static_cast<int>( ID ) );

			if( ImGui::DragFloat( "##floatinput", &value ) )
			{
				ExtraData.Write( &value, sizeof( float ), pinIndex * sizeof(float) );
			}

			ImGui::PopID();

			ImGui::Spring( 0 );
		}

		ImGui::PopStyleVar();

		rBuilder.EndInput();
	}

	void Pin::RenderOutput( ax::NodeEditor::Utilities::BlueprintNodeBuilder& rBuilder, bool linked )
	{
		bool OpenAssetColorPicker = false;
		bool OpenAssetIDPopup = false;

		auto alpha = ImGui::GetStyle().Alpha;

		ImGui::PushStyleVar( ImGuiStyleVar_Alpha, alpha );

		rBuilder.Output( ed::PinId( ID ) );

		if( !Name.empty() )
		{
			ImGui::Spring( 0 );
			ImGui::TextUnformatted( Name.c_str() );

			// TODO: Allow for certain asset types.
			if( Type == PinType::AssetHandle )
			{
				auto& rSavedUUID = Node->ExtraData.Read<UUID>( 0 );

				std::string name = "Select Asset";

				if( rSavedUUID != 0 )
					name = std::to_string( rSavedUUID );
				
				if( ImGui::Button( name.c_str() ) )
				{
					OpenAssetIDPopup = true;
				}
			}
			else if( Node->Name == "Color Picker" && Type == PinType::Material_Sampler2D )
			{
				ImGui::BeginHorizontal( "PickerH" );

				ImVec2 buttonSize = { ImGui::GetFrameHeight(), ImGui::GetFrameHeight() };
				ImRect boundingBox = { ImGui::GetCursorPos(), ImGui::GetCursorPos() + buttonSize };

				bool hovered, held;

				ImGui::ButtonBehavior( boundingBox, ImGui::GetID( &ID ), &hovered, &held );

				// TODO: Ruby and ImGui do not match! so ImGuiButtonFlags_None = 0 and RubyMouse::Left = 0
				if( hovered && ImGui::IsMouseClicked( ImGuiButtonFlags_None ) )
				{
					OpenAssetColorPicker = true;
				}

				Auxiliary::DrawColoredRect( buttonSize, Node->ExtraData.Read<ImVec4>( 0 ) );

				ImGui::EndHorizontal();
			}
		}

		ImGui::Spring( 0 );
		DrawIcon( linked, ( int ) ( alpha * 255 ) );

		rBuilder.EndOutput();
		ImGui::PopStyleVar();
		
		ed::Suspend();

		if( OpenAssetColorPicker )
		{
			ImGui::OpenPopup( "AssetColorPicker" );
		}

		ImGui::SetNextWindowSize( { 350.0f, 0.0f } );
		if( ImGui::BeginPopup( "AssetColorPicker", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize ) )
		{
			bool PopupModified = false;

			constexpr auto FALLBACK_COLOR = ImVec4( 114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f );

			ImVec4 color = FALLBACK_COLOR;

			color = Node->ExtraData.Read<ImVec4>( 0 );

			if( color.x == 0 && color.y == 0 && color.z == 0 && color.w == 0 )
				color = FALLBACK_COLOR;

			if( ImGui::ColorPicker3( "Color Picker", ( float* ) &color ) )
			{
				Node->ExtraData.Write( ( uint8_t* ) &color, sizeof( ImVec4 ), 0 );

				PopupModified = true;
			}
			else
			{
				if( PopupModified )
				{
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ed::Resume();
	}

	bool Pin::CanCreateLink( const Ref<Pin>& rOther )
	{
		if( !rOther || Kind == rOther->Kind || Type != rOther->Type || Node == rOther->Node )
			return false;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SERIAILSATION

	void Pin::Serialise( const Ref<Pin>& rObject, std::ofstream& rStream )
	{
		RawSerialisation::WriteObject( rObject->ID, rStream );

		RawSerialisation::WriteString( rObject->Name, rStream );
		RawSerialisation::WriteObject( rObject->Type, rStream );
		RawSerialisation::WriteObject( rObject->Kind, rStream );

		RawSerialisation::WriteSaturnBuffer( rObject->ExtraData, rStream );
	}

	void Pin::Deserialise( Ref<Pin>& rObject, std::ifstream& rStream )
	{
		RawSerialisation::ReadObject( rObject->ID, rStream );

		rObject->Name = RawSerialisation::ReadString( rStream );
		RawSerialisation::ReadObject( rObject->Type, rStream );
		RawSerialisation::ReadObject( rObject->Kind, rStream );

		RawSerialisation::ReadSaturnBuffer( rObject->ExtraData, rStream );
	}
}