#pragma once

#include "Saturn/Layer.h"
#include "Saturn/Core/Base.h"
#include "Saturn/Log.h"
#include "Saturn/Events/ApplicationEvent.h"
#include "Saturn/Events/KeyEvent.h"
#include "Saturn/Events/MouseEvent.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Entity.h"
#include "Saturn/Editor/EditorCamera.h"
#include "Saturn/Renderer/Mesh.h"
#include "Saturn/Core/Ray.h"

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
	};

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
		bool OnMouseButtonPressed(MouseButtonEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		std::pair<float, float> GetMouseViewportSpace();
		std::pair<glm::vec3, glm::vec3> CastRay(float mx, float my);
		Ray CastMouseRay();
		void SelectEntity(Entity entity);
		float GetSnapValue();

		void DeserialiseDebugLvl();

		void Begin();
		void End();


		enum class PropertyFlag
		{
			None = 0, ColorProperty = 1, DragProperty = 2, SliderProperty = 4
		};

		// ImGui UI helpers
		bool Property(const std::string& name, bool& value);
		bool Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		bool Property(const std::string& name, glm::vec2& value, PropertyFlag flags);
		bool Property(const std::string& name, glm::vec2& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		bool Property(const std::string& name, glm::vec3& value, PropertyFlag flags);
		bool Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		bool Property(const std::string& name, glm::vec4& value, PropertyFlag flags);
		bool Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);


	private:
		void SaveSceneAs();

		Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;

		EditorCamera m_EditorCamera;

		struct SelectedSubmesh
		{
			Saturn::Entity Entity;
			Submesh* Mesh = nullptr;
			float Distance = 0.0f;
		};


		enum class SelectionMode
		{
			None = 0, Entity = 1, SubMesh = 2
		};

		glm::vec2 m_ViewportBounds[2];
		int m_GizmoType = -1; // -1 = no gizmo
		float m_SnapValue = 0.5f;
		float m_RotationSnapValue = 45.0f;
		bool m_AllowViewportCameraEvents = false;
		bool m_DrawOnTopBoundingBoxes = false;

		SelectionMode m_SelectionMode;
		std::vector<SelectedSubmesh> m_SelectionContext;
		Ref<Scene> m_RuntimeScene, m_EditorScene;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		std::thread m_Serialiser_Thread;

		void OnSelected(const SelectedSubmesh& selectionContext);

		friend class SceneHierarchyPanel;

	};

	class SATURN_API SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene> & scene);
		void SetContext(const Ref<Scene>& scene);
		void SetSelected(Entity entity);
		void SetSelectionChangedCallback(const std::function<void(Entity)>& func) { m_SelectionChangedCallback = func; }

		void OnImGuiRender();
		void OnUpdate(Timestep ts);

		Entity& GetSelectionContext() { return m_SelectionContext; }


	private:
		void DrawEntityNode(Entity entity);
		void DrawEntityComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;

		std::function<void(Entity)> m_SelectionChangedCallback;

		friend class EditorLayer;
	};
}
