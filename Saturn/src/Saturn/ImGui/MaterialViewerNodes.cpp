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
#include "Saturn/Vulkan/Mesh.h"
#include "MaterialViewerNodes.h"

#include "MaterialNodeEditorEvaluator.h"

#include "MaterialAssetViewer.h"
#include "Saturn/Asset/AssetManager.h"

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL NODE LIBRARY

	Ref<Node> MaterialNodeLibrary::SpawnGetAsset( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Asset ID";
		pin.Type = PinType::AssetHandle;

		NodeSpecification nodeSpec;
		nodeSpec.Color = ImColor( 30, 117, 217 );
		nodeSpec.Name = "Get Asset";

		nodeSpec.Outputs.push_back( pin );

		Ref<Node> node = Ref<Node>::Create( nodeSpec );
		node->ExecutionType = NodeExecutionType::AssetID;
		rNodeEditor->AddNode( node );

		return node;
	}

	Ref<MaterialColorPickerNode> MaterialNodeLibrary::SpawnColorPicker( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "RGBA";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification nodeSpec;
		nodeSpec.Color = ImColor( 252, 186, 3 );
		nodeSpec.Name = "Color Picker";

		nodeSpec.Outputs.push_back( pin );

		Ref<MaterialColorPickerNode> node = Ref<MaterialColorPickerNode>::Create( nodeSpec );
		rNodeEditor->AddNode( node );

		return node;
	}

	Ref<MaterialSampler2DNode> MaterialNodeLibrary::SpawnSampler2D( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Albedo";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification nodeSpec;
		nodeSpec.Color = ImColor( 0, 255, 0 );
		nodeSpec.Name = "Sampler2D";

		pin.Name = "RGBA";
		nodeSpec.Outputs.push_back( pin );
		pin.Name = "R";
		nodeSpec.Outputs.push_back( pin );
		pin.Name = "G";
		nodeSpec.Outputs.push_back( pin );
		pin.Name = "B";
		nodeSpec.Outputs.push_back( pin );
		pin.Name = "A";
		nodeSpec.Outputs.push_back( pin );

		pin.Name = "Asset";
		pin.Type = PinType::AssetHandle;
		nodeSpec.Inputs.push_back( pin );

		Ref<MaterialSampler2DNode> node = Ref<MaterialSampler2DNode>::Create( nodeSpec );
		rNodeEditor->AddNode( node );

		return node;
	}

	Ref<Node> MaterialNodeLibrary::SpawnMixColors( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification output;
		output.Name = "Out";
		output.Type = PinType::Material_Sampler2D;

		PinSpecification input;
		input.Name = "Color 1";
		input.Type = PinType::Material_Sampler2D;

		NodeSpecification nodeSpec;
		nodeSpec.Name = "Mix Colors";
		nodeSpec.Outputs.push_back( output );
		nodeSpec.Inputs.push_back( input );

		input.Name = "Color 2";
		nodeSpec.Inputs.push_back( input );

		input.Name = "Power";
		input.Type = PinType::Float;
		nodeSpec.Inputs.push_back( input );

		Ref<Node> node = Ref<Node>::Create( nodeSpec );
		node->ExecutionType = NodeExecutionType::Mix;
		rNodeEditor->AddNode( node );

		return node;
	}

	Ref<MaterialOutputNode> MaterialNodeLibrary::SpawnOutputNode( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Albedo";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification nodeSpec;
		nodeSpec.Color = ImColor( 255, 128, 128 );
		nodeSpec.Name = "Material Output";
		nodeSpec.Inputs.push_back( pin );

		pin.Name = "Normal";
		nodeSpec.Inputs.push_back( pin );
		pin.Name = "Metallic";
		nodeSpec.Inputs.push_back( pin );
		pin.Name = "Roughness Map";
		nodeSpec.Inputs.push_back( pin );
		pin.Name = "Emission";
		pin.Type = PinType::Float;
		nodeSpec.Inputs.push_back( pin );
		pin.Name = "Roughness";
		nodeSpec.Inputs.push_back( pin );
		pin.Name = "Metalness";
		nodeSpec.Inputs.push_back( pin );

		Ref<MaterialOutputNode> node = Ref<MaterialOutputNode>::Create( nodeSpec );
		rNodeEditor->AddNode( node );

		return node;
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL OUTPUT NODE

	MaterialOutputNode::MaterialOutputNode( const NodeSpecification& rSpec )
		: Node( rSpec )
	{
		ExecutionType = NodeExecutionType::MaterialOutput;
	}

	MaterialOutputNode::~MaterialOutputNode()
	{
		RuntimeData.MaterialAsset = nullptr;
	}

	void MaterialOutputNode::EvaluateNode( NodeEditorRuntime* evaluator )
	{
		MaterialNodeEditorEvaluator* materialEval = dynamic_cast< MaterialNodeEditorEvaluator* >( evaluator );

		if( !materialEval )
			return;

		auto& TextureStack = materialEval->GetTextureStack();

		while( !TextureStack.empty() )
		{
			const auto& tv = TextureStack.top();
			TextureStack.pop();

			if( tv.Slot == 0 )
			{
				HandleAlbedo( tv );
			}
			else if( tv.Slot == 1 )
			{
				RuntimeData.MaterialAsset->SetNormalMap( tv.TextureAssetID );
			}
			else if( tv.Slot == 2 )
			{
				RuntimeData.MaterialAsset->SetMetallicMap( tv.TextureAssetID );
			}
			else if( tv.Slot == 3 )
			{
				RuntimeData.MaterialAsset->SetRoughnessMap( tv.TextureAssetID );
			}
		}
	}

	void MaterialOutputNode::HandleAlbedo( const MaterialEvaluatorValue& rTextureValue )
	{
		if( rTextureValue.Color.r != 0.0f || rTextureValue.Color.g != 0.0f || rTextureValue.Color.b != 0.0f )
		{
			RuntimeData.MaterialAsset->SetAlbeoColor( rTextureValue.Color );
		}
		else if( rTextureValue.TextureAssetID != 0 )
		{
			Ref<Asset> TextureAsset = nullptr;
			TextureAsset = AssetManager::Get().FindAsset( rTextureValue.TextureAssetID );

			RuntimeData.MaterialAsset->SetAlbeoColor( glm::vec3( 1.0f ) );

			if( TextureAsset )
			{
				RuntimeData.MaterialAsset->SetAlbeoMap( TextureAsset->Path );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL SAMPLER 2D NODE

	MaterialSampler2DNode::MaterialSampler2DNode( const NodeSpecification& rSpec )
		: Node( rSpec )
	{
		ExecutionType = NodeExecutionType::Sampler2D;
	}

	MaterialSampler2DNode::~MaterialSampler2DNode()
	{
	}

	void MaterialSampler2DNode::EvaluateNode( NodeEditorRuntime* evaluator )
	{
		MaterialNodeEditorEvaluator* materialEval = dynamic_cast< MaterialNodeEditorEvaluator* >( evaluator );

		if( !materialEval )
			return;

		if( TextureSlot != UINT64_MAX )
		{
			MaterialEvaluatorValue tv;
			tv.TextureAssetID = 2391113416952765199;
			tv.Slot = static_cast<uint32_t>( TextureSlot );

			// Add to root node
			materialEval->AddToValueStack( tv );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL COLOR PICKER NODE

	MaterialColorPickerNode::MaterialColorPickerNode( const NodeSpecification& rSpec )
		: Node( rSpec )
	{
		ExecutionType = NodeExecutionType::ColorPicker;
	}

	MaterialColorPickerNode::~MaterialColorPickerNode()
	{
	}

	void MaterialColorPickerNode::EvaluateNode( NodeEditorRuntime* evaluator )
	{
		MaterialNodeEditorEvaluator* materialEval = dynamic_cast< MaterialNodeEditorEvaluator* >( evaluator );

		if( !materialEval )
			return;

		if( TextureSlot != UINT64_MAX )
		{
			MaterialEvaluatorValue tv;
			tv.Slot = TextureSlot;
			tv.Color = ExtraData.Read<glm::vec4>();

			// Add to root node
			materialEval->AddToValueStack( tv );
		}
	}
}