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

#include "AssetPanel.h"

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

#include <filesystem>

namespace Saturn {

	AssetPanel::AssetPanel( void ) : Layer( "AssetPanel" )
	{
	}

	AssetPanel::~AssetPanel( void )
	{

	}

	void AssetPanel::OnAttach( void )
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); ( void )io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		m_CheckerboardTex = Texture2D::Create( "assets/editor/Checkerboard.tga" );

		namespace fs = std::filesystem;

		for( fs::recursive_directory_iterator it( "assets/" ); it != fs::recursive_directory_iterator(); ++it )
		{
			m_AssetsFolderContents.push_back( it->path().string() );

			if( it->path().extension() == ".asset" )
			{
				m_Assets.push_back( it->path().string() );
			}
		}

		ImGui_ImplOpenGL3_Init( "#version 410" );
	}

	void AssetPanel::OnDetach( void )
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void AssetPanel::OnUpdate( Timestep ts )
	{

	}

	void AssetPanel::OnImGuiRender()
	{
		bool p_open = true;
		namespace fs = std::filesystem;

		std::string asset_path = "assets";

		int selected = ( 1 << 2 );

		unsigned int idx;

		if( ImGui::Begin( "AssetPanel", &p_open ) )
		{
			for( fs::recursive_directory_iterator it( asset_path ); it != fs::recursive_directory_iterator(); ++it )
			{
				if ( fs::is_directory( it->path() ) )
				{
					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Indent();
					}

					if( ImGui::TreeNode( it->path().filename().string().c_str() ) )
					{
						ImGui::TreePop();
					}

					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Unindent();
					}
				}
				else
				{
					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Indent();
					}

					ImGui::Text( it->path().filename().string().c_str() );

					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Unindent();
					}
				}
			}
		}
		ImGui::End();

		if( ImGui::Begin( "Assets", &p_open ) )
		{
			std::string m_FolderPath = "abc..";

			for( fs::recursive_directory_iterator it( asset_path ); it != fs::recursive_directory_iterator(); ++it )
			{
				{

					if ( !it->path().has_extension() )
					{
						m_FolderPath = it->path().string();
					}

					if( it->path().extension() == ".png" )
					{
						m_FolderPath = m_FolderPath;

						if ( CheckHasAsset( it->path().filename().string(), it->path().string(), m_FolderPath ) == false )
						{
							//Ref<Texture2D> PngImage = Texture2D::Create("assets/");
							//ImGui::ImageButton( ( ImTextureID )PngImage->GetRendererID(), ImVec2( 64, 64 ) );
						}
					}

					if( it->path().extension() == ".tga" )
					{
						m_FolderPath = m_FolderPath;

					}

				}

			}
		}
		ImGui::End();
	}

	bool AssetPanel::CheckHasAsset( std::string name, std::string filepath, std::string folder )
	{
		namespace fs = std::filesystem;

		for( fs::directory_iterator it( folder ); it != fs::directory_iterator(); ++it )
		{
			if ( it->path().filename() == name && std::find( m_Assets.begin(), m_Assets.end(), name ) == m_Assets.end() && it->path().filename() != name + ".asset" )
			{
				m_Assets.push_back( name );

				//fs::path asset( folder + name + ".asset" );

				//fs::remove( folder + name + ".asset" );

				return false;
			}
			else if( it->path().filename() == name + ".asset" )
			{
				return true;
			}
		}
	}

	void AssetPanel::OnEvent( Event& e )
	{

	}

	bool AssetPanel::OnMouseButtonPressed( MouseButtonEvent& e )
	{

	}

	bool AssetPanel::OnKeyPressedEvent( KeyPressedEvent& e )
	{

	}

}