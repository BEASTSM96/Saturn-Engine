#pragma once

#include "Sparky/Input.h"

namespace Sparky {

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
