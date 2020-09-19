#pragma once



#include "Saturn/Layer.h"
#include "Saturn/Core.h"
#include "Saturn/Log.h"

#include "Saturn/Events/ApplicationEvent.h"
#include "Saturn/Events/KeyEvent.h"
#include "Saturn/Events/MouseEvent.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Entity.h"

#include "Saturn/Editor/Core.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	class SATURN_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		float m_Time = 0.0f;
		virtual void archive() override {

			SerialisationData(new Serialisable<int>("m_Time", &m_Time));
		}

	};

	class  SATURN_API ImGuiFPS : public Layer
	{
	public:
		ImGuiFPS();
		~ImGuiFPS();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		float m_Time = 0.0f;

		virtual void archive() override {

			SerialisationData(new Serialisable<int>("m_Time", &m_Time));
		}
	};

	class  SATURN_API ImGuiRenderStats : public Layer
	{
	public:
		ImGuiRenderStats();
		~ImGuiRenderStats();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		float m_Time = 0.0f;

		virtual void archive() override {

			SerialisationData(new Serialisable<int>("m_Time", &m_Time));
		}
	};

	class  SATURN_API ImguiTopBar : public Layer
	{
	public:
		ImguiTopBar();
		~ImguiTopBar();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

		void EditTransform(const float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition);

	private:
#ifdef ED_ENUMS
		std::string GetContextForType(E_EditorFileType type);
#endif
		float m_Time = 0.0f;

		virtual void archive() override {

			SerialisationData(new Serialisable<int>("m_Time", &m_Time));
		}
	};

	class  SATURN_API EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;

		void EditTransform(glm::mat4  t);

		void Begin();
		void End();

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

	private:

		template<typename T>
		static void DrawInfo(const char* name, bool* p_open, void* flags, T comp, std::string compname, T compnameinfo);

	private:
		float m_Time = 0.0f;

		Ref<Framebuffer> m_Framebuffer;

		virtual void archive() override {

			SerialisationData(new Serialisable<int>("m_Time", &m_Time));
		}

		friend class SceneHierarchyPanel;

	};

	class SATURN_API SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene> & scene);

		void SetContext(const Ref<Scene> & scene);

		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
		void DrawEntityComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;

		friend class EditorLayer;
	};
}
