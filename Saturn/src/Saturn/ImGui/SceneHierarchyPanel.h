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
#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Entity.h"
#include "Saturn/Core/Timestep.h"

#include <functional>

namespace Saturn {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel( const Ref<Scene>& scene );
		void SetContext( const Ref<Scene>& scene );
		void SetSelected( Entity entity );
		void SetSelectionChangedCallback( const std::function<void( Entity )>& func ) { m_SelectionChangedCallback = func; }

		void OnImGuiRender( void );
		void OnUpdate( Timestep ts );

		Entity& GetSelectionContext() { return m_SelectionContext; }


	private:
		void DrawEntityNode( Entity entity );
		void DrawEntityComponents( Entity entity );
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		std::string m_NCSTag = "Enter Class Name";

		std::function<void( Entity )> m_SelectionChangedCallback;

		friend class EditorLayer;
	};


}