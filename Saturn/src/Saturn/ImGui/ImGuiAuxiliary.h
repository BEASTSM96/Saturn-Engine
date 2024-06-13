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

#pragma once

#include "Saturn/Vulkan/Image2D.h"
#include "Saturn/Vulkan/Texture.h"

#include "Saturn/Asset/AssetManager.h"

#include <string>
#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_internal.h>	

namespace Saturn::Auxiliary {
	
	extern bool DrawVec3Control( const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f );
	extern bool DrawVec2Control( const std::string& label, glm::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f );

	extern bool DrawColorVec3Control( const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f );

	extern bool DrawFloatControl( const std::string& label, float& values, float min = 0.0f, float max = 500.0f, float columnWidth = 125.0f );

	extern bool DrawIntControl( const std::string& label, int& values, float columnWidth = 125.0f );
	
	extern bool DrawBoolControl( const std::string& label, bool& value, float columnWidth = 125.0f );
	extern bool DrawDisabledBoolControl( const std::string& label, bool& value, float columnWidth = 125.0f );
	
	extern void DrawDisabledButton( const std::string& label );

	extern bool DrawOverlay( const std::string& label, ImVec2 Pos );
	extern bool DrawOverlay( const std::string& label );
	extern void EndOverlay();

	extern void Image( Ref< Image2D > Image, const ImVec2& Size, const ImVec2& UV0 = ImVec2( 0, 1 ), const ImVec2& UV1 = ImVec2( 1, 0 ), const ImVec4& TintColor = ImVec4( 1, 1, 1, 1 ), const ImVec4& BorderColor = ImVec4( 0, 0, 0, 0 ) );
	
	extern void Image( Ref< Texture2D > Image, const ImVec2& Size, const ImVec2& UV0 = ImVec2( 0, 1 ), const ImVec2& UV1 = ImVec2( 1, 0 ), const ImVec4& TintColor = ImVec4( 1, 1, 1, 1 ), const ImVec4& BorderColor = ImVec4( 0, 0, 0, 0 ) );
	
	extern void Image( Ref< Texture2D > Image, uint32_t Mip, const ImVec2& Size, const ImVec2& UV0 = ImVec2( 0, 1 ), const ImVec2& UV1 = ImVec2( 1, 0 ), const ImVec4& TintColor = ImVec4( 1, 1, 1, 1 ), const ImVec4& BorderColor = ImVec4( 0, 0, 0, 0 ) );

	extern void Image( Ref<Image2D> Image, uint32_t ImageLayer, const ImVec2& Size, const ImVec2& UV0 = ImVec2( 0, 1), const ImVec2& UV1 = ImVec2( 1, 0 ), const ImVec4& TintColor = ImVec4( 1, 1, 1, 1 ), const ImVec4& BorderColor = ImVec4( 0, 0, 0, 0 ) );

	extern bool ImageButton( Ref< Image2D > Image, const ImVec2& Size, const ImVec2& UV0 = ImVec2( 0, 1 ), const ImVec2& UV1 = ImVec2( 1, 0 ), int FramePadding = -1, const ImVec4& BackgroundColor = ImVec4( 0, 0, 0, 0 ), const ImVec4& TintColor = ImVec4( 1, 1, 1, 1 ) );
	
	extern bool ImageButton( Ref< Texture2D > Image, const ImVec2& Size, const ImVec2& UV0 = ImVec2( 0, 1 ), const ImVec2& UV1 = ImVec2( 1, 0 ), int FramePadding = -1, const ImVec4& BackgroundColor = ImVec4( 0, 0, 0, 0 ), const ImVec4& TintColor = ImVec4( 1, 1, 1, 1 ) );

	extern bool TreeNode( const std::string& label, bool open = true );
	extern void EndTreeNode();

	extern bool ButtonRd( const char* label, const ImRect& bb, bool rounded = false );

	extern void DrawColoredRect( const ImVec2& size, const ImVec4& color );

	extern void TextEllipsis( const char* fmt, const ImVec2& rStart, const ImVec2& rEnd, ... );
	extern void TextEllipsisV( const char* fmt, const ImVec2& rStart, const ImVec2& rEnd, va_list args );
	extern void TextEllipsisEx( const char* fmt, va_list args );

	inline bool DrawAssetFinder( AssetType type, bool* rOpen, AssetID& rOut )
	{
		bool Modified = false;

		if( *rOpen == true ) 
		{
			ImGui::OpenPopup( "AssetFinderPopup" );
			*rOpen = false;
		}

		ImGui::SetNextWindowSize( { 250.0f, 0.0f } );
		if( ImGui::BeginPopup( "AssetFinderPopup", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize ) )
		{
			bool PopupModified = false;

			if( ImGui::BeginListBox( "##ASSETLIST", ImVec2( -FLT_MIN, 0.0f ) ) )
			{
				// TODO: Change with editor.
				for( const auto& [assetID, rAsset] : AssetManager::Get().GetCombinedAssetMap() )
				{
					bool Selected = ( rOut == assetID );

					ImGui::PushID( static_cast<int>( assetID ) );

					if( rAsset->GetAssetType() == type || type == AssetType::Unknown )
					{
						if( ImGui::Selectable( rAsset->Name.c_str(), Selected ) )
						{
							rOut = assetID;

							PopupModified = true;
						}
					}

					ImGui::PopID();

					if( Selected )
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndListBox();
			}

			if( PopupModified )
			{
				ImGui::CloseCurrentPopup();

				Modified = true;
				*rOpen = false;
			}

			ImGui::EndPopup();
		}

		return Modified;
	}

	template<typename Function>
	inline bool DrawAssetFinder( AssetType allowedTypes, bool* rOpen, Function&& rrFunction )
	{
		bool Modified = false;

		if( *rOpen == true && !ImGui::IsPopupOpen( "AssetFinderPopup" ) )
		{
			ImGui::OpenPopup( "AssetFinderPopup" );
			*rOpen = true;
		}

		ImGui::SetNextWindowSize( { 250.0f, 0.0f } );
		if( ImGui::BeginPopup( "AssetFinderPopup", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize ) )
		{
			bool PopupModified = false;

			if( ImGui::BeginListBox( "##ASSETLIST", ImVec2( -FLT_MIN, 0.0f ) ) )
			{
				for( const auto& [assetID, rAsset] : AssetManager::Get().GetAssetRegistry()->GetAssetMap() )
				{
					bool Selected = ( false );

					ImGui::PushID( static_cast< int >( assetID ) );

					if( rAsset->GetAssetType() == allowedTypes || allowedTypes == AssetType::Unknown )
					{
						if( ImGui::Selectable( rAsset->Name.c_str(), Selected ) )
						{
							rrFunction( rAsset );

							PopupModified = true;
						}
					}

					ImGui::PopID();

					if( Selected )
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndListBox();
			}

			if( PopupModified )
			{
				ImGui::CloseCurrentPopup();

				Modified = true;
				*rOpen = false;
			}

			ImGui::EndPopup();
		}

		return Modified;
	}

	template<typename Ty, typename Fn>
	bool DrawAssetDragDropTarget( const char* label, const char* assetName, UUID& returnID, Fn&& function ) 
	{
		if( returnID == 0 )
			return false;

		bool changed = false;

		ImGui::Text( label );

		std::string ButtonName = "";

		if( auto asset = AssetManager::Get().FindAsset( returnID ) )
			ButtonName = asset->GetName();
		else
			ButtonName = "Unknown";

		ImGui::Button( ButtonName.c_str() );

		if( ImGui::BeginDragDropTarget() )
		{
			auto data = ImGui::AcceptDragDropPayload( "asset_payload" );

			if( data )
			{
				const wchar_t* path = ( const wchar_t* ) data->Data;

				Ref<Asset> asset = AssetManager::Get().GetAssetAs<Asset>( AssetManager::Get().PathToID( path ) );

				if( asset )
				{
					function( asset.As<Ty>() );
					changed = true;
				}
			}
		}

		return changed;
	}
}