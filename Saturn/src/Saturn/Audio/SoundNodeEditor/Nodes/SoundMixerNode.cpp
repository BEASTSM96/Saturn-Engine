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
#include "SoundMixerNode.h"

#if !defined(SAT_DIST)
#include "Saturn/NodeEditor/UI/NodeEditor.h"
#else
#include "Saturn/NodeEditor/NodeEditorBase.h"
#endif

#include "SoundPlayerNode.h"
#include "SoundRandomNode.h"

#include "Saturn/Audio/SoundNodeEditor/SoundEditorEvaluator.h"

namespace Saturn {

	SoundMixerNode::SoundMixerNode( const NodeSpecification& rSpec )
		: Node( rSpec )
	{
		ExecutionType = NodeExecutionType::SoundMixer;
	}

	SoundMixerNode::~SoundMixerNode()
	{
	}

	NodeEditorCompilationStatus SoundMixerNode::EvaluateNode( NodeEditorRuntime* evaluator )
	{
		SoundEditorEvaluator* pSoundEditorEvaluator = dynamic_cast< SoundEditorEvaluator* >( evaluator );

		if( !pSoundEditorEvaluator )
			return NodeEditorCompilationStatus::Failed;

#if !defined( SAT_DIST )
		auto count = std::count_if( Inputs.begin(), Inputs.end(),
			[=]( const auto& pin )
			{
				return pSoundEditorEvaluator->GetTargetNodeEditor()->IsLinked( pin->ID );
			} );

		if( count != Inputs.size() )
		{
			Ref<NodeEditor> uiEditor = pSoundEditorEvaluator->GetTargetNodeEditor().As<NodeEditor>();

			uiEditor->ThrowError( "Not all pins are linked to the mixer node!" );

			return NodeEditorCompilationStatus::Failed;
		}
#endif

		std::map<UUID, UUID> PinToSoundMap;
		uint32_t index = 0;

		auto ids = pSoundEditorEvaluator->GetTargetNodeEditor()->FindNeighbors( this );
		for( const auto& rID : ids )
		{
			Ref<Node> neighorNode = pSoundEditorEvaluator->GetTargetNodeEditor()->FindNode( rID );
			if( !neighorNode )
				continue;

			switch( neighorNode->ExecutionType )
			{
				case NodeExecutionType::SoundPlayer:
				{
					Ref<SoundPlayerNode> playerNode = neighorNode.As<SoundPlayerNode>();
					PinToSoundMap[ index ] =  playerNode->SoundAssetID;
				} break;

				case NodeExecutionType::RandomSound:
				{
					Ref<SoundRandomNode> randomNode = neighorNode.As<SoundRandomNode>();
					PinToSoundMap[ index ] = randomNode->ChosenSoundID;
				} break;

				// TODO:
				/*
				case NodeExecutionType::SoundMixer:
				{
					Ref<SoundPlayerNode> playerNode = neighorNode.As<SoundPlayerNode>();
					PinToSoundMap[ index ] = playerNode->SoundAssetID;
				} break;
				*/
			}

			index++;
		}

		for( const auto& [ index, assetID ] : PinToSoundMap )
		{
			pSoundEditorEvaluator->SoundStack.push( assetID );
		}

		return NodeEditorCompilationStatus::Success;
	}

}