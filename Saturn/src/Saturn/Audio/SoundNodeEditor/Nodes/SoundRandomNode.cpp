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
#include "SoundRandomNode.h"

#include "Saturn/NodeEditor/NodeEditorBase.h"

#include "SoundPlayerNode.h"
#include "Saturn/Audio/SoundNodeEditor/SoundEditorEvaluator.h"

namespace Saturn {

	SoundRandomNode::SoundRandomNode( const NodeSpecification& rSpec )
		: Node( rSpec )
	{
		ExecutionType = NodeExecutionType::Random;
	}

	SoundRandomNode::~SoundRandomNode()
	{
	}

	void SoundRandomNode::EvaluateNode( NodeEditorRuntime* evaluator )
	{
		SoundEditorEvaluator* pSoundEditorEvaluator = dynamic_cast< SoundEditorEvaluator* >( evaluator );

		if( !pSoundEditorEvaluator )
			return;

		std::map<UUID, UUID> PinToSoundMap;

		auto ids = pSoundEditorEvaluator->GetTargetNodeEditor()->FindNeighbors( this );

		uint32_t index = 0;
		for( const auto& rID : ids )
		{
			Ref<Node> neighorNode = pSoundEditorEvaluator->GetTargetNodeEditor()->FindNode( rID );
			if( !neighorNode )
				continue;

			// TODO: Support more node types coming into the random node
			if( neighorNode->ExecutionType != NodeExecutionType::SoundPlayer )
				continue;

			Ref<SoundPlayerNode> playerNode = neighorNode.As<SoundPlayerNode>();
			PinToSoundMap[ index ] = playerNode->SoundAssetID;

			index++;
		}

		int range = (int)Inputs.size();
		int number = std::rand() % range + 0;

		ChosenSoundID = PinToSoundMap[ number ];

		pSoundEditorEvaluator->SoundStack.push( ChosenSoundID );
	}

	void SoundRandomNode::OnRenderOutput( UUID pinID )
	{
	}
}