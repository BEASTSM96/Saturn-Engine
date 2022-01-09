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
#include "Viewport.h"
#include "Saturn/Core/App.h"

#include "imgui.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	Viewport::Viewport()
	{
	}

	void Viewport::Draw()
	{
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
		
		ImGui::Begin( "Viewport", 0, flags );

		m_SendCameraEvents = ImGui::IsWindowFocused();

	#if !defined( SAT_DONT_USE_GL ) 

		Renderer::Get().RendererCamera().AllowEvents( m_SendCameraEvents );
		Renderer::Get().RendererCamera().SetActive( m_SendCameraEvents );

		Renderer::Get().RendererCamera().OnUpdate( Application::Get().Time() );

		auto viewportSize = ImGui::GetContentRegionAvail();

		Renderer::Get().RendererCamera().SetProjectionMatrix( glm::perspectiveFov( glm::radians( 45.0f ), viewportSize.x, viewportSize.y, 0.1f, 10000.0f ) );
		Renderer::Get().RendererCamera().SetViewportSize( viewportSize.x, viewportSize.y );

		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 2, 2 ) );
		ImGui::Image( ( void* )( Renderer::Get().GetFinalColorBufferRendererID() ), viewportSize, { 0, 1 }, { 1, 0 } );
		ImGui::PopStyleVar();
	#endif

		ImGui::End();

		ImGui::PopStyleVar();
	}

}