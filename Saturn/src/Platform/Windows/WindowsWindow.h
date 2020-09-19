#pragma once

#include "Saturn/Window.h"

#include <GLFW\glfw3.h>


#include "Saturn/Renderer\GraphicsContext.h"

namespace Saturn {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		SAT_FORCE_INLINE unsigned int GetWidth() const override { return m_Data.Width; }
		SAT_FORCE_INLINE unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		SAT_FORCE_INLINE void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual void* GetNativeWindow() const { return m_Window; };

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;

		GraphicsContext* m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}