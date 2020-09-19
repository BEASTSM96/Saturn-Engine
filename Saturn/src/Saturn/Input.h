#pragma once

#include "Saturn/Core.h"


namespace Saturn {

	class SATURN_API Input 
	{
	
	public:
		SAT_FORCE_INLINE static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
	
		SAT_FORCE_INLINE static float GetMouseX() { return s_Instance->GetMouseXImpl(); }

		SAT_FORCE_INLINE static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

		SAT_FORCE_INLINE static std::pair<float , float> GetMousePos() { return s_Instance->GetMousePosImpl(); }

		SAT_FORCE_INLINE static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }

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