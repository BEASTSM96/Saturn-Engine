#pragma once

#include "Sparky/Layer.h"

#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/KeyEvent.h"
#include "Sparky/Events/MouseEvent.h"

namespace Sparky {


#define IM_STATIC_FUNC static void
#define IM_STATIC_BOOL static bool
	class SPARKY_API ImGuiLayer : public Layer
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

	class SPARKY_API ImGuiFPS : public Layer
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
	};

	class SPARKY_API ImGuiRenderStats : public Layer
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

	class SPARKY_API ImguiTopBar : public Layer
	{
	public:
		ImguiTopBar();
		~ImguiTopBar();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		float m_Time = 0.0f;
	};

	class SPARKY_API EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		OPENFILENAME ofn;       // common dialog box structure
		TCHAR szFile[260] = { 0 };       // if using TCHAR macros

		float m_Time = 0.0f;
	};
}
