/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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

#include "TextureViewer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

// TEMPORARY
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Saturn/Application.h>

namespace Saturn {

	static Ref<Texture2D> m_CheckerboardTex;

	TextureViewer::TextureViewer( void ) : Layer( "TexureViewerPanel" )
	{
	}

	TextureViewer::~TextureViewer( void )
	{
	}

	void TextureViewer::OnAttach( void )
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); ( void )io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		m_CheckerboardTex = Texture2D::Create( "assets/editor/Checkerboard.tga" );

		ImGui_ImplOpenGL3_Init( "#version 410" );
	}

	void TextureViewer::OnDetach( void )
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void DrawBoolControlTextureViewer( const std::string& label, bool* val, float colWidth = 100.0f )
	{
		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2, NULL, false );


		ImGui::SetColumnWidth( 0, colWidth );
		ImGui::Text( "%s", label.c_str() );
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths( 1, ImGui::CalcItemWidth() );
		ImGui::Checkbox( "##value", val );

		ImGui::PopItemWidth();

		ImGui::Columns( 1, NULL, false );

		ImGui::PopID();
	}

	void DrawFloatControlTextureViewer( const std::string& label, float* val, float min = 0.0, float max = 0.0, float step = 1.0f, float colWidth = 100.0f )
	{
		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2, NULL, false );


		ImGui::SetColumnWidth( 0, colWidth );
		ImGui::Text( "%s", label.c_str() );
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths( 1, ImGui::CalcItemWidth() );
		ImGui::DragFloat( "##FLIN", val, step, min, max );

		ImGui::PopItemWidth();

		ImGui::Columns( 1, NULL, false );

		ImGui::PopID();
	}

	void DrawIntControlTextureViewer( const std::string& label, int* val, float min = 0.0, float max = 0.0, float step = 1.0f, float colWidth = 100.0f )
	{
		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2, NULL, false );


		ImGui::SetColumnWidth( 0, colWidth );
		ImGui::Text( "%s", label.c_str() );
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths( 1, ImGui::CalcItemWidth() );
		ImGui::DragInt( "##FLIN", val, step, min, max );

		ImGui::PopItemWidth();

		ImGui::Columns( 1, NULL, false );

		ImGui::PopID();
	}

	void TextureViewer::OnImGuiRender()
	{
		bool p_open = true;

		if( ImGui::Begin( "TextureViewer" ) )
		{
			ImGui::Spacing();

			DrawIntControlTextureViewer( "Set X Size", &PerImagineSizeX, 0.0F, 2500.0F, 1.0F, 75.0F ); ImGui::SameLine(); ImGui::Spacing();
			if( ImGui::Button( "Reset To Texture Size", ImVec2( 175, 25 ) ) )
			{
				PerImagineSizeX = m_CheckerboardTex->GetWidth();
				PerImagineSizeY = m_CheckerboardTex->GetWidth();
				Reset = false;
			}
			DrawIntControlTextureViewer( "Set Y Size", &PerImagineSizeY, 0.0F, 2500.0F, 1.0F, 75.0F );
			if( ImGui::Button( "Half Size", ImVec2( 125, 25 ) ) )
			{
				if( PerImagineSizeX == 0 && PerImagineSizeY == 0 )
				{
					PerImagineSizeX = m_CheckerboardTex->GetWidth();
					PerImagineSizeY = m_CheckerboardTex->GetWidth();
					Reset = false;
				}

				PerImagineSizeX = PerImagineSizeX / 2;
				PerImagineSizeY = PerImagineSizeY / 2;
			}
			if( ImGui::Button( "Open New Texture", ImVec2( 125, 25 ) ) )
			{
				std::string filepath = Application::Get().OpenFile( "Texture 2D (*.tga *.png)\0*.tga; *.png\0" ).first;
				m_CheckerboardTex = nullptr;
				m_CheckerboardTex = Texture2D::Create( filepath, false );

				Reset = true;
			}
			if( m_CheckerboardTex )
			{
				ImGui::SameLine();
				ImGui::Text( "File Path: %s", m_CheckerboardTex->GetPath().c_str() );
			}
			ImGui::Image( ( ImTextureID )m_CheckerboardTex->GetRendererID(), ImVec2( PerImagineSizeX, PerImagineSizeY ) );
		}
		ImGui::End();
	}

	void TextureViewer::OnUpdate( Timestep ts )
	{
	}

	void TextureViewer::OnEvent( Event& e )
	{
	}

	void TextureViewer::SetRenderImageTarget( std::string filepath )
	{
		m_CheckerboardTex = nullptr;
		Ref<Texture2D> newTexture = Texture2D::Create( filepath );
		m_CheckerboardTex = newTexture;
	}

	bool TextureViewer::OnMouseButtonPressed( MouseButtonEvent& e )
	{
		return true;
	}

	bool TextureViewer::OnKeyPressedEvent( KeyPressedEvent& e )
	{
		return true;
	}

}