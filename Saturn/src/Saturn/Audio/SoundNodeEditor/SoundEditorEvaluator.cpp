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
#include "SoundEditorEvaluator.h"

#include "Saturn/Audio/Sound.h"

#include "Nodes/SoundOutputNode.h"
#include "Nodes/SoundPlayerNode.h"
#include "Nodes/SoundRandomNode.h"

#include "Saturn/NodeEditor/NodeEditorBase.h"

namespace Saturn {

	SoundEditorEvaluator::SoundEditorEvaluator( const SoundEdEvaluatorInfo& rInfo )
		: m_Info( rInfo )
	{
	}

	void SoundEditorEvaluator::SetTargetNodeEditor( Ref<NodeEditorBase> nodeEditor )
	{
		m_NodeEditor = nodeEditor;
	}

	NodeEditorCompilationStatus SoundEditorEvaluator::EvaluateEditor()
	{
		if( !m_NodeEditor )
			return NodeEditorCompilationStatus::Failed;

		Ref<Node> OutputNode = m_NodeEditor->FindNode( m_Info.OutputNodeID );
		if( !OutputNode )
			return NodeEditorCompilationStatus::Failed;

		UUID FinalSoundPinID = OutputNode->Inputs[ 0 ]->ID;

		// We must have something linked to the final output.
		if( !m_NodeEditor->IsLinked( FinalSoundPinID ) )
			return NodeEditorCompilationStatus::Failed;

		// Stacks are last in first out, so our output node will be evaluated last which is what we want.
		std::stack<UUID> order;
		m_NodeEditor->TraverseFromStart( OutputNode,
			[&]( const UUID id )
			{
				order.push( id );
			} );

		if( order.size() <= 1 )
			return NodeEditorCompilationStatus::Failed;
	
		std::stack<UUID> soundAssetStack;

		while( !order.empty() )
		{
			const UUID currentNodeID = order.top();
			order.pop();

			Ref<Node> currentNode = m_NodeEditor->FindNode( currentNodeID );

			switch( currentNode->ExecutionType )
			{
				case NodeExecutionType::SoundOutput:
				{
					Ref<SoundOutputNode> outNode = currentNode.As<SoundOutputNode>();
					currentNode = outNode;
				} break;

				case NodeExecutionType::SoundPlayer:
				{
					Ref<SoundPlayerNode> playerNode = currentNode.As<SoundPlayerNode>();
				
					if( playerNode && playerNode->SoundAssetID != 0 )
						soundAssetStack.push( playerNode->SoundAssetID );
				} break;
			}

			currentNode->EvaluateNode( this );
		}

		return NodeEditorCompilationStatus::Success;
	}
}