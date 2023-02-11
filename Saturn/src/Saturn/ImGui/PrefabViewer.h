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

#pragma once

#include "AssetViewer.h"
#include "Saturn/Asset/Prefab.h"

#include "SceneHierarchyPanel.h"

namespace Saturn {

	class PrefabViewer : public AssetViewer
	{
	public:
		static PrefabViewer& Get() { return *SingletonStorage::Get().GetOrCreateSingleton<PrefabViewer>(); };
	public:
		PrefabViewer();
		~PrefabViewer();

		virtual void Draw() override;

		void AddPrefab( Ref<Asset>& rAsset );

	private:
		void DrawInternal( Ref<Prefab>& rPrefab );
	private:
		std::vector<Ref<Prefab>> m_Prefabs;
		std::unordered_map<UUID, bool> m_OpenPrefabs;

		Ref<SceneHierarchyPanel> m_SceneHierarchyPanel;
	};
}