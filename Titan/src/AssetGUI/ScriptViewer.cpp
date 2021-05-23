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

#include "ScriptViewer.h"

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

	ScriptViewerStandalone::ScriptViewerStandalone( void ) : Layer( "ScriptViewerStandalone" )
	{
	}

	ScriptViewerStandalone::~ScriptViewerStandalone( void )
	{
	}

	void ScriptViewerStandalone::OnAttach( void )
	{
	}

	void ScriptViewerStandalone::OnDetach( void )
	{
	}

	void ScriptViewerStandalone::OnImGuiRender()
	{
		if( ImGui::Begin( "Script Viewer Standalone" ) )
		{
			if( ImGui::Button( "OpenScript", ImVec2( 100, 50 ) ) )
			{
				if( !m_Filepath.empty() )
					m_Filepath = "";
		
				if( !m_FileLines.empty() )
					m_FileLines = "";

				m_Filepath = Application::Get().OpenFile( "C# Script (*.cs)\0*.cs;" ).first;

				m_File = std::ifstream( m_Filepath );

				if( m_File.is_open() )
				{
					std::string line;
					std::string fullText;
		
					while( std::getline( m_File, line ) )
					{
						fullText = fullText + m_FileLines + line + "\n";
					}
					m_FileLines = "\n" + fullText;

					m_File.close();
				}
			}

			ImGui::SameLine();

			ImGui::Text( "File Path: %s", m_Filepath.c_str() );

			ImGui::Separator();

			ImGui::Text( m_FileLines.c_str() );

		}
		ImGui::End();
	}

	void ScriptViewerStandalone::OnUpdate( Timestep ts )
	{
	}

	void ScriptViewerStandalone::OnEvent( Event& e )
	{
	}

	bool ScriptViewerStandalone::OnMouseButtonPressed( MouseButtonEvent& e )
	{
		return true;
	}

	bool ScriptViewerStandalone::OnKeyPressedEvent( KeyPressedEvent& e )
	{
		return true;
	}

	ScriptViewerEntity::ScriptViewerEntity( void ) : Layer( "ScriptViewerEntity" )
	{
	}

	ScriptViewerEntity::~ScriptViewerEntity( void )
	{
	}

	void ScriptViewerEntity::OnAttach( void )
	{

	}

	void ScriptViewerEntity::OnDetach( void )
	{

	}

	void ScriptViewerEntity::OnImGuiRender()
	{

	}

	void ScriptViewerEntity::OnUpdate( Timestep ts )
	{

	}

	void ScriptViewerEntity::OnEvent( Event& e )
	{

	}

	bool ScriptViewerEntity::OnMouseButtonPressed( MouseButtonEvent& e )
	{
		return true;
	}

	bool ScriptViewerEntity::OnKeyPressedEvent( KeyPressedEvent& e )
	{
		return true;
	}

}