#include <Saturn.h>
#include <Saturn/EntryPoint.h>

#include "EditorLayer.h"

#include <Saturn/GameFramework/HotReload.h>
#include <Saturn/Core/Modules/ModuleManager.h>
#include <Saturn/Scene/SceneManager.h>
#include <Saturn/Core/Modules/Module.h>
#include <Saturn/Scene/Scene.h>

class EditorApplication : public Saturn::Application
{
public:
	EditorApplication( const Saturn::ApplicationProps& props ) : Application( props )
	{
		m_EditorLayer = new Saturn::EditorLayer();
		PushOverlay( m_EditorLayer );
	}
private:
	Saturn::EditorLayer* m_EditorLayer;
};

Saturn::Application* Saturn::CreateApplication()
{
	return new EditorApplication( { "SaturnEditor", 1600, 900 } );
}
