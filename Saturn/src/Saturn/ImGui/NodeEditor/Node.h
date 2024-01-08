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
		Material_Sampler2D,
		AssetHandle
	};

	inline std::string_view PinTypeToString( PinType type )
	{
		switch( type )
		{
			case Saturn::PinType::Flow:
				return "Flow";
			case Saturn::PinType::Bool:
				return "Bool";
			case Saturn::PinType::Int:
				return "Int";
			case Saturn::PinType::Float:
				return "Float";
			case Saturn::PinType::String:
				return "String";
			case Saturn::PinType::Object:
				return "Object";
			case Saturn::PinType::Function:
				return "Function";
			case Saturn::PinType::Delegate:
				return "Delegate";
			case Saturn::PinType::Material_Sampler2D:
				return "Material_Sampler2D";
			case Saturn::PinType::AssetHandle:
				return "AssetHandle";
			default:
				break;
		}

		return "";
	}

	inline PinType StringToPinType( const std::string& rString )
	{
		if( rString == "Flow" )
			return PinType::Flow;
		else if( rString == "Bool" )
			return PinType::Bool;
		else if( rString == "Int" )
			return PinType::Int;
		else if( rString == "Float" )
			return PinType::Float;
		else if( rString == "String" )
			return PinType::String;
		else if( rString == "Object" )
			return PinType::Object;
		else if( rString == "Function" )
			return PinType::Function;
		else if( rString == "Material_Sampler2D" )
			return PinType::Material_Sampler2D;
		else if( rString == "AssetHandle" )
			return PinType::AssetHandle;
		else
			return PinType::Object;
	}

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

	class Node;

	class Pin : public RefTarget
	{
	public:
		Pin( int id, const std::string& name, PinType type, ed::NodeId nodeID ) :
			ID( id ), Node( nullptr ), Name( name ), Type( type ), Kind( PinKind::Input ), NodeID( nodeID )
		{
			ExtraData = Buffer();
		}
	public:
		ed::PinId	ID;
		ed::NodeId  NodeID;
		Ref<Node>	Node;
		std::string Name;
		PinType     Type;
		PinKind     Kind;
		Buffer      ExtraData;
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
		Node( int id, const std::string& name, ImColor color = ImColor( 255, 255, 255 ) ) :
			ID( id ), Name( name ), Color( color ), Type( NodeType::Blueprint ), Size( 0, 0 )
		{
			ExtraData = Buffer();
		}
	public:
		ed::NodeId ID;
		std::string Name;
		std::vector<Ref<Pin>> Inputs;
		std::vector<Ref<Pin>> Outputs;
		ImColor Color;
		NodeType Type;
		ImVec2 Size;

		// Any other extra data that should be stored in the node.
		Buffer ExtraData;

		std::string State;
		std::string SavedState;
	};

	class Link : public RefTarget
	{
	public:
		Link( ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId ) :
			ID( id ), StartPinID( startPinId ), EndPinID( endPinId ), Color( 255, 255, 255 )
		{
		}
	public:
		ed::LinkId ID;

		ed::PinId StartPinID;
		ed::PinId EndPinID;

		ImColor Color;
	};
}