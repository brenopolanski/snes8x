#pragma once

#include "EmulatorInput.h"
#include <collection.h>

using namespace Windows::UI::Input;
using namespace Platform::Collections;

#define ARRAY_SIZE			1009
#define SMALL_ARRAY_SIZE	10

namespace Emulator
{
	class EmulatorGame;

	class VirtualControllerInput
		: public EmulatorInput
	{
	public:
		VirtualControllerInput(void);
		~VirtualControllerInput(void);

		const ControllerState *GetControllerState(void);

		void GetStickRectangle(RECT *rect);
		void GetStickCenterRectangle(RECT *rect);
		bool StickFingerDown(void);
		void Reset(void);
		
		void PointerPressed(PointerPoint ^point);
		void PointerMoved(PointerPoint ^point);
		void PointerReleased(PointerPoint ^point);

		void UpdateVirtualControllerRectangles(void);

		void GetCrossRectangle(RECT *rect);
		void GetButtonsRectangle(RECT *rect);
		void GetStartSelectRectangle(RECT *rect);
		void GetLRectangle(RECT *rect);
		void GetRRectangle(RECT *rect);

	private:
		int top;
		unsigned int PointerIDs[SMALL_ARRAY_SIZE];
		Platform::Array<PointerPoint ^> ^pointersArray;

		CRITICAL_SECTION cs;
		EmulatorGame *emulator;
		ControllerState state;
		Map<unsigned int, PointerPoint ^> ^pointers;

		bool stickFingerDown;
		int stickFingerID;
		Windows::Foundation::Point stickPos;
		Windows::Foundation::Point stickOffset;
		POINT visibleStickPos;
		POINT visibleStickOffset;
		Windows::Foundation::Rect stickBoundaries;

		RECT padCrossRectangle;
		RECT startSelectRectangle;
		RECT buttonsRectangle;
		RECT lRectangle;
		RECT rRectangle;

		Windows::Foundation::Rect leftRect;
		Windows::Foundation::Rect upRect;
		Windows::Foundation::Rect rightRect;
		Windows::Foundation::Rect downRect;
		Windows::Foundation::Rect startRect;
		Windows::Foundation::Rect selectRect;
		Windows::Foundation::Rect lRect;
		Windows::Foundation::Rect rRect;
		Windows::Foundation::Rect aRect;
		Windows::Foundation::Rect bRect;
		Windows::Foundation::Rect xRect;
		Windows::Foundation::Rect yRect;

		void CreateRectangleOnTheLeft(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
		void CreateRectangleOnTheRight(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
		void CreateTouchRectangleOnTheLeft(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
		void CreateTouchRectangleOnTheRight(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
	};
}