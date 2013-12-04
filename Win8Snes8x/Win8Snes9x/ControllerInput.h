#pragma once

#include "Input.h"
#include "CXBOXController.h"
#include "EmulatorInput.h"

namespace Emulator
{
	class ControllerInput
		: public EmulatorInput
	{
	public:
		ControllerInput(int index);
		~ControllerInput(void);

		const ControllerState *GetControllerState(void);

	private:
		ControllerState state;
		CXBOXController *xboxPad;
	};
}