/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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
#include "Toolbar.h"

namespace Saturn {

	Toolbar::Toolbar()
	{
		// Create and load the images.
		
		m_pPlayImage  = new Texture2D( "assets/textures/PlayButton.png", AddressingMode::Repeat );
		m_pPauseImage = new Texture2D( "assets/textures/PauseButton.png", AddressingMode::Repeat );
		m_pStopImage  = new Texture2D( "assets/textures/StopButton.png", AddressingMode::Repeat );

		m_pPlayDescSet = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_pPlayImage->GetSampler(), m_pPlayImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
		m_pPauseDescSet = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_pPauseImage->GetSampler(), m_pPauseImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
		m_pStopDescSet = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_pStopImage->GetSampler(), m_pStopImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
	}

	Toolbar::~Toolbar()
	{
		delete m_pPlayImage;
		delete m_pPauseImage;
		delete m_pStopImage;
	}

	void Toolbar::Draw()
	{
		// Window flags.
		ImGuiWindowFlags WindowFlags = 0;
		
		WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
		
		// Hide the tab bar.
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 3.0f ) );

		ImGui::Begin( "Toolbar", nullptr, WindowFlags );
		
		// Center the play button.
		ImGui::SetCursorPosX( ImGui::GetWindowWidth() / 2 - m_pPlayImage->Width() / 2 );
		
		VkDescriptorSet ImageSet = WantsToStartRuntime ? m_pStopDescSet : m_pPlayDescSet;

		if( ImGui::ImageButton( ImageSet, ImVec2( 32, 32 ) ) )
		{
			WantsToStartRuntime = !WantsToStartRuntime;
		}
		
		ImGui::PopStyleVar();

		ImGui::End();
	}

}