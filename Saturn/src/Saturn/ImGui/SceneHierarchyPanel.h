/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Scene.h"

#include "Panel/Panel.h"
#include "Saturn/Vulkan/Texture.h"

#include <functional>

namespace Saturn {

	class SceneHierarchyPanel : public Panel
	{
	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void SetContext( const Ref<Scene>& scene );
		void SetSelected( Entity entity );
		void SetSelectionChangedCallback( const std::function<void( Entity )>& func ) { m_SelectionChangedCallback = func; }

		std::vector<Entity>& GetSelectionContexts() { return m_SelectionContexts; }
		const std::vector<Entity>& GetSelectionContexts() const { return m_SelectionContexts; }
		
		Entity& GetSelectionContext( uint32_t index = 0 ) 
		{
			return m_SelectionContexts[ index ]; 
		}
		
		const Entity& GetSelectionContext( uint32_t index = 0 ) const 
		{
			return m_SelectionContexts[ index ]; 
		}

		void Draw();

		void SetIsPrefabScene( bool value ) { m_IsPrefabScene = value; }

		void AddID( UUID ID ) { m_CustomID = ID; }
		void SetName( const std::string& rName ) { m_WindowName = rName; }

	protected:

		void DrawComponents( Entity entity );
		bool IsEntitySelected( Entity entity );
		void DrawEntityNode( Entity entity );
		void DrawEntityComponents( Entity entity );
		void DrawEntities();

		void ClearSelection();

		template<typename Ty>
		void DrawAddComponents( const char* pName, Entity entity );

	private:
		UUID m_CustomID = 0;
		std::string m_WindowName = "Scene Hierarchy";

		bool m_IsPrefabScene = false;

		Ref<Texture2D> m_EditIcon;

		bool m_OpenAssetFinderPopup = false;
		AssetID m_CurrentAssetID = 0;

		AssetType m_CurrentFinderType = AssetType::Unknown;

		bool m_IsMultiSelecting = false;

		Ref<Scene> m_Context;
		std::vector<Entity> m_SelectionContexts;
		std::function<void( Entity )> m_SelectionChangedCallback;
	};

}