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

#include "sppch.h"
#include "ContentBrowserThumbnailGenerator.h"

#include "Saturn/Asset/Asset.h"

namespace Saturn {

	static Ref<Texture2D> s_NoIcon;
	static Ref<Texture2D> s_FolderIcon;

	void ContentBrowserThumbnailGenerator::Init()
	{
		//s_NoIcon = Ref<Texture2D>::Create( "content/textures/editor/NoIcon.png", AddressingMode::Repeat );
		s_NoIcon = Ref<Texture2D>::Create( "content/textures/editor/FileIcon.png", AddressingMode::Repeat );
		s_FolderIcon = Ref<Texture2D>::Create( "content/textures/editor/DirectoryIcon.png", AddressingMode::Repeat );
	}

	void ContentBrowserThumbnailGenerator::Terminate()
	{
		s_NoIcon = nullptr;
		s_FolderIcon = nullptr;
	}

	Ref<Texture2D> ContentBrowserThumbnailGenerator::GetDefault( int Identifier )
	{
		return Identifier == 0 ? s_FolderIcon : s_NoIcon;
	}

	Ref<Texture2D> ContentBrowserThumbnailGenerator::GetFor( const Ref<Asset>& rAsset )
	{
		return s_NoIcon;
	}
}