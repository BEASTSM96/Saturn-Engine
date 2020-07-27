#pragma once

#include "Sparky/Layer.h"

#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/KeyEvent.h"
#include "Sparky/Events/MouseEvent.h"


namespace Sparky {

	class SPARKY_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		float m_Time = 0.0f;
	};
}
