#include <Saturn.h>

#include <Saturn/GameBase/GameObject.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"

#ifndef SPARKY_SANDBOX
#define SPARKY_SANDBOX
#include <Saturn/Core/Serialisation/Serialiser.h>
#endif // SPARKY_SANDBOX

class Sandbox : public Saturn::Application
{
public:
	Sandbox()
	{
	}

	~Sandbox()
	{

	}

};

Saturn::Application* Saturn::CreateApplication()
{
	return new Sandbox();
}