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

#pragma once

#include "Saturn/Core/Base.h"

namespace Saturn {

	enum class AssetType
	{
		None = 0, Folder
	};

	class FolderAsset;
	class AssetCollection;

	class Asset : public RefCounted
	{
	public:
		Asset() = default;
		virtual ~Asset() = default;

		static Ref<Asset> Load(std::string path, std::string RootDir, AssetCollection* assetCollection);

		Ref<Asset> GetParent() const;

		std::string& GetName() { return m_Name; }
		std::string& GetPath() { return m_Path; }

		const std::string& GetName() const { return m_Name; }
		const std::string& GetPath() const { return m_Path; }

		UUID& GetUUID() { return m_UUID; }
		const UUID& GetUUID() const { return m_UUID; }
		void SetUUID();

		bool IsFolder() const { m_Type == AssetType::Folder; }

		FolderAsset* GetFolderAsset();
	protected:
		AssetType m_Type = AssetType::None;

		std::string m_Name;
		std::string m_Path;
		UUID m_UUID;

		AssetCollection* m_AssetCollection = nullptr;

		friend class AssetCollection;

	};
}