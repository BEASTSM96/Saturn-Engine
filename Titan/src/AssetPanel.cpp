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
#include "AssetGUI/TextureViewer.h"
#include "EditorLayer.h"
#include "Saturn/Renderer/Texture.h"

#include "Saturn/Core/Assets/FileCollection.h"
#include "Saturn/Core/Assets/PNGFile.h"


namespace Saturn {

	AssetPanel::AssetPanel( void ) : Layer( "AssetPanel" )
	{
	}

	AssetPanel::~AssetPanel( void )
	{

	}

	void AssetPanel::OnAttach( void )
	{
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

	}

	void AssetPanel::OnDetach( void )
	{
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
			std::string m_FolderPath = "abc..";

			for( fs::recursive_directory_iterator it( asset_path ); it != fs::recursive_directory_iterator(); ++it )
			{
				if( fs::is_directory( it->path() ) )
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

					if( !it->path().has_extension() )
					{
						m_FolderPath = it->path().string();
					}

				}
				else
				{
					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Indent();
					}

					if( it->path().extension() == ".png" || it->path().extension() == ".tga" || it->path().extension() == ".hdr" )
					{
						if( ImGui::Button( it->path().filename().string().c_str() ) )
						{
							ImGui::Begin( "TextureViewer" );
							TextureViewer::SetRenderImageTarget( m_FolderPath + "\\" + it->path().filename().string().c_str() );
							ImGui::End();
						}
					}
					else
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
			ImGui::Text( "File Path: %s", m_FolderPath.c_str() );

			if( m_FolderPath == "assets" )
			{
				ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
				ImGui::Button( "Back" );
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
			else
			{
				if( ImGui::Button( "Back" ) )
				{
					fs::path currentPath( m_FolderPath );
					fs::path root_path = currentPath.parent_path();

					m_FolderPath = root_path.string();

				}

			}

			if( ImGui::Button( "Scan All Folders for Assets" ) )
			{
				for( fs::recursive_directory_iterator it( "assets\\" ); it != fs::recursive_directory_iterator(); ++it )
				{
					if( it->path().extension().string() == ".sc" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  "assets\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::SCENE );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".txt" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  "assets\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::TEXT );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".png" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path = it->path().string();

							Ref<PNGFile>& scFile = Ref<PNGFile>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::PNG );

							FileCollection::AddFileToCollection( ( Ref<File> )scFile );
						}
					}

					if( it->path().extension().string() == ".tga" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path = it->path().string();

							Ref<PNGFile>& scFile = Ref<PNGFile>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::TGA );

							FileCollection::AddFileToCollection( ( Ref<File> )scFile );
						}
					}

					if( it->path().extension().string() == ".obj" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  "assets\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::OBJ );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".fbx" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  "assets\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::FBX );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".c#" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  "assets\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::SCRIPT );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".glsl" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  "assets\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::SHADER );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".hdr" )
					{
						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path = it->path().string();

							Ref<PNGFile>& scFile = Ref<PNGFile>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::HDR );

							FileCollection::AddFileToCollection( ( Ref<File> )scFile );
						}
					}
				}
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			for( fs::directory_iterator it( m_FolderPath ); it != fs::directory_iterator(); ++it )
			{
				if( !it->path().has_extension() )
				{
					if( ImGui::Button( it->path().filename().string().c_str() ) )
					{
						m_CurrentFolder = it->path().filename().string().c_str();
						m_FolderPath = m_FolderPath + "\\" + it->path().filename().string();
					}
					ImGui::SameLine();
				}
			}

			for( fs::directory_iterator it( m_FolderPath ); it != fs::directory_iterator(); ++it )
			{
				if( it->path().has_extension() )
				{
					ImGui::SameLine();

					if( it->path().extension().string() == ".sc" )
					{
						std::string path = m_FolderPath + "\\" + it->path().filename().string();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::SCENE );

							FileCollection::AddFileToCollection( scFile );
						}

						if( ImGui::Button( it->path().filename().string().c_str(), ImVec2( 64, 64 ) ) )
						{
							
						}

					}

					if( it->path().extension().string() == ".png" )
					{
						std::string path = m_FolderPath + "\\" + it->path().filename().string();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							Ref<PNGFile>& scFile = Ref<PNGFile>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::PNG );

							FileCollection::AddFileToCollection( ( Ref<File> )scFile );
						}

						Ref<PNGFile> file = FileCollection::GetFile( it->path().filename().string() );

						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::ImageButton( ( ImTextureID )file->GetData()->GetRendererID(), ImVec2( 64, 64 ) );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();

					}

					if( it->path().extension().string() == ".tga" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							Ref<PNGFile>& scFile = Ref<PNGFile>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::TGA );

							FileCollection::AddFileToCollection( ( Ref<File> )scFile );
						}

						Ref<PNGFile> file = FileCollection::GetFile( it->path().filename().string() );

						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::ImageButton( ( ImTextureID )file->GetData()->GetRendererID(), ImVec2( 64, 64 ) );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();
					}

					if( it->path().extension().string() == ".hdr" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							Ref<PNGFile>& scFile = Ref<PNGFile>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::HDR );

							FileCollection::AddFileToCollection( ( Ref<File> )scFile );
						}

						Ref<PNGFile> file = FileCollection::GetFile( it->path().filename().string() );

						if( ImGui::ImageButton( ( ImTextureID )file->GetData()->GetRendererID(), ImVec2( 64, 64 ) ) )
						{
							TextureViewer::SetRenderImageTarget( file->GetData() );
						}

					}

					if( it->path().extension().string() == ".c#" )
					{
						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::Button( it->path().filename().string().c_str() );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  m_FolderPath + "\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::SCRIPT );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".obj" )
					{
						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::Button( it->path().filename().string().c_str() );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  m_FolderPath + "\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::OBJ );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".fbx" )
					{
						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::Button( it->path().filename().string().c_str() );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  m_FolderPath + "\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::FBX );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".txt" )
					{
						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::Button( it->path().filename().string().c_str() );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  m_FolderPath + "\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::TEXT );

							FileCollection::AddFileToCollection( scFile );
						}
					}

					if( it->path().extension().string() == ".glsl" )
					{
						ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
						ImGui::Button( it->path().filename().string().c_str() );
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();

						if( !FileCollection::DoesFileExistInCollection( it->path().filename().string() ) )
						{
							std::string path =  m_FolderPath + "\\" + it->path().filename().string();

							Ref<File>& scFile = Ref<File>::Create();

							scFile->Init( it->path().filename().string(), path, FileExtensionType::SHADER );

							FileCollection::AddFileToCollection( scFile );
						}
					}

				}
			}
		}
		ImGui::End();

		if( ImGui::Begin( "AssetDebuger" ) )
		{
			ImGui::Text( "File Collection Size: %i", FileCollection::GetCollectionSize() );
		}
		ImGui::End();
	}

	bool AssetPanel::CheckHasAsset( std::string name, std::string filepath, std::string folder )
	{
		namespace fs = std::filesystem;

		for( fs::directory_iterator it( folder ); it != fs::directory_iterator(); ++it )
		{
			if( it->path().filename() == name && std::find( m_Assets.begin(), m_Assets.end(), name ) == m_Assets.end() && it->path().filename() != name + ".asset" )
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
		return true;
	}

	bool AssetPanel::OnKeyPressedEvent( KeyPressedEvent& e )
	{
		return true;
	}

}