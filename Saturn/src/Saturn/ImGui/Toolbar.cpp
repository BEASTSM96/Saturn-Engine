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
	}

	Toolbar::~Toolbar()
	{
		delete m_pPlayImage;
		delete m_pPauseImage;
		delete m_pStopImage;
	}

	void Toolbar::Draw()
	{
		ImGui::Begin( "Toolbar", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
		
		// Center the play button.
		ImGui::SetCursorPosX( ImGui::GetWindowWidth() / 2 - m_pPlayImage->Width() / 2 );

		VkDescriptorSet ImageSet = WantsToStartRuntime ? m_pStopImage->GetDescriptorSet() : m_pPlayImage->GetDescriptorSet();

		if( ImGui::ImageButton( ImageSet, ImVec2( 32, 32 ) ) )
		{
			WantsToStartRuntime = !WantsToStartRuntime;
		}
		
		ImGui::End();
	}

}