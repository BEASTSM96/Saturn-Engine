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
#include "MaterialNodeEditorEvaluator.h"

#include "MaterialViewerNodes.h"

#include "Saturn/NodeEditor/Node.h"
#include "Saturn/NodeEditor/NodeEditorBase.h"
#include "Saturn/Vulkan/Mesh.h"
#include "Saturn/Vulkan/Material.h"
#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

namespace Saturn {

	MaterialNodeEditorEvaluator::MaterialNodeEditorEvaluator( const MaterialNodeEdInfo& rInfo )
		: m_Info( rInfo )
	{
	}

	NodeEditorCompilationStatus MaterialNodeEditorEvaluator::EvaluateEditor()
	{
		if( !m_NodeEditor )
			return NodeEditorCompilationStatus::Failed;

		Ref<Node> OutputNode = m_NodeEditor->FindNode( m_Info.OutputNodeID );
		if( !OutputNode )
			return NodeEditorCompilationStatus::Failed;

		UUID AlbedoPinID = OutputNode->Inputs[ 0 ]->ID;
		UUID NormalPinID = OutputNode->Inputs[ 1 ]->ID;
		UUID MetallicPinID = OutputNode->Inputs[ 2 ]->ID;
		UUID RoughnessPinID = OutputNode->Inputs[ 3 ]->ID;

		// We must have something linked to the albedo.
		if( !m_NodeEditor->IsLinked( AlbedoPinID ) )
			return NodeEditorCompilationStatus::Failed;

		// Stacks are last in first out, so our output node will be evaluated last which is what we want.
		std::stack<UUID> order;
		m_NodeEditor->TraverseFromStart( OutputNode,
			[&]( const UUID id )
			{
				order.push( id );
			} );

		// If we only have one node (output node always exists) then we fail as we need something connected to the albedo pin.
		if( order.size() <= 1 )
			return NodeEditorCompilationStatus::Failed;

		m_Info.HostMaterial->Reset();

		auto findLinkEndpoint = [=]( const Ref<Link>& rLink, UUID AnyPinID ) -> Ref<Pin>
			{
				UUID TargetLinkEnd = 0;

				if( rLink->StartPinID == AnyPinID )
					TargetLinkEnd = rLink->EndPinID;
				else
					TargetLinkEnd = rLink->StartPinID;

				return m_NodeEditor->FindPin( TargetLinkEnd );
			};

		while( !order.empty() )
		{
			const UUID currentNodeID = order.top();
			order.pop();

			Ref<Node> currentNode = m_NodeEditor->FindNode( currentNodeID );
			
			switch( currentNode->ExecutionType )
			{
				case NodeExecutionType::ColorPicker:
				{
					// Because we are a color picker check to make sure if we are linked to the output node
					// If so then create the texture value.
					size_t index = IsOutputsLinkedToOutNode( currentNode );

					Ref<MaterialColorPickerNode> pickerNode = currentNode.As<MaterialColorPickerNode>();
					pickerNode->TextureSlot = index;

					currentNode = pickerNode;
				} break;

				case NodeExecutionType::Sampler2D:
				{
					size_t index = IsOutputsLinkedToOutNode( currentNode );

					Ref<MaterialSampler2DNode> sampler2DNode = currentNode.As<MaterialSampler2DNode>();
					sampler2DNode->TextureSlot = index;

					currentNode = sampler2DNode;
				} break;

				case NodeExecutionType::MaterialOutput:
				{
					Ref<MaterialOutputNode> outNode = currentNode.As<MaterialOutputNode>();
					outNode->RuntimeData.MaterialAsset = m_Info.HostMaterial;
					currentNode = outNode;
				} break;

				/*
				// Math nodes
				case NodeExecutionType::Add:
				{
					Ref<Link> link = m_NodeEditor->FindLinkByPin( currentNode->Outputs[ 0 ]->ID );
					Ref<Pin> endpoint = findLinkEndpoint( link, currentNode->Outputs[ 0 ]->ID );

					Ref<Pin> floatPin1 = currentNode->Inputs[ 0 ];
					Ref<Pin> floatPin2 = currentNode->Inputs[ 1 ];

					//////

					nodeRT = Ref<AddFloatRuntime>::Create( currentNodeID );
					auto outputRT = nodeRT.As<AddFloatRuntime>();

					outputRT->Value1 = floatPin1->ExtraData.Read<float>( 0 * sizeof( float ) );
					outputRT->Value2 = floatPin2->ExtraData.Read<float>( 1 * sizeof( float ) );
					outputRT->OutPin = endpoint;
				} break;

				case NodeExecutionType::Subtract:
				{
					Ref<Pin> floatPin1 = currentNode->Inputs[ 0 ];
					Ref<Pin> floatPin2 = currentNode->Inputs[ 1 ];

					Ref<Link> link = m_NodeEditor->FindLinkByPin( currentNode->Outputs[ 0 ]->ID );
					Ref<Pin> endpoint = findLinkEndpoint( link, currentNode->Outputs[ 0 ]->ID );

					//////

					nodeRT = Ref<SubtractFloatRuntime>::Create( currentNodeID );
					auto outputRT = nodeRT.As<SubtractFloatRuntime>();

					outputRT->Value1 = floatPin1->ExtraData.Read<float>( 0 * sizeof( float ) );
					outputRT->Value2 = floatPin2->ExtraData.Read<float>( 1 * sizeof( float ) );
					outputRT->OutPin = endpoint;
				} break;

				case NodeExecutionType::Multiply:
				{
					Ref<Link> link = m_NodeEditor->FindLinkByPin( currentNode->Outputs[ 0 ]->ID );
					Ref<Pin> endpoint = findLinkEndpoint( link, currentNode->Outputs[ 0 ]->ID );

					Ref<Pin> floatPin1 = currentNode->Inputs[ 0 ];
					Ref<Pin> floatPin2 = currentNode->Inputs[ 1 ];

					//////

					nodeRT = Ref<MultiplyFloatRuntime>::Create( currentNodeID );
					auto outputRT = nodeRT.As<MultiplyFloatRuntime>();

					outputRT->Value1 = floatPin1->ExtraData.Read<float>( 0 * sizeof( float ) );
					outputRT->Value2 = floatPin2->ExtraData.Read<float>( 1 * sizeof( float ) );
					outputRT->OutPin = endpoint;
				} break;

				case NodeExecutionType::Divide:
				{
					Ref<Pin> floatPin1 = currentNode->Inputs[ 0 ];
					Ref<Pin> floatPin2 = currentNode->Inputs[ 1 ];

					Ref<Link> link = m_NodeEditor->FindLinkByPin( currentNode->Outputs[ 0 ]->ID );
					Ref<Pin> endpoint = findLinkEndpoint( link, currentNode->Outputs[ 0 ]->ID );

					//////

					nodeRT = Ref<DivideFloatRuntime>::Create( currentNodeID );
					auto outputRT = nodeRT.As<DivideFloatRuntime>();

					outputRT->Value1 = floatPin1->ExtraData.Read<float>( 0 * sizeof( float ) );
					outputRT->Value2 = floatPin2->ExtraData.Read<float>( 1 * sizeof( float ) );
					outputRT->OutPin = endpoint;
				} break;
				*/
			}

			currentNode->EvaluateNode( this );
		}

		m_Info.HostMaterial->ApplyChanges();

		m_NodeEditor->ShowFlow();

		// Save the material
		MaterialAssetSerialiser mas;
		mas.Serialise( m_Info.HostMaterial );

		return NodeEditorCompilationStatus::Success;
	}

	size_t MaterialNodeEditorEvaluator::IsOutputsLinkedToOutNode( const Ref<Node>& rNode )
	{
		Ref<Node> outputNode = m_NodeEditor->FindNode( m_Info.OutputNodeID );

		size_t i = 0;
		for( const auto& rOutput : rNode->Outputs )
		{
			Ref<Link> link = m_NodeEditor->FindLinkByPin( rOutput->ID );
			if( !link )
				continue;

			// Inputs are always the end id.
			if( link->EndPinID == outputNode->Inputs[ i ]->ID )
			{
				return i;
			}

			i++;
		}

		return UINT64_MAX;
	}

	void MaterialNodeEditorEvaluator::AddToValueStack( const MaterialEvaluatorValue& rValue )
	{
		m_ValueStack.push( rValue );
	}
}