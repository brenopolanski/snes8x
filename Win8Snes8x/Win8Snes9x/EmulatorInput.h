#pragma once

#include "Input.h"

namespace Emulator
{
	class EmulatorInput
	{
	public:
		virtual ~EmulatorInput(void) { }

		const virtual ControllerState *GetControllerState(void) = 0;
	};
}
