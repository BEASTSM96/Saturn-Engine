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
#include "UITools.h"

#include "Saturn/Asset/AssetRegistry.h"

#include <backends/imgui_impl_vulkan.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>

namespace Saturn {

	bool DrawVec2Control( const std::string& label, glm::vec2& values, float resetValue /*= 0.0f*/, float columnWidth /*= 100.0f */ )
	{
		bool modified = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[ 0 ];

		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2 );
		ImGui::SetColumnWidth( 0, columnWidth );
		ImGui::Text( label.c_str() );
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 } );

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize ={ lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f } );
		ImGui::PushFont( boldFont );
		if( ( ImGui::Button( "X", buttonSize ) ) )
		{
			values.x = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor( 3 );

		ImGui::SameLine();
		modified |= ImGui::DragFloat( "##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f" );
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f } );
		ImGui::PushFont( boldFont );
		if( ( ImGui::Button( "Y", buttonSize ) ) )
		{
			values.y = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor( 3 );

		ImGui::SameLine();
		modified |= ImGui::DragFloat( "##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f" );
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PopStyleVar();

		ImGui::Columns( 1 );

		ImGui::PopID();

		return modified;
	}

	bool DrawVec3Control( const std::string& label, glm::vec3& values, float resetValue /*= 0.0f*/, float columnWidth /*= 100.0f */ )
	{
		bool modified = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[ 0 ];

		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2 );
		ImGui::SetColumnWidth( 0, columnWidth );
		ImGui::Text( label.c_str() );
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 } );

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize ={ lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f } );
		ImGui::PushFont( boldFont );
		if( ( ImGui::Button( "X", buttonSize ) ) )
		{
			values.x = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor( 3 );

		ImGui::SameLine();
		modified |= ImGui::DragFloat( "##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f" );
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f } );
		ImGui::PushFont( boldFont );
		if( ( ImGui::Button( "Y", buttonSize ) ) )
		{
			values.y = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor( 3 );

		ImGui::SameLine();
		modified |= ImGui::DragFloat( "##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f" );
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f } );
		ImGui::PushFont( boldFont );
		if( ( ImGui::Button( "Z", buttonSize ) ) )
		{
			values.z = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor( 3 );

		ImGui::SameLine();
		modified |= ImGui::DragFloat( "##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f" );
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns( 1 );

		ImGui::PopID();

		return modified;
	}

	bool DrawColorVec3Control( const std::string& label, glm::vec3& values, float resetValue /*= 0.0f*/, float columnWidth /*= 100.0f */ )
	{
		bool modified = false;

		ImGui::PushID( label.c_str() );

		ImGui::Text( label.c_str() );

		ImGui::SameLine();

		modified |= ImGui::ColorEdit3( "##picker", glm::value_ptr( values ) );

		ImGui::PopID();

		return modified;
	}

	bool DrawFloatControl( const std::string& label, float& values, float min, float max, float columnWidth /*= 125.0f */ )
	{
		bool modified = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[ 0 ];

		ImGui::PushID( label.c_str() );

		ImGui::Text( label.c_str() );

		ImGui::NextColumn();

		ImGui::SameLine();

		modified |= ImGui::DragFloat( "##floatx", &values, 0.1f, max, min, "%.2f" );

		ImGui::PopID();

		return modified;
	}

	bool DrawIntControl( const std::string& label, int& values, float columnWidth /*= 125.0f */ )
	{
		bool modified = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[ 0 ];

		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2 );
		ImGui::SetColumnWidth( 0, columnWidth );

		ImGui::Text( label.c_str() );

		ImGui::NextColumn();

		ImGui::SameLine();

		ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );

		modified |= ImGui::DragInt( "##intx", &values, 0.0f, 5000, 0.0f, "%.2f" );

		ImGui::PopItemWidth();

		ImGui::PopID();

		return modified;
	}

	bool DrawBoolControl( const std::string& label, bool& value, float columnWidth /*= 125.0f */ )
	{
		bool modified = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[ 0 ];

		ImGui::PushID( label.c_str() );

		//ImGui::Columns( 2 );
		//ImGui::SetColumnWidth( 0, columnWidth );

		ImGui::Text( label.c_str() );

		//ImGui::NextColumn();

		ImGui::SameLine();

		ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );

		modified |= ImGui::Checkbox( "##boolean", &value );

		ImGui::PopItemWidth();

		ImGui::PopID();

		return modified;
	}

	bool DrawOverlay( const std::string& label )
	{
		ImGui::PushID( label.c_str() );

		bool SkipItem = ImGui::Begin( label.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus );
		
		return SkipItem;
	}

	bool DrawOverlay( const std::string& label, ImVec2 Pos )
	{
		ImGui::PushID( label.c_str() );

		ImGui::SetNextWindowPos( Pos );

		bool SkipItem = ImGui::Begin( label.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus );

		return SkipItem;
	}

	void EndOverlay()
	{
		ImGui::End();
		ImGui::PopID();
	}

	void Image( Ref<Image2D> Image, const ImVec2& Size, const ImVec2& UV0, const ImVec2& UV1, const ImVec4& TintColor, const ImVec4& BorderColor )
	{
		void* TextureID = ImGui_ImplVulkan_AddTexture( Image->GetSampler(), Image->GetImageView(), Image->GetDescriptorInfo().imageLayout );

		ImGui::Image( TextureID, Size, UV0, UV1, TintColor, BorderColor );
	}

	void Image( Ref<Image2D> Image, uint32_t ImageLayer, const ImVec2& Size, const ImVec2& UV0, const ImVec2& UV1, const ImVec4& TintColor, const ImVec4& BorderColor )
	{
		void* TextureID = ImGui_ImplVulkan_AddTexture( Image->GetSampler(), Image->GetImageView( ImageLayer ), Image->GetDescriptorInfo().imageLayout );

		ImGui::Image( TextureID, Size, UV0, UV1, TintColor, BorderColor );
	}

	void Image( Ref<Texture2D> Image, const ImVec2& Size, const ImVec2& UV0, const ImVec2& UV1, const ImVec4& TintColor, const ImVec4& BorderColor )
	{
		void* TextureID = ImGui_ImplVulkan_AddTexture( Image->GetSampler(), Image->GetImageView(), Image->GetDescriptorInfo().imageLayout );

		ImGui::Image( TextureID, Size, UV0, UV1, TintColor, BorderColor );
	}

	void Image( Ref<Texture2D> Image, uint32_t Mip, const ImVec2& Size, const ImVec2& UV0, const ImVec2& UV1, const ImVec4& TintColor, const ImVec4& BorderColor )
	{
		void* TextureID = ImGui_ImplVulkan_AddTexture( Image->GetSampler(), Image->GetOrCreateMipImageView( Mip ), Image->GetDescriptorInfo().imageLayout );

		ImGui::Image( TextureID, Size, UV0, UV1, TintColor, BorderColor );
	}

	bool ImageButton( Ref< Image2D > Image, const ImVec2& Size, const ImVec2& UV0, const ImVec2& UV1, int FramePadding, const ImVec4& BackgroundColor, const ImVec4& TintColor )
	{
		void* TextureID = ImGui_ImplVulkan_AddTexture( Image->GetSampler(), Image->GetImageView(), Image->GetDescriptorInfo().imageLayout );

		return ImGui::ImageButton( TextureID, Size, UV0, UV1, FramePadding, BackgroundColor, TintColor );
	}

	bool ImageButton( Ref< Texture2D > Image, const ImVec2& Size, const ImVec2& UV0, const ImVec2& UV1, int FramePadding, const ImVec4& BackgroundColor, const ImVec4& TintColor )
	{
		void* TextureID = ImGui_ImplVulkan_AddTexture( Image->GetSampler(), Image->GetImageView(), Image->GetDescriptorInfo().imageLayout );

		return ImGui::ImageButton( TextureID, Size, UV0, UV1, FramePadding, BackgroundColor, TintColor );
	}

	bool TreeNode( const std::string& label, bool open /*= true */ )
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		if( open )
			flags |= ImGuiTreeNodeFlags_DefaultOpen;

		return ImGui::TreeNodeEx( label.c_str(), flags );
	}

	void EndTreeNode()
	{
		ImGui::TreePop();
	}

	static inline ImVec2 operator+( const ImVec2& lhs, const ImVec2& rhs ) { return ImVec2( lhs.x + rhs.x, lhs.y + rhs.y ); }
	static inline ImVec2 operator-( const ImVec2& lhs, const ImVec2& rhs ) { return ImVec2( lhs.x - rhs.x, lhs.y - rhs.y ); }

	bool ButtonRd( const char* label, const ImRect& bb, bool rounded /*= false */ )
	{
		using namespace ImGui;

		ImGuiButtonFlags flags = ImGuiButtonFlags_None;

		ImGuiWindow* window = GetCurrentWindow();
		if( window->SkipItems )
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID( label );
		const ImVec2 label_size = CalcTextSize( label, NULL, true );

		ImVec2 pos = window->DC.CursorPos;
		if( ( flags & ImGuiButtonFlags_AlignTextBaseLine ) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset ) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
			pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;

		ImVec2 size = CalcItemSize( bb.Min, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f );

		ItemSize( size, style.FramePadding.y );
		if( !ItemAdd( bb, id ) )
			return false;

		if( g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat )
			flags |= ImGuiButtonFlags_Repeat;

		bool hovered, held;
		bool pressed = ButtonBehavior( bb, id, &hovered, &held, flags );

		// Render
		const ImU32 col = GetColorU32( ( held && hovered ) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button );
		RenderNavHighlight( bb, id );

		// RenderFrame
		{
			if( rounded )
			{
				window->DrawList->AddRect( bb.Min, bb.Max, col, 5.0f, ImDrawFlags_RoundCornersAll );
				const float BORDER_SIZE = g.Style.FrameBorderSize;
			}
			else
				RenderFrame( bb.Min, bb.Max, col, true, style.FrameRounding );
		}

		if( g.LogEnabled )
			LogSetNextTextDecoration( "[", "]" );
		RenderTextClipped( bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb );

		// Automatically close popups
		//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
		//    CloseCurrentPopup();

		IMGUI_TEST_ENGINE_ITEM_INFO( id, label, g.LastItemData.StatusFlags );
		return pressed;
	}

	void DrawColoredRect( const ImVec2& size, const ImVec4& color )
	{
		// Context
		ImGuiContext& g = *GImGui;

		const float DefualtSize = ImGui::GetFrameHeight();
		const ImVec2 RealSize( size.x == 0.0f ? DefualtSize : size.x, size.y == 0.0f ? DefualtSize : size.y );

		const ImRect BoundingBox( ImGui::GetCursorPos(), ImGui::GetCursorPos() + size );

		ImGui::ItemSize( BoundingBox, (size.y >= DefualtSize) ? g.Style.FramePadding.y : 0.0f );
		ImGui::ItemAdd( BoundingBox, ImGui::GetID( "colored_rect" ) );

		float grid_step = ImMin( size.x, size.y ) / 2.99f;
		float rounding = ImMin( g.Style.FrameRounding, grid_step * 0.5f );

		// Draw the rect

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		ImVec4 ColorNoAlpha = ImVec4( color.x, color.y, color.z, 1.0f );
		pDrawList->AddRectFilled( BoundingBox.Min, BoundingBox.Max, ImGui::GetColorU32( ColorNoAlpha ), rounding );
	}
}