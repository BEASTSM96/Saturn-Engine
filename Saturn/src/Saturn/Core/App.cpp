/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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
#include "App.h"

#include "Window.h"

#include "Saturn/OpenGL/Shader.h"
#include "Saturn/OpenGL/Renderer.h"
#include "Saturn/OpenGL/Texture.h"
#include "Saturn/Core/Renderer/EditorCamera.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#define APP_BIND_EVENT_FN(_) std::bind(&Application::_, this, std::placeholders::_1)


namespace Saturn {

	void Application::Run()
	{
		Window::Get();
		Window::Get().SetEventCallback( APP_BIND_EVENT_FN( OnEvent ) );

		Renderer::Get();

		while( m_Running )
		{
			Window::Get().Render();

			Renderer::Get().RendererCamera().OnUpdate( m_Timestep );

			Renderer::Get().GeoPass();
			Renderer::Get().CompositePass();

			Window::Get().OnUpdate();

			float time = ( float )glfwGetTime(); //Platform::GetTime();

			m_Timestep = time - m_LastFrameTime;

			m_LastFrameTime = time;
		}
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent( Event& e )
	{
		EventDispatcher dispatcher( e );

		dispatcher.Dispatch< WindowResizeEvent >( APP_BIND_EVENT_FN( OnWindowResize ) );

		Renderer::Get().OnEvent( e );
	}

	bool Application::OnWindowResize( WindowResizeEvent& e )
	{
		int width = e.Width(), height = e.Height();
		if( width && height == 0 )
			return false;

		glViewport( 0, 0, width, height );

		Renderer::Get().Resize( width, height );
	}
}
