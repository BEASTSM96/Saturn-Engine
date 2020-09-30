#include "sppch.h"
#include "ImGuiLayer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Saturn/Scene/Components.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#include "Saturn/Core/Serialisation/Serialiser.h"

#include "Saturn/Application.h"

#include "Saturn/Core/Timestep.h"

#include "Saturn/Log.h"

#include "Saturn/Scene/Components.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>



// TEMPORARY
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Saturn {

#define IMGUI_class SATURN_APIES_SPARKY -0
#define IMGUI_OBJECTS_SPARKY nullptr

	///////////////////////////////////////
	////////////FORALLPLATFORMS///////////
	/////////////////////////////////////

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{

		archive();
	}

	void ImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
																	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
																	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
																	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;																	// Setup Dear ImGui style
		ImGui::StyleColorsDark();

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
	}

	void ImGuiLayer::OnImGuiRender()
	{
		archive();
	}


	////////////////////////////////////////////////
	////////////////IMGUIFPS///////////////////////
	//////////////////////////////////////////////


	//////////////////////////////////////////
	////////////NOTFORALLPLATFORMS///////////
	////////////////////////////////////////

	//#define SAT_FPS
	ImGuiFPS::ImGuiFPS() : Layer("ImguiFPS")
	{
		archive();
	}

	ImGuiFPS::~ImGuiFPS()
	{
	}

	void ImGuiFPS::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

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
			//SAT_CORE_INFO("FPS {0}", timestep);
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
		archive();
	}

	ImGuiRenderStats::~ImGuiRenderStats()
	{
	}

	void ImGuiRenderStats::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
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

		archive();

	}

	ImguiTopBar::~ImguiTopBar()
	{
	}

	void ImguiTopBar::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Platform/Renderer bindings
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImguiTopBar::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
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
	}


	static void ShowAboutSparkyWindow()
	{
		ImGui::MenuItem("(About menu)", NULL, false, false);
		if (ImGui::MenuItem("About")) {}
		if (ImGui::MenuItem("ver :", "v 0.21")) {}
		if (ImGui::MenuItem("Build :", "dev")) {}
		if (ImGui::MenuItem("GitHub branch :", "master")) {}
	}

	static void ShowSceneStuff(bool* p_open)
	{
	}

	static void ShowCodeFiles(bool* p_open)
	{
	}

	static void ShowDirectoryChooser(bool* p_open)
	{
	}

#ifdef ED_ENUMS
	std::string ImguiTopBar::GetContextForType(E_EditorFileType type)
	{
		if (type == E_EditorFileType::FileScene)
		{
			return ".sats";
		}
	}
#endif
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
				if (ImGui::Button("Open")) {

					auto [name, ex] = Application::Get().OpenFile("SaturnSceneFiles\0* .sats;\0\0");

					std::ifstream sc(name + ex);

					sc >> Application::Get().GetCurrentScene().GetData().name;

					SAT_CORE_WARN("Scene Name =  {0}", Application::Get().GetCurrentScene().GetData().name);
				}

				if (ImGui::Button("Save")) {
					auto[name, ex] = Application::Get().SaveFile(".sats\0* .sats;\0\0");



					Json::Value s;

					{

						if (ex == ".sats")
						{
							ex = "";
						}

						std::ofstream sc(name + ex);

						s["Level"]["Name"] = Application::Get().GetCurrentScene().GetData().name;

						sc << s;

					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::Button("Open code files..."))
				{
					auto [name, ex] = Application::Get().OpenFile("CodeFiles\0* .cpp; * .h;\0\0");
				}

				if (ImGui::Button("Save"))
				{

				}
				if (ImGui::Button("SaveAs..."))
				{
					auto [name, ex] = Application::Get().SaveFile("ye");
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

	///////////////////////////////////////////
	////////////FORALLPLATFORMS--NOT///////////
	//////////////////////////////////////////

	///////////////////////////////////////////////////////////
	////////////////SHOULD-MOVE-TO-EDITOR-PROJECT/////////////
	/////////////////////////////////////////////////////////




	EditorLayer::EditorLayer() : Layer("EditorLayer")
	{
		archive();
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		FramebufferSpecification fbSpec;
		fbSpec.Width = Application::Get().GetWindow().GetWidth();
		fbSpec.Height = Application::Get().GetWindow().GetHeight();
		m_Framebuffer = Framebuffer::Create(fbSpec);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImVec4* colors = ImGui::GetStyle().Colors;

#define SAT_DARK_THMEME
#ifdef SAT_DARK_THMEME
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.33f, 0.33f, 0.33f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.41f, 0.41f, 0.41f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.38f, 0.38f, 0.38f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.65f, 0.65f, 0.65f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.34f, 0.34f, 0.34f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.54f, 0.54f, 0.54f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.55f, 0.55f, 0.55f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.40f, 0.40f, 0.40f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.51f, 0.51f, 0.51f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.21f, 0.21f, 0.21f, 0.86f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.27f, 0.27f, 0.80f);
		colors[ImGuiCol_TabActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.19f, 0.19f, 0.19f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.44f, 0.69f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.66f, 0.71f, 1.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

#elif defined(SAT_DARK_THMEME_TWO)

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
		colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
		colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

#elif defined(SAT_DARK_THMEME_THREE)
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
		colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

#elif defined(SAT_DARK_THMEME_FOUR)
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 5.3f;
		style.FrameRounding = 2.3f;
		style.ScrollbarRounding = 0;

		//From the theme one
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.33f, 0.33f, 0.33f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.41f, 0.41f, 0.41f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.38f, 0.38f, 0.38f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.65f, 0.65f, 0.65f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.34f, 0.34f, 0.34f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.54f, 0.54f, 0.54f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.55f, 0.55f, 0.55f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.40f, 0.40f, 0.40f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.51f, 0.51f, 0.51f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.21f, 0.21f, 0.21f, 0.86f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.27f, 0.27f, 0.80f);
		colors[ImGuiCol_TabActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.19f, 0.19f, 0.19f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.44f, 0.69f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.66f, 0.71f, 1.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


#endif // SAT_DARK_THMEME

#ifdef SAT_LIGHT_THMEME
		ImGui::StyleColorsLight();
#endif // SAT_LIGHT_THMEME

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


	void EditorLayer::OnUpdate(Timestep ts)
	{
		m_Framebuffer->Bind();
		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{

		ImGuizmo::BeginFrame();
		
		static bool dock = true;
		{
			ImGuiIO& io = ImGui::GetIO();

			io.ConfigWindowsMoveFromTitleBarOnly = true;

			if (ImGui::Begin("Viewport")) {	

				uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
				ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

				ImVec2 portSize =  ImGui::GetContentRegionAvail();
				m_ViewportSize = { portSize.x, portSize.y };
#if 0
				// view/projection transformations
				glm::mat4 projection = glm::perspective(glm::radians(Application::Get().m_gameLayer->Get3DCamera().Zoom), (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), 0.1f, 100.0f);

				glm::mat4 view = Application::Get().m_gameLayer->Get3DCamera().GetViewMatrix();

				glm::mat4& ObjectMatrix = Application::Get().gameObject->GetComponent<TransformComponent>().Transform;

				float* v = glm::value_ptr(view);

				float* p = glm::value_ptr(projection);

				float* o = glm::value_ptr(ObjectMatrix);

				static const float identityMatrix[16] =
				{
					1.f, 0.f, 0.f, 0.f,
					0.f, 1.f, 0.f, 0.f,
					0.f, 0.f, 1.f, 0.f,
					0.f, 0.f, 0.f, 1.f
				};

				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

				ImGuizmo::DrawGrid(v, p, identityMatrix, 100.f);

				{
					static bool q;
					static bool e;
					static bool c;
					static bool z;


					if (Input::IsKeyPressed(SAT_KEY_Q))
					{
						q = true;
						z = false;
						c = false;
						e = false;
					}

					if (q)
					{
						ImGuizmo::Manipulate(v, p, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, o, NULL, NULL);
					}

					if (Input::IsKeyPressed(SAT_KEY_E))
					{
						e = true;
						z = false;
						c = false;
						q = false;
					}

					if (e)
					{
						ImGuizmo::Manipulate(v, p, ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::LOCAL, o, NULL, NULL);
					}

					if (Input::IsKeyPressed(SAT_KEY_C))
					{
						c = true;
						z = false;
						e = false;
						q = false;
					}

					if (c)
					{
						ImGuizmo::Manipulate(v, p, ImGuizmo::OPERATION::SCALE, ImGuizmo::MODE::LOCAL, o, NULL, NULL);
					}

					if (Input::IsKeyPressed(SAT_KEY_Z))
					{
						z = true;
						c = false;
						e = false;
						q = false;
					}

					if (z)
					{
						ImGuizmo::Manipulate(v, p, ImGuizmo::OPERATION::BOUNDS, ImGuizmo::MODE::LOCAL, o, NULL, NULL);
					}
				}
#endif
			}

			io.ConfigWindowsMoveFromTitleBarOnly = true;

			if (ImGui::Begin("Debugger")) {

				if(ImGui::Button("Import")){

					SAT_CORE_INFO("--------------------------------------------------------------------------------------------------");
					SAT_CORE_INFO("Awaiting user file location!");

					auto& [name, ex] = Application::Get().OpenFile(".obj\0* .obj;\0\0");

					auto& [namefs, exfs] = Application::Get().OpenFile(".satshaderf\0* .satshaderf;\0\0");
					auto& [namevs, exvs] = Application::Get().OpenFile(".satshaderv\0* .satshaderv;\0\0");


					SAT_CORE_INFO("Found Files. Files : {0}, {1} and {2}", name, namefs, namevs);


					const char* nameOBJ = name.c_str(); 	const char* exOBJ = ex.c_str();
					const char* FSName = namefs.c_str();	const char* FS_ex = exfs.c_str();
					const char* VSName = namevs.c_str();	const char* VS_ex = exvs.c_str();

					/* Shader and Model Config */
					DShader* importedShader = new DShader(VSName, FSName);
					Model* importedModel = new Model(name);

					SAT_CORE_WARN("Compiling Shader : (ID) {0}", importedShader->ID);
					SAT_CORE_WARN("	Compiling Shaders can take a while!");
					SAT_CORE_WARN("	Compiling Shaders can also use system resources!");

					GameObject * gb = Application::Get().GetCurrentScene().CreateEntityGameObjectprt("");

					SAT_CORE_INFO("Creating new GameObject!");

					gb->ourShader = importedShader;
					gb->ourModel = importedModel;

					SAT_CORE_ASSERT(gb->HasComponent<TagComponent>(), "GameObject dose not A TagComponent!");
					SAT_CORE_INFO("New GameObject made! : (Tag) {0}", gb->GetComponent<TagComponent>().Tag);
					SAT_CORE_WARN("Setting new Shader to the imported shader");
					SAT_CORE_WARN("Setting new Model to the imported Model");
					SAT_CORE_INFO("--------------------------------------------------------------------------------------------------");

					SAT_CORE_INFO("{0} {1} {2} {3} {4} {5}", importedShader->ID, importedModel->directory, VSName, FSName, name, gb->GetComponent<TagComponent>().Tag);

					delete importedShader;
					delete importedModel;
				}

			}

			ImGui::End();
			ImGui::End();

		} 

	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([&](auto entityID)
		{
			Entity entity{ entityID , m_Context.get() };
			DrawEntityNode(entity);
		});

		ImGui::End();


		if (ImGui::Begin("Inspector")) {

			ImGui::Spacing();
			if (m_SelectionContext) 
				DrawEntityComponents(m_SelectionContext);
			
		}

		ImGui::End();


	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		auto& id = entity.GetComponent<IdComponent>().Id;

		auto& transform = entity.GetComponent<TransformComponent>().Transform;

		float* t =  glm::value_ptr(transform);

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;

		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();

			if (ImGui::Begin("Viewport")) {
				// view/projection transformations
				glm::mat4 projection = glm::perspective(glm::radians(Application::Get().m_gameLayer->Get3DCamera().Zoom), (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), 0.1f, 100.0f);

				glm::mat4 view = Application::Get().m_gameLayer->Get3DCamera().GetViewMatrix();

				glm::mat4& ObjectMatrix = Application::Get().gameObject->GetComponent<TransformComponent>().Transform;

				float* v = glm::value_ptr(view);

				float* p = glm::value_ptr(projection);

				float* o = glm::value_ptr(ObjectMatrix);

				ImGuizmo::BeginFrame();

				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

				static const float identityMatrix[16] =
				{
					1.f, 0.f, 0.f, 0.f,
					0.f, 1.f, 0.f, 0.f,
					0.f, 0.f, 1.f, 0.f,
					0.f, 0.f, 0.f, 1.f
				};

				ImGuizmo::DrawGrid(v, p, identityMatrix, 100.f);

				ImGuizmo::Manipulate(v, p, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, o, NULL, NULL);
			}

			ImGui::End();

		}

	}

	void SceneHierarchyPanel::DrawEntityComponents(Entity entity)
	{
		auto& tagORNL = entity.GetComponent<TagComponent>().Tag;

		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			static bool hasOldTag = false;
			if (!hasOldTag)
			{

				hasOldTag = true;
			}

			char buffer[265];

			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			if (entity.HasComponent<IdComponent>())
			{
				auto& id = entity.GetComponent<IdComponent>().Id;

				ImGui::Text("Tag"); ImGui::SameLine();
				if (ImGui::InputText("##empty", buffer, sizeof(buffer)))
				{
					tag = std::string(buffer);
				}

				ImGui::SameLine();
				ImGui::Text("%f", id);

				tag = tag.empty() ? "Unmanned GameObject / Entity" : tag;
			}
			else
			{
				ImGui::Text("Tag"); ImGui::SameLine();
				if (ImGui::InputText("##empty", buffer, sizeof(buffer)))
				{
					tag = std::string(buffer);
				}

				tag = tag.empty() ? "Unmanned GameObject / Entity" : tag;

			}

		}
		else
			SAT_CORE_ASSERT(entity.HasComponent<TagComponent>(), "Entity dose not have a TagComponent!");

		
		if (entity.HasComponent<TransformComponent>())
		{
			auto& pos = entity.GetComponent<TransformComponent>().Transform;

			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
				if (ImGui::DragFloat3("Position", glm::value_ptr(entity.GetComponent<TransformComponent>().Transform[3]), 0.5f));

				if (ImGui::DragFloat3("Scale", glm::value_ptr(entity.GetComponent<TransformComponent>().Transform[2]), 0.5f));

				ImGui::TreePop();
			}
		}
		else
			SAT_CORE_ASSERT(entity.HasComponent<TransformComponent>(), "Entity dose not have a TransformComponent!");
	}

} //namespace