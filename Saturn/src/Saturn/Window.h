#pragma once

#include "sppch.h"

#include "Core/Base.h"
#include "Events/Event.h"

namespace Saturn {

	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

#ifdef SAT_DEBUG
		WindowProps( const std::string& title = "Saturn Engine Mode : Debug",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title( title ), Width( width ), Height(height)
		{
		}
#endif // SAT_DEBUG

#ifdef SAT_DIST
		WindowProps( const std::string& title = "Saturn Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title( title ), Width( width ), Height( height )
		{
		}
#endif // SAT_DIST

#ifdef SAT_RELEASE
		WindowProps( const std::string& title = "Saturn Engine Mode : Release",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title( title ), Width( width ), Height( height )
		{
		}
#endif // SAT_RELEASE
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback( const EventCallbackFn& callback ) = 0;
		virtual void SetVSync( bool enabled ) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create( const WindowProps& props = WindowProps() );
	};

}