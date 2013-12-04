//#include "App.xaml.h"
#include "Emulator.h"
#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "display.h"
#include "apu/apu.h"
#include "controls.h"
#include "snapshot.h"
#include "EmulatorFileHandler.h"
#include "EmulatorSettings.h"
#include <d3d11_1.h>

#define JOYPAD_A			1
#define JOYPAD_B			2
#define JOYPAD_X			3
#define JOYPAD_Y			4
#define JOYPAD_L			5
#define JOYPAD_R			6
#define JOYPAD_START		7
#define JOYPAD_SELECT		8
#define JOYPAD_LEFT			9
#define JOYPAD_RIGHT		10
#define JOYPAD_UP			11
#define JOYPAD_DOWN			12

#define JOYPAD2_A			21
#define JOYPAD2_B			22
#define JOYPAD2_X			23
#define JOYPAD2_Y			24
#define JOYPAD2_L			25
#define JOYPAD2_R			26
#define JOYPAD2_START		27
#define JOYPAD2_SELECT		28
#define JOYPAD2_LEFT		29
#define JOYPAD2_RIGHT		30
#define JOYPAD2_UP			31
#define JOYPAD2_DOWN		32

using namespace Platform;
using namespace concurrency;

using namespace Platform;
using namespace Windows::Foundation;

bool enableTurboMode = false;
bool turboPressed = false;

namespace Emulator
{
	//extern bool timeMeasured;
	bool autosaving;
	float lastElapsedTime = 0.0f;
	bool lastSkipped = false;

	IS9xSoundOutput *EmulatorGame::Audio = nullptr;
	int EmulatorGame::SnesImageWidth = 0;
	int EmulatorGame::SnesImageHeight = 0;

	EmulatorGame *EmulatorGame::instance = nullptr;

	EmulatorGame *EmulatorGame::GetInstance(void)
	{
		return EmulatorGame::instance;
	}

	EmulatorGame::EmulatorGame(bool restore) 
		: restoreState(restore),
		graphicsResourcesReleased(false), stopThread(false),
		updateCount(0), frameSkipped(0)
	{
		EmulatorGame::instance = this;
	}

	EmulatorGame::~EmulatorGame(void)
	{
		//this->threadAction->Cancel();
		this->stopThread = true;
		SetEvent(this->s9xUpdateEvent);
		WaitForSingleObjectEx(this->flipBufferEvent, INFINITE, false);
		CloseHandle(this->s9xUpdateEvent);
		CloseHandle(this->flipBufferEvent);
		CloseHandle(this->sleepEvent);
		DeleteCriticalSection(&this->pauseSync);
		audio.DeInitSoundOutput();
		S9xSetSamplesAvailableCallback(nullptr, nullptr);

		this->ReleaseAllResources();

		delete [] this->gfxbuffer;

	}

	void EmulatorGame::ReleaseAllResources(void)
	{
		delete this->p1Controller;
		delete this->p2Controller;
		delete this->keyboard;
		delete this->virtualInput;
		
		this->graphicsResourcesReleased = true;
	}

	void EmulatorGame::Initialize()
	{
		this->p1Controller = new ControllerInput(1);
		this->p2Controller = new ControllerInput(2);
		this->keyboard = new KeyboardInput();
		this->virtualInput = new VirtualControllerInput();
		
		this->updateCount = 0;
		this->frameSkipped = false;

		GFX.RealPPL = EXT_PITCH;

		if(!this->graphicsResourcesReleased)
		{
			memset(&Settings, 0, sizeof(Settings));
			Settings.MouseMaster = true;
			Settings.SuperScopeMaster = true;
			Settings.MultiPlayer5Master = true;
			Settings.JustifierMaster = true;
			Settings.BlockInvalidVRAMAccess = true;
			//Settings.HDMATimingHack = 100;
			Settings.SoundSync = true;
			Settings.SoundPlaybackRate = 32000;
			Settings.Stereo = true;
			Settings.SixteenBitSound = true;
			Settings.Transparency = true;
			Settings.SupportHiRes = true;
			//Settings.SkipFrames = AUTO_FRAMERATE;	

			Settings.FrameTimeNTSC = 16667;
			Settings.FrameTimePAL = 20000;

			Settings.SkipFrames = 10;
			Settings.TurboSkipFrames = 19;

			gfxbuffer = new BYTE[EXT_PITCH * EXT_HEIGHT];
		
			GFX.Screen = (uint16 *) gfxbuffer;
			GFX.Pitch = EXT_PITCH;
			GFX.RealPPL = EXT_PITCH;

			Memory.Init();
		
			S9xInitAPU();
		
			this->InitSound();
		
			S9xSetRenderPixelFormat(RGB565);
			S9xGraphicsInit();
			IPPU.RenderThisFrame = false;

			S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
			S9xSetController(1, CTL_JOYPAD,     1, 0, 0, 0);

			S9xMapButton(JOYPAD_START, S9xGetCommandT("Joypad1 Start"), false);
			S9xMapButton(JOYPAD_SELECT, S9xGetCommandT("Joypad1 Select"), false);
			S9xMapButton(JOYPAD_A, S9xGetCommandT("Joypad1 A"), false);
			S9xMapButton(JOYPAD_B, S9xGetCommandT("Joypad1 B"), false);
			S9xMapButton(JOYPAD_X, S9xGetCommandT("Joypad1 X"), false);
			S9xMapButton(JOYPAD_Y, S9xGetCommandT("Joypad1 Y"), false);
			S9xMapButton(JOYPAD_L, S9xGetCommandT("Joypad1 L"), false);
			S9xMapButton(JOYPAD_R, S9xGetCommandT("Joypad1 R"), false);
			S9xMapButton(JOYPAD_UP, S9xGetCommandT("Joypad1 Up"), false);
			S9xMapButton(JOYPAD_DOWN, S9xGetCommandT("Joypad1 Down"), false);
			S9xMapButton(JOYPAD_LEFT, S9xGetCommandT("Joypad1 Left"), false);
			S9xMapButton(JOYPAD_RIGHT, S9xGetCommandT("Joypad1 Right"), false);
			
			S9xMapButton(JOYPAD2_START, S9xGetCommandT("Joypad2 Start"), false);
			S9xMapButton(JOYPAD2_SELECT, S9xGetCommandT("Joypad2 Select"), false);
			S9xMapButton(JOYPAD2_A, S9xGetCommandT("Joypad2 A"), false);
			S9xMapButton(JOYPAD2_B, S9xGetCommandT("Joypad2 B"), false);
			S9xMapButton(JOYPAD2_X, S9xGetCommandT("Joypad2 X"), false);
			S9xMapButton(JOYPAD2_Y, S9xGetCommandT("Joypad2 Y"), false);
			S9xMapButton(JOYPAD2_L, S9xGetCommandT("Joypad2 L"), false);
			S9xMapButton(JOYPAD2_R, S9xGetCommandT("Joypad2 R"), false);
			S9xMapButton(JOYPAD2_UP, S9xGetCommandT("Joypad2 Up"), false);
			S9xMapButton(JOYPAD2_DOWN, S9xGetCommandT("Joypad2 Down"), false);
			S9xMapButton(JOYPAD2_LEFT, S9xGetCommandT("Joypad2 Left"), false);
			S9xMapButton(JOYPAD2_RIGHT, S9xGetCommandT("Joypad2 Right"), false);

			if(this->restoreState)
			{
				this->restoreState = false;
				RestoreFromApplicationDataAsync();
			}

			RestoreSettings();

			// Start Snes9x Thread
			InitializeCriticalSectionEx(&this->pauseSync, NULL, NULL);
			this->flipBufferEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET, EVENT_ALL_ACCESS);
			this->s9xUpdateEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
			this->sleepEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
			this->threadAction = ThreadPool::RunAsync(ref new WorkItemHandler([this](IAsyncAction ^action)
			{
				this->S9xUpdateAsync();	
			}), WorkItemPriority::High, WorkItemOptions::None);
		}else
		{
			ResetEvent(this->s9xUpdateEvent);
			SetEvent(this->flipBufferEvent);
		}

		this->graphicsResourcesReleased = false;
	}

	void EmulatorGame::InitSound(void)
	{
		EmulatorGame::Audio = &this->audio;
		//Settings.SoundInputRate = 31900;
		Settings.SoundPlaybackRate = 32000;
		this->audio.DeInitSoundOutput();
		S9xInitSound(128,0);
	}

	VirtualControllerInput *EmulatorGame::GetVirtualController(void) const
	{
		return this->virtualInput;
	}

	void EmulatorGame::FocusChanged(bool focus)
	{
		this->focus = focus;
	}

	void uSleep(int waitTime) 
	{
		__int64 time1 = 0, time2 = 0, freq = 0;

		QueryPerformanceCounter((LARGE_INTEGER *) &time1);
		QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

		do {
			QueryPerformanceCounter((LARGE_INTEGER *) &time2);
		} while((time2-time1) < waitTime);
	}

	void EmulatorGame::ResizeBuffer(float width, float height)
	{
		this->width = width;
		this->height = height;
		this->virtualInput->UpdateVirtualControllerRectangles();
	}

	task<void> EmulatorGame::StopROMAsync(void)
	{
		return create_task([this]()
		{
			if(IsROMLoaded())
			{
				this->Pause();
				SaveSRAMAsync().wait();
				this->InitSound();
				Memory.ClearSRAM();
				ROMFile = nullptr;
				ROMFolder = nullptr;
				ROMLoaded = false;
				this->frameSkipped = false;
				this->updateCount = 0;
			}
		});
	}

	void EmulatorGame::Pause(void)
	{
		EnterCriticalSection(&this->pauseSync);
		Settings.StopEmulation = true;
		LeaveCriticalSection(&this->pauseSync);
	}

	void EmulatorGame::Unpause(void)
	{
		if(IsROMLoaded())
		{
			EnterCriticalSection(&this->pauseSync);
			Settings.StopEmulation = false;
			LeaveCriticalSection(&this->pauseSync);
		}
	}

	bool EmulatorGame::IsPaused(void)
	{
		return Settings.StopEmulation;
	}
	
	int EmulatorGame::GetWidth(void)
	{
		return this->width;
	}

	int EmulatorGame::GetHeight(void)
	{
		return this->height;
	}

	bool EmulatorGame::LastFrameSkipped(void)
	{
		return this->frameSkipped;
	}

	void EmulatorGame::S9xUpdateAsync(void)
	{
		WaitForSingleObjectEx(this->s9xUpdateEvent, INFINITE, false);
		while(!this->stopThread)
		{			
			EnterCriticalSection(&this->pauseSync);
			if(!Settings.StopEmulation)
			{
				if(Settings.PAL && updateCount >= 5)
				{
					updateCount = 0;
					this->frameSkipped = true;
				}else
				{
					int loopLength = 1;
					float targetFps = 55.0f;
					int skip = GetPowerFrameSkip();
					if(!turboPressed && !enableTurboMode)
					{
						if(GetMonitorType() == MONITOR_30HZ)
						{
							skip = skip * 2 + 1;
							targetFps = 28.0f;
						}
						if(GetFrameSkip() == -1 && GetPowerFrameSkip() == 0)
						{
							if(!lastSkipped && (lastElapsedTime > (1.0f / targetFps)))
							{
								skip = 1;//(int)((lastElapsedTime / (1.0f / targetFps)));
								lastSkipped = true;
							}else
							{
								lastSkipped = false;
							}
						}else if(GetFrameSkip() >= 0)
						{
							skip += GetFrameSkip();
						}
					}else
					{
						skip = GetTurboFrameSkip();
					}
					loopLength += skip;

					Settings.SkipFrames = loopLength - 1;
					for (int i = 0; i < loopLength; i++)
					{
						if(SynchronizeAudio())
						{
							this->SyncSound();
						}

						S9xMainLoop();
					}

					if(Settings.PAL)
					{
						updateCount++;
					}
					this->frameSkipped = false;
				}
			}
			LeaveCriticalSection(&this->pauseSync);

			SetEvent(this->flipBufferEvent);
			WaitForSingleObjectEx(this->s9xUpdateEvent, INFINITE, false);
		}
		SetEvent(this->flipBufferEvent);
	}

	void EmulatorGame::Update(void *buffer, size_t rowPitch, float elapsedTime)
	{		
		if(/*timeMeasured && */IsROMLoaded() && (!Settings.StopEmulation || autosaving))
		{		
			Settings.Mute = !SoundEnabled();
			
			if(this->focus)
			{
				this->HandleInput();
			}

			this->FlipBuffers(buffer, rowPitch, elapsedTime);			
		}
	}

	void EmulatorGame::FlipBuffers(void *buffer, size_t rowPitch, float elapsedTime)
	{
		WaitForSingleObjectEx(this->flipBufferEvent, INFINITE, false);

		lastElapsedTime = elapsedTime;
		GFX.Screen = (uint16*) (buffer);
		GFX.Pitch = rowPitch;

		SetEvent(this->s9xUpdateEvent);
	}

	void EmulatorGame::SyncSound(void)
	{
		__int64 time1 = 0, time2 = 0;
		QueryPerformanceCounter((LARGE_INTEGER *) &time1);
		while(!S9xSyncSound())
		{
			//wait(2);
			WaitForSingleObjectEx(this->sleepEvent, 2, false);
			QueryPerformanceCounter((LARGE_INTEGER *) &time2);
			if(time2 - time1 > 1000)
			{
				S9xClearSamples();
				break;
			}
		}	
	}

	ControllerState oldKeyboardState;
	ControllerState oldControllerState1;
	ControllerState oldControllerState2;
	ControllerState oldvControllerState;

	void EmulatorGame::HandleInput(void)
	{
		const ControllerState *p1Xbox = this->p1Controller->GetControllerState();
		const ControllerState *p2Xbox = this->p2Controller->GetControllerState();
		const ControllerState *keyboardState = this->keyboard->GetControllerState();
		const ControllerState *virtualState = this->virtualInput->GetControllerState();
		/*const ControllerState *keyboardState = p1Xbox;
		const ControllerState *virtualState = p1Xbox;*/

		bool default1Left = false;
		bool default1Right = false;
		bool default1Down = false;
		bool default1Up = false;
		bool default1Start = false;
		bool default1Select = false;
		bool default1A = false;
		bool default1B = false;
		bool default1X = false;
		bool default1Y = false;
		bool default1L = false;
		bool default1R = false;
		bool default2Left = false;
		bool default2Right = false;
		bool default2Down = false;
		bool default2Up = false;
		bool default2Start = false;
		bool default2Select = false;
		bool default2A = false;
		bool default2B = false;
		bool default2X = false;
		bool default2Y = false;
		bool default2L = false;
		bool default2R = false;
		if(!AssignedToPlayer2())
		{
			default1Left = keyboardState->LeftPressed | virtualState->LeftPressed;
			default1Right = keyboardState->RightPressed | virtualState->RightPressed;
			default1Up = keyboardState->UpPressed | virtualState->UpPressed;
			default1Down = keyboardState->DownPressed | virtualState->DownPressed;
			default1Start = keyboardState->StartPressed | virtualState->StartPressed;
			default1Select = keyboardState->SelectPressed | virtualState->SelectPressed;
			default1A = keyboardState->APressed | virtualState->APressed;
			default1B = keyboardState->BPressed | virtualState->BPressed;
			default1X = keyboardState->XPressed | virtualState->XPressed;
			default1Y = keyboardState->YPressed | virtualState->YPressed;
			default1L = keyboardState->LPressed | virtualState->LPressed;
			default1R = keyboardState->RPressed | virtualState->RPressed;
		}else
		{
			default2Left = keyboardState->LeftPressed | virtualState->LeftPressed;
			default2Right = keyboardState->RightPressed | virtualState->RightPressed;
			default2Up = keyboardState->UpPressed | virtualState->UpPressed;
			default2Down = keyboardState->DownPressed | virtualState->DownPressed;
			default2Start = keyboardState->StartPressed | virtualState->StartPressed;
			default2Select = keyboardState->SelectPressed | virtualState->SelectPressed;
			default2A = keyboardState->APressed | virtualState->APressed;
			default2B = keyboardState->BPressed | virtualState->BPressed;
			default2X = keyboardState->XPressed | virtualState->XPressed;
			default2Y = keyboardState->YPressed | virtualState->YPressed;
			default2L = keyboardState->LPressed | virtualState->LPressed;
			default2R = keyboardState->RPressed | virtualState->RPressed;
		}
				
		S9xReportButton(JOYPAD_START, default1Start | p1Xbox->StartPressed);
		S9xReportButton(JOYPAD_SELECT, default1Select | p1Xbox->SelectPressed);
		S9xReportButton(JOYPAD_A, default1A | p1Xbox->APressed);
		S9xReportButton(JOYPAD_B, default1B | p1Xbox->BPressed);
		S9xReportButton(JOYPAD_X, default1X | p1Xbox->XPressed);
		S9xReportButton(JOYPAD_Y, default1Y | p1Xbox->YPressed);
		S9xReportButton(JOYPAD_L, default1L | p1Xbox->LPressed);
		S9xReportButton(JOYPAD_R, default1R | p1Xbox->RPressed);
		S9xReportButton(JOYPAD_UP, default1Up | p1Xbox->UpPressed);
		S9xReportButton(JOYPAD_DOWN, default1Down | p1Xbox->DownPressed);
		S9xReportButton(JOYPAD_LEFT, default1Left | p1Xbox->LeftPressed);
		S9xReportButton(JOYPAD_RIGHT, default1Right | p1Xbox->RightPressed);
				
		S9xReportButton(JOYPAD2_START, default2Start | p2Xbox->StartPressed);
		S9xReportButton(JOYPAD2_SELECT, default2Select | p2Xbox->SelectPressed);
		S9xReportButton(JOYPAD2_A, default2A | p2Xbox->APressed);
		S9xReportButton(JOYPAD2_B, default2B | p2Xbox->BPressed);
		S9xReportButton(JOYPAD2_X, default2X | p2Xbox->XPressed);
		S9xReportButton(JOYPAD2_Y, default2Y | p2Xbox->YPressed);
		S9xReportButton(JOYPAD2_L, default2L | p2Xbox->LPressed);
		S9xReportButton(JOYPAD2_R, default2R | p2Xbox->RPressed);
		S9xReportButton(JOYPAD2_UP, default2Up | p2Xbox->UpPressed);
		S9xReportButton(JOYPAD2_DOWN, default2Down | p2Xbox->DownPressed);
		S9xReportButton(JOYPAD2_LEFT, default2Left | p2Xbox->LeftPressed);
		S9xReportButton(JOYPAD2_RIGHT, default2Right | p2Xbox->RightPressed);


		// Speed
		bool toggledThisUpdate = false;
		if(!toggledThisUpdate)
		{
			if(!virtualState->TurboTogglePressed && oldvControllerState.TurboTogglePressed)
			{
				enableTurboMode = !enableTurboMode;
				toggledThisUpdate = true;
			}
		}
		if(!toggledThisUpdate)
		{
			if(!keyboardState->TurboTogglePressed && oldKeyboardState.TurboTogglePressed)
			{
				enableTurboMode = !enableTurboMode;
				toggledThisUpdate = true;
			}
		}
		if(!toggledThisUpdate)
		{
			if(!p1Xbox->TurboTogglePressed && oldControllerState1.TurboTogglePressed)
			{
				enableTurboMode = !enableTurboMode;
				toggledThisUpdate = true;
			}
		}
		if(!toggledThisUpdate)
		{
			if(!p2Xbox->TurboTogglePressed && oldControllerState2.TurboTogglePressed)
			{
				enableTurboMode = !enableTurboMode;
				toggledThisUpdate = true;
			}
		}

		if(enableTurboMode &&
			(oldKeyboardState.TurboPressed && !keyboardState->TurboPressed) ||
			(oldControllerState1.TurboPressed && !p1Xbox->TurboPressed) ||
			(oldControllerState2.TurboPressed && !p2Xbox->TurboPressed) ||
			(oldvControllerState.TurboPressed && !virtualState->TurboPressed))
		{
			enableTurboMode = false;
		}

		turboPressed = (keyboardState->TurboPressed || p1Xbox->TurboPressed || p2Xbox->TurboPressed || virtualState->TurboPressed);

		oldControllerState1 = *p1Xbox;
		oldControllerState2 = *p2Xbox;
		oldKeyboardState = *keyboardState;
		oldvControllerState = *virtualState;
	}
}

void S9xSoundCallback(void *data)
{	
	Emulator::EmulatorGame::Audio->ProcessSound();
}

bool8 S9xOpenSoundDevice (void)
{
	if(!Emulator::EmulatorGame::Audio->InitSoundOutput())
		return false;
	
	if(!Emulator::EmulatorGame::Audio->SetupSound())
		return false;

	S9xSetSamplesAvailableCallback(S9xSoundCallback, NULL);
	return true;
}

bool8 S9xDeinitUpdate (int Width, int Height)
{
	Emulator::EmulatorGame::SnesImageWidth = Width;
	Emulator::EmulatorGame::SnesImageHeight = Height;
	return true;
}
