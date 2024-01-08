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
#include "DefaultNodes.h"

namespace Saturn {

	extern Ref<Node> DefaultNodes::SpawnNewGetAssetNode( Ref<NodeEditor>& rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Asset ID";
		pin.Type = PinType::AssetHandle;

		NodeSpecification node;
		node.Color = ImColor( 30, 117, 217 );
		node.Name = "Get Asset";

		node.Outputs.push_back( pin );

		return rNodeEditor->AddNode( node );
	}

	extern Ref<Node> DefaultNodes::SpawnNewSampler2D( Ref<NodeEditor>& rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Albedo";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification node;
		node.Color = ImColor( 0, 255, 0 );
		node.Name = "Sampler2D";

		pin.Name = "RGBA";
		node.Outputs.push_back( pin );
		pin.Name = "R";
		node.Outputs.push_back( pin );
		pin.Name = "G";
		node.Outputs.push_back( pin );
		pin.Name = "B";
		node.Outputs.push_back( pin );
		pin.Name = "A";
		node.Outputs.push_back( pin );

		pin.Name = "Asset";
		pin.Type = PinType::AssetHandle;
		node.Inputs.push_back( pin );

		return rNodeEditor->AddNode( node );
	}

	extern Ref<Node> DefaultNodes::SpawnNewColorPickerNode( Ref<NodeEditor>& rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "RGBA";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification node;
		node.Color = ImColor( 252, 186, 3 );
		node.Name = "Color Picker";

		node.Outputs.push_back( pin );

		return rNodeEditor->AddNode( node );
	}
}