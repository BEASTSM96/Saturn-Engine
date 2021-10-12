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
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#define APP_BIND_EVENT_FN(_) std::bind(&Application::_, this, std::placeholders::_1)


namespace Saturn {

	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n\0";


	void Application::Run()
	{
		Window::Get();
		Window::Get().SetEventCallback( APP_BIND_EVENT_FN( OnEvent ) );

		Renderer::Get();

		// build and compile our shader program
  // ------------------------------------
  // vertex shader
		unsigned int vertexShader = glCreateShader( GL_VERTEX_SHADER );
		glShaderSource( vertexShader, 1, &vertexShaderSource, NULL );
		glCompileShader( vertexShader );
		// check for shader compile errors
		int success;
		char infoLog[ 512 ];
		glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &success );
		if( !success )
		{
			glGetShaderInfoLog( vertexShader, 512, NULL, infoLog );
			//std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// fragment shader
		unsigned int fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		glShaderSource( fragmentShader, 1, &fragmentShaderSource, NULL );
		glCompileShader( fragmentShader );
		// check for shader compile errors
		glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &success );
		if( !success )
		{
			glGetShaderInfoLog( fragmentShader, 512, NULL, infoLog );
			//std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// link shaders
		unsigned int shaderProgram = glCreateProgram();
		glAttachShader( shaderProgram, vertexShader );
		glAttachShader( shaderProgram, fragmentShader );
		glLinkProgram( shaderProgram );
		// check for linking errors
		glGetProgramiv( shaderProgram, GL_LINK_STATUS, &success );
		if( !success )
		{
			glGetProgramInfoLog( shaderProgram, 512, NULL, infoLog );
			//std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader( vertexShader );
		glDeleteShader( fragmentShader );

		// set up vertex data (and buffer(s)) and configure vertex attributes
		// ------------------------------------------------------------------
		float vertices[] ={
			 0.5f,  0.5f, 0.0f,  // top right
			 0.5f, -0.5f, 0.0f,  // bottom right
			-0.5f, -0.5f, 0.0f,  // bottom left
			-0.5f,  0.5f, 0.0f   // top left 
		};
		unsigned int indices[] ={  // note that we start from 0!
			0, 1, 3,  // first Triangle
			1, 2, 3   // second Triangle
		};
		unsigned int VBO, VAO, EBO;
		glGenVertexArrays( 1, &VAO );
		glGenBuffers( 1, &VBO );
		glGenBuffers( 1, &EBO );
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray( VAO );

		glBindBuffer( GL_ARRAY_BUFFER, VBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), ( void* )0 );
		glEnableVertexAttribArray( 0 );

		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		glBindBuffer( GL_ARRAY_BUFFER, 0 );

		// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		glBindVertexArray( 0 );


		// uncomment this call to draw in wireframe polygons.
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		while( m_Running )
		{
			Window::Get().Render();

			Renderer::Get().RendererCamera().OnUpdate( m_Timestep );

			//Renderer::Get().Clear();

			glUseProgram( shaderProgram );
			glBindVertexArray( VAO ); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
			//glDrawArrays(GL_TRIANGLES, 0, 6);
			glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
			// glBindVertexArray(0); // no need to unbind it every time 

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

	std::pair< std::string, std::string > Application::OpenFile( const char* filter ) const
	{
	#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[ 260 ] ={ 0 };       // if using TCHAR macros

										// Initialize OPENFILENAME
		ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = glfwGetWin32Window( ( GLFWwindow* )Window::Get().NativeWindow() );
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof( szFile );
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if( GetOpenFileNameA( &ofn ) == TRUE )
		{
			return { ofn.lpstrFile,  ofn.lpstrFilter };
		}
		return { std::string(), std::string() };
	#endif

	#ifdef  SAT_PLATFORM_LINUX
		return { std::string(), std::string() };
	#endif

		return  { std::string(), std::string() };
	}

	std::pair< std::string, std::string > Application::SaveFile( const char* f ) const
	{
		return  { std::string(), std::string() };
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
