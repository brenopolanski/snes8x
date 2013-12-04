#pragma once

#include "Emulator.h"

using namespace Windows::System;

#define MONITOR_30HZ	0
#define MONITOR_60HZ	1
#define MONITOR_120HZ	2

namespace Emulator
{		
	void StoreSettings(void);
	void RestoreSettings(void);

	VirtualKey GetLeftKeyBinding(void);
	void SetLeftKeyBinding(VirtualKey key);
	VirtualKey GetRightKeyBinding(void);
	void SetRightKeyBinding(VirtualKey key);
	VirtualKey GetUpKeyBinding(void);
	void SetUpKeyBinding(VirtualKey key);
	VirtualKey GetDownKeyBinding(void);
	void SetDownKeyBinding(VirtualKey key);
	VirtualKey GetStartKeyBinding(void);
	void SetStartKeyBinding(VirtualKey key);
	VirtualKey GetSelectKeyBinding(void);
	void SetSelectKeyBinding(VirtualKey key);
	VirtualKey GetAKeyBinding(void);
	void SetAKeyBinding(VirtualKey key);
	VirtualKey GetBKeyBinding(void);
	void SetBKeyBinding(VirtualKey key);
	VirtualKey GetYKeyBinding(void);
	void SetYKeyBinding(VirtualKey key);
	VirtualKey GetXKeyBinding(void);
	void SetXKeyBinding(VirtualKey key);
	VirtualKey GetLKeyBinding(void);
	void SetLKeyBinding(VirtualKey key);
	VirtualKey GetRKeyBinding(void);
	void SetRKeyBinding(VirtualKey key);
	VirtualKey GetTurboKeyBinding(void);
	void SetTurboKeyBinding(VirtualKey key);

	void SetFrameSkip(int skip);
	int GetFrameSkip(void);
	void SetTurboFrameSkip(int skip);
	int GetTurboFrameSkip(void);
	void SetPowerFrameSkip(int skip);
	int GetPowerFrameSkip(void);
	void SetControllerOpacity(int opacity);
	int GetControllerOpacity(void);
	int GetControllerScaling(void);
	void SetControllerScaling(int scaling);

	void EnableAutosaving(bool enable);
	bool AutosavingEnabled(void);
	bool TouchButtonsIntersecting(void);
	void SetTouchButtonsIntersecting(bool intersecting);
	bool IsTouchControllerOnTop(void);
	void SetTouchControllerOnTop(bool onTop);
	/*bool LowDisplayRefreshModeActivated(void);
	void EnableLowDisplayRefreshMode(bool enable);*/
	bool TouchControlsEnabled(void);
	void EnableTouchControls(bool enable);
	bool SoundEnabled(void);
	void EnableSound(bool enable);
	bool ShowingFPS(void);
	void ShowFPS(bool show);
	bool AssignedToPlayer2(void);
	void AssignToPlayer2(bool assignP2);
	bool ButtonsGray(void);
	void SetButtonsGray(bool gray);
	bool SynchronizeAudio(void);
	void SetSynchronizeAudio(bool sync);
	int GetDPadStyle(void);
	void SetDPadStyle(int dpad);
	float GetDeadzone(void);
	void SetDeadzone(float value);
	int GetMonitorType(void);
	void SetMonitorType(int type);

	bool IsSaveConfirmationDisabled(void);
	void DisableSaveConfirmation(bool disable);

	bool IsLoadConfirmationDisabled(void);
	void DisableLoadConfirmation(bool disable);
}