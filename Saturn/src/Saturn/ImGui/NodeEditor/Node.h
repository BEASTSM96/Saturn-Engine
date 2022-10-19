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

#pragma once

#include <string>
#include <vector>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Saturn {

	enum class PinType
	{
		Flow,
		Bool,
		Int,
		Float,
		String,
		Object,
		Function,
		Delegate,
	};

	enum class PinKind
	{
		Output,
		Input
	};

	enum class NodeType
	{
		Blueprint,
		Simple,
		Comment
	};

	struct Node;

	struct Pin
	{
		ed::PinId	ID;
		Node*		Node;
		std::string Name;
		PinType     Type;
		PinKind     Kind;

		Pin( int id, const char* name, PinType type ) :
			ID( id ), Node( nullptr ), Name( name ), Type( type ), Kind( PinKind::Input )
		{
		}
	};

	struct Node
	{
		ed::NodeId ID;
		std::string Name;
		std::vector<Pin> Inputs;
		std::vector<Pin> Outputs;
		ImColor Color;
		NodeType Type;
		ImVec2 Size;

		std::string State;
		std::string SavedState;

		Node( int id, const char* name, ImColor color = ImColor( 255, 255, 255 ) ) :
			ID( id ), Name( name ), Color( color ), Type( NodeType::Blueprint ), Size( 0, 0 )
		{
		}
	};

	struct Link
	{
		ed::LinkId ID;

		ed::PinId StartPinID;
		ed::PinId EndPinID;

		ImColor Color;

		Link( ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId ) :
			ID( id ), StartPinID( startPinId ), EndPinID( endPinId ), Color( 255, 255, 255 )
		{
		}
	};
}