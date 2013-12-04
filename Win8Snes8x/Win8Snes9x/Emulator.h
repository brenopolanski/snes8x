#ifndef EMULATOR_H_
#define EMULATOR_H_

#include "CXAudio2.h"
#include "IS9xSoundOutput.h"
#include <ppltasks.h>
#include <collection.h>
#include "VirtualControllerInput.h"
#include "KeyboardInput.h"
#include "ControllerInput.h"
#include "GameTime.h"

using namespace Platform::Collections;
using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;
using namespace Windows::UI::Input;
using namespace Windows::System::Threading;
using namespace Windows::System;

bool8 S9xOpenSoundDevice (void);
void S9xSoundCallback(void *data);
bool8 S9xDeinitUpdate (int Width, int Height);

#define EXT_WIDTH (MAX_SNES_WIDTH + 4)
#define EXT_PITCH (EXT_WIDTH * 2)
#define EXT_HEIGHT (MAX_SNES_HEIGHT + 4)
#define EXT_OFFSET (EXT_PITCH * 2 + 2 * 2)

namespace Emulator
{
	struct ROMData
	{
		unsigned char *ROM;
		size_t Length;
	};

	class EmulatorGame
	{
	private:
		static EmulatorGame *instance;

		uint32_t frontbuffer, backbuffer;		
		bool stopThread;
		Windows::Foundation::IAsyncAction ^threadAction;
		HANDLE s9xUpdateEvent;
		HANDLE flipBufferEvent;
		HANDLE sleepEvent;	
		CRITICAL_SECTION pauseSync;
		CXAudio2 audio;
		uint32_t width, height;
		BYTE *gfxbuffer;
		bool focus;		
		bool graphicsResourcesReleased;
		bool restoreState;
		ControllerInput *p1Controller;
		ControllerInput *p2Controller;
		KeyboardInput *keyboard;
		VirtualControllerInput *virtualInput;
		int updateCount;
		bool frameSkipped;

		void S9xUpdateAsync(void);
		void InitSound(void);
		void HandleInput(void);
		void FlipBuffers(void *buffer, size_t rowPitch, float elapsedTime);
		void SyncSound(void);

	public:
		static IS9xSoundOutput *Audio;
		static int SnesImageWidth, SnesImageHeight;

		EmulatorGame(bool restore);
		~EmulatorGame(void);
		void ReleaseAllResources(void);
		static EmulatorGame *GetInstance();		
		VirtualControllerInput *GetVirtualController(void) const;
		int GetWidth(void);
		int GetHeight(void);
		void FocusChanged(bool focus);
		void ResizeBuffer(float width, float height);
		void Initialize();
		task<void> StopROMAsync(void);
		bool IsPaused(void);
		void Pause(void);
		void Unpause(void);
		void Update(void *buffer, size_t rowPitch, float elapsedTime);
		bool LastFrameSkipped(void);
	};
}

#endif