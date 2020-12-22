/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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
#include "Asset.h"

#include "AssetCollection.h"

namespace Saturn {

	Ref<Saturn::Asset> Asset::Load( std::string path, std::string RootDir, AssetCollection* assetCollection )
	{

	}

	Ref<Asset> Asset::GetParent() const
	{
		if (m_Path == "/")
			return nullptr;

		SAT_CORE_ASSERT(m_AssetCollection, "Asset is not owned by a AssetCollector");

		auto lastSlash = m_Path.find_first_of('/');
		return m_AssetCollection->GetAsset(m_Path.substr(0, lastSlash +1 ));

	}

	Saturn::FolderAsset* Asset::GetFolderAsset()
	{
	
		if (m_Type != AssetType::Folder)
		{
			return nullptr;
		}

		return (FolderAsset*)this;
	}

}