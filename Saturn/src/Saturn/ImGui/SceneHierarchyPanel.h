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

		Entity& GetSelectionContext() { return m_SelectionContext; }

		void Draw();

		void SetIsPrefabScene( bool value ) { m_IsPrefabScene = value; }

	protected:

		void DrawComponents( Entity entity );
		void DrawEntityNode( Entity entity );
		void DrawEntityComponents( Entity entity );
		void DrawEntities();

	private:

		bool m_IsPrefabScene = false;

		Ref<Texture2D> m_EditIcon;

		Ref<Scene> m_Context;
		Entity m_SelectionContext ={};
		std::function<void( Entity )> m_SelectionChangedCallback;
	};

}