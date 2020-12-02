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
		Layer( const std::string& name = "Layer" );
		virtual ~Layer( void );

		virtual void OnAttach( void ) {}
		virtual void OnDetach( void ) {}
		virtual void OnUpdate( Timestep ts ) {}
		virtual void OnImGuiRender( void ) {}
		virtual void OnEvent( Event& event ) {}

		const std::string& GetName( void ) const { return m_DebugName; }
	private:
		std::string m_DebugName;

	};
}

