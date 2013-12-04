#pragma once

#include "Input.h"
#include "EmulatorInput.h"

using namespace Windows::UI::Core;

namespace Emulator
{
	class KeyboardInput
		: public EmulatorInput
	{		
	public:
		KeyboardInput(void);
		~KeyboardInput(void);

		const ControllerState *GetControllerState(void);

	private:
		ControllerState state;
		CoreWindow ^window;
	};
}