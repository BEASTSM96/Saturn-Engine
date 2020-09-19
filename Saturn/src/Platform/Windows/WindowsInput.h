#pragma once

#include "Saturn/Input.h"

namespace Saturn {

	class WindowsInput : public Input
	{

	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;

		virtual float GetMouseXImpl() override;

		virtual float GetMouseYImpl() override;

		virtual bool IsMouseButtonPressedImpl(int button) override;

		virtual std::pair<float, float> GetMousePosImpl() override;
	};

}
