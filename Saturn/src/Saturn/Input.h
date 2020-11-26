#pragma once

#include "Saturn/Core/Base.h"
#include "KeyCodes.h"
#include "MouseButtons.h"

namespace Saturn {

	class SATURN_API Input 
	{
	
	public:
		static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
	
		static float GetMouseX() { return s_Instance->GetMouseXImpl(); }

		static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

		static std::pair<float , float> GetMousePos() { return s_Instance->GetMousePosImpl(); }

		static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;

		virtual float GetMouseXImpl() = 0;

		virtual float GetMouseYImpl() = 0;

		virtual std::pair<float, float> GetMousePosImpl() = 0;

		virtual bool IsMouseButtonPressedImpl(int button) = 0;
	private:
		static Input* s_Instance;

	};
}