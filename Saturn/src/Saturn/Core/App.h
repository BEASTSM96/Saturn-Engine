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

#include "Base.h"

#include "Layer.h"
#include "Events.h"
#include "Input.h"

#include "Saturn/Editor/EditorLayer.h"

namespace Saturn {

	class Application
	{
	public:
		Application()  {}
		~Application() {}

		void Run();
		void Close();

		bool Running() { return m_Running; }

		Timestep& Time() { return m_Timestep; }

		std::string OpenFile( const char* pFilter ) const;
		std::string SaveFile( const char* pFilter ) const;

		EditorLayer* GetEditorLayer() { return m_EditorLayer; }

		static inline Application& Get() { return *s_Instance; }
		
	protected:

		void OnEvent( Event& e );
		bool OnWindowResize( WindowResizeEvent& e );

		void RenderImGui();

	private:
		bool m_Running = true;
		
		ImGuiLayer* m_ImGuiLayer = nullptr;
		EditorLayer* m_EditorLayer = nullptr;

		Timestep m_Timestep;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

}