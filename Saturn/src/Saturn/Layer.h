#pragma once

#include "Saturn/Core/Base.h"
#include "Core/Timestep.h"

#include "Saturn/Core/Serialisation/Serialiser.h"
#include "Saturn/Events/Event.h"
#include "Saturn/Events/KeyEvent.h"
#include "Saturn/Events/MouseEvent.h"
#include "Saturn/Events/ApplicationEvent.h"

namespace Saturn {

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		const std::string& GetName() const { return m_DebugName; }
	private:
		std::string m_DebugName;

	};
}

