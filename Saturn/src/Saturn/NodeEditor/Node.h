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

#include "Saturn/Core/Memory/Buffer.h"
#include "Pin.h"

#include <string>
#include <vector>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Saturn {

	enum class NodeType
	{
		Blueprint,
		Simple,
		Comment
	};

	struct PinSpecification
	{
		std::string Name;
		PinType     Type = PinType::Object;
	};

	struct NodeSpecification
	{
		std::string                   Name;
		std::vector<PinSpecification> Outputs;
		std::vector<PinSpecification> Inputs;
		ImColor						  Color;
	};

	class Node : public RefTarget
	{
	public:
		Node() = default;

		Node( int id, 
			const std::string& rName, 
			ImColor color = ImColor( 255, 255, 255 ) ) :
			ID( id ), Name( rName ), Color( color ), Type( NodeType::Blueprint ), Size( 0, 0 )
		{
			ExtraData = Buffer();
		}

		~Node() 
		{
			ExtraData.Free();
		}

		static void Serialise( const Ref<Node>& rObject, std::ofstream& rStream );
		static void Deserialise( Ref<Node>& rObject, std::ifstream& rStream );

	public:
		ed::NodeId ID;
		std::string Name;
		std::vector<Ref<Pin>> Inputs;
		std::vector<Ref<Pin>> Outputs;
		ImColor Color;
		NodeType Type;
		ImVec2 Size;
		ImVec2 Position;
		bool CanBeDeleted = true;

		// Any other extra data that should be stored in the node.
		Buffer ExtraData;

		std::string ActiveState;
		std::string SavedState;
	};

}