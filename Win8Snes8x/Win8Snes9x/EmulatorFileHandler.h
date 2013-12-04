#pragma once

#include <ppltasks.h>
#include "Emulator.h"

using namespace concurrency;
using namespace Windows::Storage;

#define MAX_SAVESTATE_SLOTS 11
#define AUTOSAVESTATE_SLOT	10

namespace Emulator
{
	extern bool ROMLoaded;
	extern StorageFile ^ROMFile;
	extern StorageFolder ^ROMFolder;
	extern int SavestateSlot;
	
	bool IsROMLoaded(void);
	task<void> SaveStateAsync(void);
	task<void> LoadStateAsync(void);
	task<void> LoadROMAsync(StorageFile ^file, StorageFolder ^folder);
	task<void> ResetAsync(void);
	task<ROMData> GetROMBytesFromFileAsync(StorageFile ^file);
	task<void> SaveSRAMAsync(void);
	task<void> SaveSRAMCopyAsync(void);
	task<void> LoadSRAMAsync(void);
	task<void> RestoreFromApplicationDataAsync(void);
	task<void> SuspendAsync(void);

	task<void> SaveBytesToFileAsync(StorageFile ^file, unsigned char *bytes, size_t length);
	task<ROMData> GetBytesFromFileAsync(StorageFile ^file);

	void SelectSavestateSlot(int slot);
	int GetSavestateSlot(void);
}