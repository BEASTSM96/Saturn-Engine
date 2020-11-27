#include <Saturn.h>
#include <Saturn/EntryPoint.h>

#include <Saturn/ImGui/ImGuiLayer.h>

class EditorApplication : public Saturn::Application
{
public:
	EditorApplication(const Saturn::ApplicationProps& props) : Application(props), m_EditorLayer(nullptr)
	{
		m_EditorLayer = nullptr;
	}

	virtual void OnInit() override
	{
		m_EditorLayer = (Saturn::EditorLayer*)PushLayer(new Saturn::EditorLayer());
	}

	virtual void OnShutdownSave() override 
	{
		Saturn::Serialiser serialiser(m_EditorLayer->GetEditorScene());
		serialiser.Serialise("assets\\testtemp.sc");
	}
private:
	Saturn::EditorLayer* m_EditorLayer;
};

Saturn::Application* Saturn::CreateApplication()
{
	return new EditorApplication({ "SaturnEditor", 1600, 900 });
}
