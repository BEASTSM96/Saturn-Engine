#include "sppch.h"
#include "ImGuiLayer.h"
#include "Sparky/Log.h"

#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#include "Sparky/Application.h"
#include "imgui_draw.cpp"
#include "imgui_internal.h"
#include "imconfig.h"

#include "Sparky/Core/Timestep.h"

#include "ImGuiFileDialog.h"
#include "ImGuiFileDialog.cpp"
#include "ImGuiFileDialogConfig.h"

// TEMPORARY
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Sparky {

	#define IMGUI_CLASSES_SPARKY -0
	#define IMGUI_OBJECTS_SPARKY nullptr

	///////////////////////////////////////
	////////////FORALLPLATFORMS///////////
	/////////////////////////////////////

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
		#define IMGUI_CLASSES_SPARKY 0
	}

	ImGuiLayer::~ImGuiLayer()
	{
		#undef 	IMGUI_CLASSES_SPARKY 0
	}

	void ImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		#define IMGUI_OBJECTS_SPARKY 0

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
																	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
																	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
																	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;																	// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		#undef IMGUI_OBJECTS_SPARKY 0
	}

	void ImGuiLayer::OnImGuiRender()
	{
		static bool show = true;
		ImGui::ShowDemoWindow(&show);
	}


	////////////////////////////////////////////////
	////////////////IMGUIFPS///////////////////////
	//////////////////////////////////////////////


	//////////////////////////////////////////
	////////////NOTFORALLPLATFORMS///////////
	////////////////////////////////////////

	//#define SP_FPS
	ImGuiFPS::ImGuiFPS() : Layer("ImguiFPS")
	{
		#define IMGUI_CLASSES_SPARKY 1
	}

	ImGuiFPS::~ImGuiFPS()
	{
		#undef IMGUI_CLASSES_SPARKY 1
	}

	void ImGuiFPS::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		#define  IMGUI_OBJECTS_SPARKY 1

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Platform/Renderer bindings
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiFPS::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiFPS::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiFPS::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		#undef IMGUI_OBJECTS_SPARKY 1
	}

	void ImGuiFPS::OnImGuiRender()
	{

		static bool show = true;

		float LastFrameTime = 0.0f;

		float time = (float)glfwGetTime(); //Platform::GetTime();

		Timestep timestep = time - LastFrameTime;

		LastFrameTime = time;

		if (show)
		{
			ImGui::Begin("test");
			//SP_CORE_INFO("FPS {0}", timestep);
			ImGui::Text("yes fps is here ", timestep);
			ImGui::End();
		}
	}

	////////////////////////////////////////////////////////
	////////////////imguirenderstats////////////////////////
	///////////////////////////////////////////////////////

	//////////////////////////////////////////
	////////////NOTFORALLPLATFORMS///////////
	////////////////////////////////////////



	//TEMP
#include <GLFW\glfw3.h>

#include <glad\glad.h>
#include <gl\GL.h>


	ImGuiRenderStats::ImGuiRenderStats() : Layer("ImguiRenderStats")
	{
		#define IMGUI_CLASSES_SPARKY 2
	}

	ImGuiRenderStats::~ImGuiRenderStats()
	{
		#undef IMGUI_CLASSES_SPARKY 2
	}

	void ImGuiRenderStats::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		#define IMGUI_OBJECTS_SPARKY 2
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Platform/Renderer bindings
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiRenderStats::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiRenderStats::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiRenderStats::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		#undef IMGUI_OBJECTS_SPARKY 2
	}

	void ImGuiRenderStats::OnImGuiRender()
	{
		static bool show = true;
		if (show)
		{

			const char* ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
			const char* vender = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
			const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

			ImGui::Begin("RendererStats");
			ImGui::Text(ver);
			ImGui::Text(vender);
			ImGui::Text(renderer);

			ImGui::End();
		}
	}


	///////////////////////////////////////////
	///////////////IMGUITOPBAR////////////////
	/////////////////////////////////////////


	ImguiTopBar::ImguiTopBar() : Layer("TopBar")
	{
		#define IMGUI_CLASSES_SPARKY 5 //TopBar
		#define IMGUI_CLASSES_SPARKY 9 //EdLayer
	}

	ImguiTopBar::~ImguiTopBar()
	{
		#undef IMGUI_CLASSES_SPARKY 5 //TopBar
		#undef IMGUI_CLASSES_SPARKY 9 //EdLayer
	}

	void ImguiTopBar::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		#define IMGUI_OBJECTS_SPARKY 5 
		#define IMGUI_OBJECTS_SPARKY 9
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Platform/Renderer bindings
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImguiTopBar::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		#undef  IMGUI_OBJECTS_SPARKY 5 
		#undef IMGUI_OBJECTS_SPARKY 9
		ImGui::DestroyContext();
	}

	void ImguiTopBar::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImguiTopBar::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		#undef  IMGUI_OBJECTS_SPARKY 5 
		#undef IMGUI_OBJECTS_SPARKY 9
	}


	static void ShowAboutSparkyWindow()
	{
		ImGui::MenuItem("(About menu)", NULL, false, false);
		if (ImGui::MenuItem("About")) {}
		if (ImGui::MenuItem("ver :", "v 0.21")) {}
		if (ImGui::MenuItem("Build :", "dev")) {}
		if (ImGui::MenuItem("GitHub branch :", "master")) {}
	}

	static bool canValidateDialog = false;
	inline void InfosPane(std::string vFilter, igfd::UserDatas vUserDatas, bool* vCantContinue) // if vCantContinue is false, the user cant validate the dialog
	{
		ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
		ImGui::Text("Selected Filter : %s", vFilter.c_str());
		if (vUserDatas)
			ImGui::Text("UserDatas : %s", vUserDatas);
		ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);
		if (vCantContinue)
			*vCantContinue = canValidateDialog;
	}

	static void ShowSceneStuff(bool* p_open)
	{
		const char* filters = "Sparky Project files (*.sproject){.sproject}";

		igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Project File", filters, ".");

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4((float)1.0f, 1.0f, 0.0f, 0.9f));

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".h", ImVec4((float)0.0f, 1.0f, 0.0f, 0.9f));

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".sproject", ImVec4((float)0.0f, 1.0f, 0.0f, 0.9f), "[SPARKY PROJECT FILE]");

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".SPARKYFILEC", ImVec4((float)1.0f, 0.0f, 1.0f, 0.9f));

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".EDFILE", ImVec4((float)0.0f, 0.0f, 8.0f, 0.9f));

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".sparkyfilec", ImVec4((float)1.0f, 0.0f, 1.0f, 0.9f));

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".edfile", ImVec4((float)0.0f, 0.0f, 8.0f, 0.9f));

			// display
		if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey"))
		{
			// action if OK
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
			{
				std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
				std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
					// action
			}
			// close
			igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
		}
	}

	static void ShowCodeFiles(bool* p_open)
	{

		const char* filters = "Source files (*.cpp, *.h){*.cpp, *.h}";

		igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose 5 File", filters, ".", 0);

		//igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Code File", filters, ".");

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4(1.f, 1.f, 0.f, 0.9f), "[C++ FILE]");

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".h", ImVec4(0.f, 1.f, 0.f, 0.9f), "[HEADER FILE]");

		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".sproject", ImVec4(0.f, 1.f, 0.f, 0.9f), "[SPARKY PROJECT FILE]");

		// display
		if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey"))
		{
			// action if OK
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
			{
				std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
				std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
				// action
			}
			// close
			igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
		}
	}

	static void ShowDirectoryChooser(bool* p_open)
	{

		igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a Directory", ".*", ".");
		//igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose a Directory", 0, ".");

		// display
		if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey"))
		{
			// action if OK
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
			{
				std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
				std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
				// action
			}
			// close
			igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
		}
	}

	void ImguiTopBar::OnImGuiRender()
	{
		
		static bool showScene = false;
		static bool showFile = false;
		static bool showCodeFile = false;
		static bool showSparky = false;
		static bool showDirectoryChooser = false;

		if (showScene)           ShowSceneStuff(&showScene);
		if (showCodeFile)           ShowCodeFiles(&showCodeFile);
		if (showDirectoryChooser)           ShowDirectoryChooser(&showDirectoryChooser);
		//if (showFile)           ShowExampleAppDocuments(&show_app_documents); 
		//if (showSparky)       ShowExampleAppMainMenuBar();


		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}


		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass - thru hole, so we ask Begin() to not render a background.
		//if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			//window_flags |= ImGuiWindowFlags_NoBackground;


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("Scene"))
			{
				if(ImGui::Button("Open")) {
					showScene = true;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::Button("Open code files..."))
				{
					showCodeFile = true;
				}

				if (ImGui::Button("Save"))
				{
					//showDirectoryChooser = true;
				}
				if (ImGui::Button("SaveAs..."))
				{
					showDirectoryChooser = true;
				}
				if (ImGui::Button("SaveAll"))
				{
					//ShowFileExp();
				}
				ImGui::EndMenu();
			}				
			if (ImGui::BeginMenu("Sparky"))
			{
				if (ImGui::Button("Quit"))
				{
						Application::Get().SetRunningState(false);
				}
				if (ImGui::BeginMenu("About"))
				{
						ShowAboutSparkyWindow();
						ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}				
			ImGui::EndMenuBar();
		}

		ImGui::End();
	}


	///////////////////////////////////////////
	//////////////EditorLayer/////////////////
	/////////////////////////////////////////

	///////////////////////////////////////
	////////////FORALLPLATFORMS///////////
	/////////////////////////////////////

	///////////////////////////////////////////////////////////
	////////////////SHOULD-MOVE-TO-EDITOR-PROJECT/////////////
	/////////////////////////////////////////////////////////




	EditorLayer::EditorLayer() : Layer("EditorLayer")
	{
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Platform/Renderer bindings
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void EditorLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void EditorLayer::Begin()
	{

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void EditorLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
	void EditorLayer::OnImGuiRender()
	{

	}
}

