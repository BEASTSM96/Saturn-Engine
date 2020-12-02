#pragma once

#include "Saturn/Input.h"

namespace Saturn {

	class WindowsInput : public Input
	{

	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;

		virtual float GetMouseXImpl( void ) override;

		virtual float GetMouseYImpl( void ) override;

		virtual bool IsMouseButtonPressedImpl(int button) override;

		virtual std::pair<float, float> GetMousePosImpl( void ) override;
	};

}
