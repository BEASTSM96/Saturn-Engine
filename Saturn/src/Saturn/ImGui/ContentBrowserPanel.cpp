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
#include "ContentBrowserPanel.h"

#include "UITools.h"

#include <imgui_internal.h>

namespace Saturn {
	
	static const std::filesystem::path s_pAssetsDirectory = "assets";
	
	ContentBrowserPanel::ContentBrowserPanel()
		: Panel( "Content Browser Panel" ), m_CurrentPath( s_pAssetsDirectory ), m_FirstFolder( s_pAssetsDirectory )
	{
		m_DirectoryIcon = Ref<Texture2D>::Create( "assets/textures/editor/DirectoryIcon.png", AddressingMode::Repeat );
		m_FileIcon      = Ref<Texture2D>::Create( "assets/textures/editor/FileIcon.png", AddressingMode::Repeat );
	}
	
	void ContentBrowserPanel::Draw()
	{
		ImGui::Begin( "Content Browser" );

		ImGui::BeginChild( "##CB_TopBar", ImVec2( 0, 30 ) );

		if( m_CurrentPath != s_pAssetsDirectory )
		{
			if( ImGui::Button( "<-" ) )
			{
				m_CurrentPath = m_CurrentPath.parent_path();
			}
		}
		else
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
			ImGui::Button( "<-" );
			ImGui::PopStyleVar( 1 );
			ImGui::PopItemFlag();
		}
		
		ImGui::SameLine();

		if( std::filesystem::exists( m_FirstFolder ) )
		{
			if( ImGui::Button( "->" ) )
			{
				m_CurrentPath /= std::filesystem::relative( m_FirstFolder, s_pAssetsDirectory );
			}
		}
		else
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.5f );
			ImGui::Button( "->" );
			ImGui::PopStyleVar( 1 );
			ImGui::PopItemFlag();
		}
		
		ImGui::SameLine();
		
		ImGui::Separator();

		ImGui::SameLine();
		
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.3f, 0.3f, 0.3f, 0.35f ) );

		int i = 0;
		// Foreach folder in the current path draw a small button then draw a forward slash keep doing this until the last path.
		for( auto& pFolder : m_CurrentPath )
		{
			i++;

			if( pFolder != "assets" )
			{
				ImGui::Text( "/" );
			}
			
			ImGui::SameLine();

			float size = strlen( pFolder.string().c_str() ) + ImGui::CalcTextSize( pFolder.string().c_str() ).x;

			if( ImGui::Selectable( pFolder.string().c_str(), false, 0, ImVec2( size, 22.0f ) ) )
			{				
			}
			ImGui::SameLine();
		}

		ImGui::PopStyleColor( 2 );
		
		ImGui::EndChild();

		ImGui::Separator();

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.3f, 0.3f, 0.3f, 0.35f ) );

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		
		int columnCount = (int)panelWidth / cellSize;

		if( columnCount < 1 )
			columnCount = 1;

		ImGui::Columns( columnCount, 0, false );

		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
			RenderEntry( rEntry, { thumbnailSize, thumbnailSize }, true );
		
		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
			RenderEntry( rEntry, { thumbnailSize, thumbnailSize }, false );

		// Get the first folder in the current directory.
		m_FirstFolder = "";
		for( auto& rEntry : std::filesystem::directory_iterator( m_CurrentPath ) )
		{
			if( rEntry.is_directory() )
			{
				m_FirstFolder = rEntry.path();
				break;
			}
		}
		
		ImGui::Columns( 1 );
		
		ImGui::SliderFloat( "Thumbnail Size", &thumbnailSize, 64, 512 );
		ImGui::SliderFloat( "Padding", &padding, 0, 32 );

		ImGui::PopStyleColor( 2 );
		ImGui::End();
	}

	void ContentBrowserPanel::RenderEntry( const std::filesystem::directory_entry& rEntry, ImVec2 ThumbnailSize, bool excludeFiles /*= true */ )
	{
		if( !rEntry.is_directory() && excludeFiles || rEntry.is_directory() && !excludeFiles )
			return;

		std::string path = std::filesystem::relative( rEntry.path(), s_pAssetsDirectory ).string();
		std::string filename = rEntry.path().filename().string();

		std::filesystem::relative( rEntry.path(), s_pAssetsDirectory );

		Ref<Texture2D> Icon = excludeFiles ? m_DirectoryIcon : m_FileIcon;

		ImageButton( Icon, ThumbnailSize, { 0, 1 }, { 1, 0 } );

		if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
		{
			if( rEntry.is_directory() )
				m_CurrentPath /= rEntry.path().filename();
			
			OnDirectorySelected( m_CurrentPath, rEntry.is_directory() );
		}

		ImGui::TextWrapped( filename.c_str() );

		ImGui::NextColumn();
	}

	void ContentBrowserPanel::OnDirectorySelected( std::filesystem::path& rPath, bool IsFile /*= false */ )
	{
		// TODO:
	}

}