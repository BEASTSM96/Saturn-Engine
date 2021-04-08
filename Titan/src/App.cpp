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

		//TODO: Make a better icon as it does not fit
		//m_Window->SetWindowImage( "assets/.github/i/sat/SaturnLogov1.png" );
	}

	Saturn::EditorLayer& GetEditorLayer() { return *m_EditorLayer; }
	const Saturn::EditorLayer& GetEditorLayer() const { return *m_EditorLayer; }

private:
	Saturn::EditorLayer* m_EditorLayer;
};

Saturn::Application* Saturn::CreateApplication()
{
	return new EditorApplication( { "SaturnEditor", 1600, 900 } );
}
