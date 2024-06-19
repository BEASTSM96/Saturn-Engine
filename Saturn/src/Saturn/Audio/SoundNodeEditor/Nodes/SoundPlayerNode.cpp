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
#include "SoundPlayerNode.h"

#include "Saturn/ImGui/ImGuiAuxiliary.h"

namespace Saturn {

	SoundPlayerNode::SoundPlayerNode( const NodeSpecification& rSpec )
		: Node( rSpec )
	{
		ExecutionType = NodeExecutionType::SoundPlayer;
	}

	SoundPlayerNode::~SoundPlayerNode()
	{
	}

	void SoundPlayerNode::EvaluateNode( NodeEditorRuntime* evaluator )
	{
	}

	static bool s_ShowAssetFinder = false;

	void SoundPlayerNode::OnRenderOutput( UUID pinID )
	{
		const auto itr = std::find_if( Outputs.begin(), Outputs.end(),
			[pinID]( const auto& rPin )
			{
				return rPin->ID == pinID;
			} );

		if( itr == Outputs.end() )
			return;

		const auto& rPin = ( *itr );

		switch( rPin->Type )
		{
			case PinType::Sound:
			{
				std::string name = "Select Asset";

				if( SoundAssetID != 0 )
					name = std::to_string( SoundAssetID );

				if( ImGui::Button( name.c_str() ) )
				{
					s_ShowAssetFinder = true;
				}
			} break;
		}

		ed::Suspend();
		Auxiliary::DrawAssetFinder( AssetType::Sound, &s_ShowAssetFinder, SoundAssetID );
		ed::Resume();
	}

	void SoundPlayerNode::OnSerialise( std::ofstream& rStream ) const
	{
		UUID::Serialise( SoundAssetID, rStream );
		SAT_CORE_INFO( "saved id: {0}", SoundAssetID );
	}

	void SoundPlayerNode::OnDeserialise( std::ifstream& rStream )
	{
		UUID::Deserialise( SoundAssetID, rStream );
		SAT_CORE_INFO( "read id: {0}", SoundAssetID );
	} 
}