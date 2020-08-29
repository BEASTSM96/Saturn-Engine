#pragma once

#include "Sparky/Core.h"
#include "Sparky/Events/Event.h"

#include "Core/Timestep.h"

#include "Core/Serialisation/Serialiser.h"

namespace Sparky {

	class SPARKY_API Layer : public Serialiser
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }

		std::string m_DebugName;
	protected:
		virtual void archive() override {

			SerialisationData(new Serialisable<std::string>("Layer Debug Name : ", &m_DebugName));
		}

	};
}

