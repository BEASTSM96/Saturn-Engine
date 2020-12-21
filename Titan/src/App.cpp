#include <Saturn.h>
#include <Saturn/EntryPoint.h>

#include <Saturn/ImGui/ImGuiLayer.h>

class EditorApplication : public Saturn::Application
{
public:
	EditorApplication( const Saturn::ApplicationProps& props ) : Application( props )
	{
	}
};

Saturn::Application* Saturn::CreateApplication()
{
	return new EditorApplication( { "SaturnEditor", 1600, 900 } );
}
