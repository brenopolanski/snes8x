#ifndef CXBOXCONTROLLER_H_
#define CXBOXCONTROLLER_H_

#include <Windows.h>
#include <Xinput.h>

namespace Emulator
{
	class CXBOXController
	{
	private:
		XINPUT_STATE state;
		int controllerNumber;

	public:
		CXBOXController (int playerNumber);
		XINPUT_STATE GetState(void);
		bool IsConnected();
		void Vibrate(int leftVal = 0, int rightVal = 0);
	};
}

#endif