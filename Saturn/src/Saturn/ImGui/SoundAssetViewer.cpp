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
#include "SoundAssetViewer.h"

#include "Saturn/Asset/AssetManager.h"

#include "ImGuiAuxiliary.h"
#include "EditorIcons.h"

#include <imgui.h>

namespace Saturn {

	SoundAssetViewer::SoundAssetViewer( AssetID id )
		: AssetViewer( id )
	{
		m_AssetType = AssetType::Sound;
		
		Ref<Sound> snd = AssetManager::Get().GetAssetAs<Sound>( m_AssetID );
		m_SoundAsset = snd;

		m_Open = true;
	}

	SoundAssetViewer::~SoundAssetViewer()
	{
	}

	void SoundAssetViewer::OnImGuiRender()
	{
		ImGui::PushID( static_cast<int>( m_SoundAsset->ID ) );

		ImGui::Begin( "Sound", &m_Open );

		ImGui::BeginVertical( "##settings_hor" );

		ImGui::Text( "Source Sound file" );

		ImGui::BeginHorizontal( "##srcsndfile" );

		if( Auxiliary::ImageButton( EditorIcons::GetIcon( "Inspect" ), { 24, 24 } ) ) 
		{
			std::string filepath = Application::Get().OpenFile( "Supported asset types (*.wav, *.mp3, *.ogg)\0*.wav; *.mp3; *.ogg\0" );

			if( !filepath.empty() )
			{
				auto& currentRawPath = m_SoundAsset->GetRawPath();
				currentRawPath = currentRawPath.parent_path();

				std::filesystem::path path = filepath;

				std::filesystem::path newPath = currentRawPath;
				currentRawPath /= path.filename();

				if( !std::filesystem::exists( filepath ) )
					std::filesystem::copy_file( filepath, newPath );

				m_SoundAsset->SetRawPath( newPath );

				// TODO: Show an editor dialog message if the user would like to delete the old source.
			}
		}

		ImGui::InputText( "##filepath", ( char* ) m_SoundAsset->GetRawPath().string().c_str(), 4096, ImGuiInputTextFlags_ReadOnly );

		ImGui::EndHorizontal();

		ImGui::EndVertical();

		ImGui::End();

		ImGui::PopID();
	}

}