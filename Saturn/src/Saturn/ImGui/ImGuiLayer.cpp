#include "sppch.h"
#include "ImGuiLayer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Saturn/Scene/Components.h"

#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#include "Saturn/Application.h"

#include "Saturn/Core/Timestep.h"

#include "Saturn/Log.h"

#include "ImGuiFileDialog.cpp"

#include "ImGuizmo.h"
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
		//ImGui::StyleColorsclass SATURN_APIic();

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

	static bool canValidateDialog = false;
	SAT_FORCE_INLINE void InfosPane(std::string vFilter, igfd::UserDatas vUserDatas, bool* vCantContinue) // if vCantContinue is false, the user cant validate the dialog
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

		//FramebufferSpecification fbSpec;
		//fbSpec.Width = 1280;
		//fbSpec.Height = 720;
		//m_Framebuffer = Framebuffer::Create(fbSpec);



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


	void EditorLayer::OnUpdate(Timestep ts)
	{
	}

	void EditorLayer::OnImGuiRender()
	{

		ImGuizmo::BeginFrame();
		
		static bool dock = true;
		{
			ImGuiIO& io = ImGui::GetIO();

			io.ConfigWindowsMoveFromTitleBarOnly = true;

			if (ImGui::Begin("Viewport")) {	
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

			if (ImGui::Begin("Inspector")) {
				
			}

			ImGui::End();
			ImGui::End();
#if 0
			ImGui::End();
			ImGui::End();
			ImGui::End();
			ImGui::End();
			ImGui::End();
			ImGui::End();
#endif // 0



		} 

	}

	template<typename T>
	inline void EditorLayer::DrawInfo(const char* name, bool* p_open, void* flags, T comp, std::string compname, T compnameinfo)
	{
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
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;


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
			}
			else
				SAT_CORE_WARN("Entity dose not have a IdComponent!");

		}
		else
			SAT_CORE_ASSERT(entity.HasComponent<TagComponent>(), "Entity dose not have a TagComponent!");

		
		if (entity.HasComponent<TransformComponent>())
		{
			auto& pos = entity.GetComponent<TransformComponent>().Transform;

			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
				if (ImGui::DragFloat3("Position", glm::value_ptr(entity.GetComponent<TransformComponent>().Transform[3]), 0.5f));
				//if (ImGui::DragFloat3("Rotation", glm::value_ptr(entity.GetComponent<TransformComponent>().Transform[4]), 0.5f));
				if (ImGui::DragFloat3("Scale", glm::value_ptr(entity.GetComponent<TransformComponent>().Transform[2]), 0.5f));

				ImGui::TreePop();
			}
		}
		else
			SAT_CORE_ASSERT(entity.HasComponent<TransformComponent>(), "Entity dose not have a TransformComponent!");


	}

} //namespace
