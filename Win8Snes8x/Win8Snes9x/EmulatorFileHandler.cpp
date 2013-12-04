#include "EmulatorFileHandler.h"
#include "EmulatorSettings.h"
#include "Emulator.h"
#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "display.h"
#include "apu/apu.h"
#include "controls.h"
#include "snapshot.h"
#include <string>
#include <sstream>

using namespace Platform;
using namespace std;

namespace Emulator
{
	bool ROMLoaded = false;
	StorageFile ^ROMFile = nullptr;
	StorageFolder ^ROMFolder = nullptr;
	int SavestateSlot = 0;

	bool IsROMLoaded(void)
	{
		return (ROMFile && ROMFolder && ROMLoaded);
	}

	task<void> LoadROMAsync(StorageFile ^file, StorageFolder ^folder)//(unsigned char *rom, size_t length)
	{		
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([emulator]()
		{
			emulator->StopROMAsync().wait();
		}).then([file]()
		{
			return GetBytesFromFileAsync(file);
		})
		.then([file, folder](ROMData data)
		{
			if(data.ROM && data.Length >= 0)
			{
				ROMFile = file;
				ROMFolder = folder;
				if(!Memory.LoadROM(data.ROM, data.Length))
				{
					ROMFile = nullptr;
					ROMFolder = nullptr;
				}
			}else
			{
				//EngineLog(LOG_LEVEL::Error, L"Unable to load ROM file.");
#if _DEBUG
				OutputDebugStringW(wstring(L"Unable to load ROM file.").c_str());
#endif
			}
			if(data.ROM)
			{
				delete [] data.ROM;
				data.ROM = nullptr;
			}
			return;
		}).then([]()
		{
			return LoadSRAMAsync();
		}).then([emulator]()
		{
			ROMLoaded = true;
			emulator->Unpause();	
		}).then([](task<void> t)
		{
			try
			{
				t.get();
			}catch(COMException ^ex)
			{
#if _DEBUG
				Platform::String ^str = ex->Message;
				wstring wstr(str->Begin(), str->End());
				OutputDebugStringW(wstr.c_str());
				//EngineLog(LOG_LEVEL::Error, wstr);
#endif
			}
		});
	}

	task<void> SaveSRAMAsync ()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - 4);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".srm");

		return create_task([sramName]()
		{
			// try to open the file			
			return ROMFolder->CreateFileAsync(sramName, CreationCollisionOption::OpenIfExists);
		})
		.then([](StorageFile ^file)
		{
			if (Settings.SuperFX && Memory.ROMType < 0x15) // doesn't have SRAM
				return;

			if (Settings.SA1 && Memory.ROMType == 0x34)    // doesn't have SRAM
				return;

			int		size;
			if (Multi.cartType && Multi.sramSizeB)
			{
				size = (1 << (Multi.sramSizeB + 3)) * 128;
				SaveBytesToFileAsync(file,Multi.sramB, size).wait();
			}

			size = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;
			if (size > 0x20000)
				size = 0x20000;

			if (size)
			{
				SaveBytesToFileAsync(file, Memory.SRAM, size).wait();

				if (Settings.SRTC || Settings.SPC7110RTC)
					Memory.SaveSRTC();
			}
		}).then([sramName](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<void> SaveSRAMCopyAsync ()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - 4);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".srm");

		EmulatorGame *emulator = EmulatorGame::GetInstance();


		if (Settings.SuperFX && Memory.ROMType < 0x15) // doesn't have SRAM
			return create_task([](){});

		if (Settings.SA1 && Memory.ROMType == 0x34)    // doesn't have SRAM
			return create_task([](){});
		
		emulator->Pause();

		int sizeCopy;
		int cartTypeCopy = Multi.cartType;
		int sramSizeBCopy = Multi.sramSizeB;
		uint8 sramSizeCopy = Memory.SRAMSize;
		uint8 *sramBCopy = nullptr;
		uint8 *sramCopy = nullptr;
		if (cartTypeCopy && sramSizeBCopy)
		{
			sizeCopy = (1 << (sramSizeBCopy + 3)) * 128;
			sramBCopy = new uint8[sizeCopy];

			memcpy_s(sramBCopy, sizeCopy, Multi.sramB, sizeCopy);
		}
		sizeCopy = sramSizeCopy ? (1 << (sramSizeCopy + 3)) * 128 : 0;
		if (sizeCopy > 0x20000)
			sizeCopy = 0x20000;

		if (sizeCopy)
		{
			sramCopy = new uint8[sizeCopy];
			memcpy_s(sramCopy, sizeCopy, Memory.SRAM, sizeCopy);
		}

		emulator->Unpause();

		auto sramBPtr = make_shared<uint8 *>(sramBCopy);
		auto sramPtr = make_shared<uint8 *>(sramCopy);

		return create_task([sramName]()
		{
			// try to open the file			
			return ROMFolder->CreateFileAsync(sramName, CreationCollisionOption::OpenIfExists);
		})
		.then([sramBPtr, sramPtr, cartTypeCopy, sramSizeBCopy, sramSizeCopy](StorageFile ^file)
		{
			if (Settings.SuperFX && Memory.ROMType < 0x15) // doesn't have SRAM
				return;

			if (Settings.SA1 && Memory.ROMType == 0x34)    // doesn't have SRAM
				return;

			int		size;
			if (cartTypeCopy && sramSizeBCopy && *sramBPtr)
			{
				size = (1 << (sramSizeBCopy + 3)) * 128;
				SaveBytesToFileAsync(file,*sramBPtr, size).wait();

				delete [] *sramBPtr;
				*sramBPtr = nullptr;
			}

			size = sramSizeCopy ? (1 << (sramSizeCopy + 3)) * 128 : 0;
			if (size > 0x20000)
				size = 0x20000;

			if (size && *sramPtr)
			{
				SaveBytesToFileAsync(file, *sramPtr, size).wait();

				delete [] *sramPtr;
				*sramPtr = nullptr;

				if (Settings.SRTC || Settings.SPC7110RTC)
					Memory.SaveSRTC();
			}

		}).then([sramName, sramBPtr, sramPtr](task<void> t)
		{
			try
			{
				if(*sramPtr)
				{
					delete [] *sramPtr;
					*sramPtr = nullptr;
				}
				if(*sramBPtr)
				{
					delete [] *sramBPtr;
					*sramBPtr = nullptr;
				}

				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<void> LoadSRAMAsync ()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^path = ROMFile->Path;
		Platform::String ^nameWithoutExt = ref new Platform::String(path->Begin(), path->Length() - 4);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".srm");

		return create_task([sramName]()
		{
			return StorageFile::GetFileFromPathAsync(sramName);
		})
		.then([](StorageFile ^file)
		{
			return GetBytesFromFileAsync(file);
		}).then([](ROMData data)
		{
			int		size, len;

			Memory.ClearSRAM();

			if (Multi.cartType && Multi.sramSizeB)
			{
				size = (1 << (Multi.sramSizeB + 3)) * 128;
				len = data.Length;
				for (int i = 0; i < len && i < 0x10000; i++)
				{
					Multi.sramB[i] = data.ROM[i];
					if (len - size == 512)
						memmove(Multi.sramB, Multi.sramB + 512, size);
				}
			}

			size = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;
			if (size > 0x20000)
				size = 0x20000;

			if (size)
			{
				len = data.Length;
				for (int i = 0; i < len && i < 0x20000; i++)
				{
					Memory.SRAM[i] = data.ROM[i];
				}
				if (len - size == 512)
					memmove(Memory.SRAM, Memory.SRAM + 512, size);

				if (Settings.SRTC || Settings.SPC7110RTC)
					Memory.LoadSRTC();
			}

			delete [] data.ROM;

		}).then([sramName](task<void> t)
		{
			try
			{
				t.get();
			}
			catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::AccessDeniedException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});	
	}

	task<void> ResetAsync(void)
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});
		return LoadROMAsync(ROMFile, ROMFolder);
	}

	task<void> SaveStateAsync(void)
	{
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([emulator]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Exception(E_FAIL, L"No ROM loaded.");
			}
			emulator->Pause();

			// Generate random file name to store in temp folder			
			Platform::String ^folderpath = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
			string folderPathStr(folderpath->Begin(), folderpath->End());

			stringstream tmpFileNameStream;
			tmpFileNameStream << folderPathStr << "\\";
			tmpFileNameStream << rand() << rand() << ".0";
			if(SavestateSlot >= 10)
			{
				tmpFileNameStream << SavestateSlot;
			}else
			{
				tmpFileNameStream << 0 << SavestateSlot;
			}
			string fileNameA = tmpFileNameStream.str();

			FILE *file;
			auto error = fopen_s(&file, fileNameA.c_str(), "wb");
			if(!file)
			{
				stringstream ss;
				ss << "Unable to open tmp file '";
				ss << fileNameA;
				ss << "' to store savestate (";
				ss << error;
				ss << ").";
				//throw GameException(ss.str().c_str());
				string str(ss.str());
				throw ref new Exception(E_FAIL, ref new Platform::String(wstring(str.begin(), str.end()).c_str()));
			}
			fclose(file);

			if(!S9xFreezeGame(fileNameA.c_str()))
			{
				stringstream ss;
				ss << "Unable to write savestate to tmp file '";
				ss << fileNameA;
				ss << ".";
				//throw GameException(ss.str().c_str());
				string str(ss.str());
				throw ref new Exception(E_FAIL, ref new Platform::String(wstring(str.begin(), str.end()).c_str()));
			}
			wstring wname(fileNameA.begin(), fileNameA.end());
			Platform::String ^str = ref new Platform::String(wname.c_str());

			return StorageFile::GetFileFromPathAsync(str); 
		}).then([](StorageFile ^file)
		{
			// Generate target file name and extension
			StorageFile ^romFile = ROMFile;
			Platform::String ^name = ref new Platform::String(romFile->Name->Begin(), romFile->Name->Length() - 4);
			Platform::String ^extension = file->FileType;
			Platform::String ^fullName = name + extension;
			return file->MoveAsync(ROMFolder, fullName, NameCollisionOption::ReplaceExisting);
		}).then([emulator](task<void> t)
		{
			try
			{
				emulator->Unpause();	
				t.get();
			}catch(Exception ^ex)
			{
#if _DEBUG
				Platform::String ^message = ex->Message;
				wstring str(message->Begin(), message->End());
				OutputDebugStringW((L"Save state: " + str).c_str());
#endif
			}
		});
	}

	task<void> LoadStateAsync(void)
	{
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([emulator]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Platform::Exception(E_FAIL, "No ROM loaded.");
			}
			emulator->Pause();
			wstringstream extension;
			extension << L".0";
			if(SavestateSlot >= 10)
			{
				extension << SavestateSlot;
			}else
			{
				extension << 0 << SavestateSlot;
			}
			Platform::String ^nameWithoutExtension = ref new Platform::String(ROMFile->Path->Data(), ROMFile->Path->Length() - 4);			
			Platform::String ^statePath = nameWithoutExtension + ref new Platform::String(extension.str().c_str());
			return StorageFile::GetFileFromPathAsync(statePath);
		}).then([](StorageFile ^file)
		{
			return file->CopyAsync(Windows::Storage::ApplicationData::Current->TemporaryFolder, file->Name, NameCollisionOption::ReplaceExisting);
		}).then([](StorageFile ^file)
		{
			Platform::String ^path = file->Path;
			wstring str = path->Data();
			string strA(str.begin(), str.end());
			S9xUnfreezeGame(strA.c_str());
			return file->DeleteAsync();
		}).then([](){}).then([emulator](task<void> t)
		{
			try
			{
				emulator->Unpause();
				t.get();
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				wstring err = ex->Message->Data();
				OutputDebugStringW(err.c_str());
				//EngineLog(LOG_LEVEL::Error, L"Load savestate: " + err);
#endif
			}
		});
	}
	

	task<void> SuspendAsync(void)
	{
		return create_task([]()
		{			
			ApplicationDataContainer ^localSettings = ApplicationData::Current->LocalSettings;

			if(!IsROMLoaded())
			{
				localSettings->Values->Remove("ROMPath");
				localSettings->Values->Remove("ROMFolderPath");
				localSettings->Values->Remove("SelectedSaveStateSlot");
				return;
			}
			int oldSlot = GetSavestateSlot();
			SelectSavestateSlot(AUTOSAVESTATE_SLOT);
			SaveStateAsync().wait();
			SelectSavestateSlot(oldSlot);			
			localSettings->Values->Insert("ROMPath", dynamic_cast<Windows::Foundation::PropertyValue ^>(Windows::Foundation::PropertyValue::CreateString(ROMFile->Path)));
			localSettings->Values->Insert("ROMFolderPath", dynamic_cast<Windows::Foundation::PropertyValue ^>(Windows::Foundation::PropertyValue::CreateString(ROMFolder->Path)));
			localSettings->Values->Insert("SelectedSaveStateSlot", dynamic_cast<Windows::Foundation::PropertyValue ^>(Windows::Foundation::PropertyValue::CreateInt32(GetSavestateSlot())));

			StoreSettings();
			SaveSRAMAsync().wait();
		});
	}

	task<void> RestoreFromApplicationDataAsync(void)
	{
		return create_task([]()
		{
			if(IsROMLoaded())
			{
				throw ref new Platform::Exception(E_FAIL, L"Unable to restore state because a ROM is already loaded.");
			}

			ApplicationDataContainer ^localSettings = ApplicationData::Current->LocalSettings;
			auto values = localSettings->Values;

			Platform::String ^romPath = safe_cast<Platform::String ^>(values->Lookup("ROMPath"));
			Platform::String ^romFolderPath = safe_cast<Platform::String ^>(values->Lookup("ROMFolderPath"));
			auto savestateSlotEntry = safe_cast<Windows::Foundation::IPropertyValue^>(values->Lookup("SelectedSaveStateSlot"));
			int savestateSlot = 0;

			if(savestateSlotEntry)
			{
				savestateSlot = savestateSlotEntry->GetInt32();
			}

			if(!romPath || !romFolderPath)
			{
				throw ref new Platform::Exception(E_FAIL, L"Unable to restore state after termination.");
			}

			StorageFile ^romfile = nullptr;
			StorageFolder ^romfolder = nullptr;
			create_task(StorageFile::GetFileFromPathAsync(romPath)).then([&romfile, romFolderPath](StorageFile ^file)
			{
				romfile = file;
				return StorageFolder::GetFolderFromPathAsync(romFolderPath);
			}).then([&romfolder](StorageFolder ^folder)
			{
				romfolder = folder;
			}).wait();

			LoadROMAsync(romfile, romfolder).wait();
			SelectSavestateSlot(AUTOSAVESTATE_SLOT);
			LoadStateAsync().wait();
			SelectSavestateSlot(savestateSlot);
			RestoreSettings();
		}).then([](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				Platform::String ^str = ex->Message;
				std::wstring wstr(str->Begin(), str->End());
				OutputDebugStringW(wstr.c_str());
#endif
			}
		});
		return task<void>();
	}

	void SelectSavestateSlot(int slot)
	{
		SavestateSlot = slot % MAX_SAVESTATE_SLOTS;
	}

	int GetSavestateSlot(void)
	{
		return SavestateSlot;
	}

	task<void> SaveBytesToFileAsync(StorageFile ^file, unsigned char *bytes, size_t length)
	{
		Platform::String ^name = file->Name;

		return create_task([file]()
		{
			return file->OpenAsync(FileAccessMode::ReadWrite);
		}).then([=](IRandomAccessStream ^stream)
		{
			IOutputStream ^outputStream = stream->GetOutputStreamAt(0L);;
			DataWriter ^writer = ref new DataWriter(outputStream);
			
			Platform::Array<unsigned char> ^array = ref new Array<unsigned char>(length);
			memcpy(array->Data, bytes, length);

			writer->WriteBytes(array);
			create_task(writer->StoreAsync()).wait();
			writer->DetachStream();
			return create_task(outputStream->FlushAsync());
		}).then([name](bool b)
		{
			if(!b)
			{
#if _DEBUG
				wstring wname(name->Begin(), name->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": Error while writing to the file.");
				OutputDebugStringW((wname + L": Error while writing to the file.").c_str());
#endif
			}
		}).then([name](task<void> t)
		{
			try
			{
				t.get();
			}catch(COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(name->Begin(), name->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": " + werror);

				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<ROMData> GetBytesFromFileAsync(StorageFile ^file)
	{
		auto inputStream = make_shared<IInputStream ^>();
		auto openTask = create_task(file->OpenSequentialReadAsync());
		
		return openTask.then([=] (IInputStream ^stream)
		{ 
			*inputStream = stream;
			return file->GetBasicPropertiesAsync();
		}).then([=](BasicProperties ^properties)
		{
			Buffer ^buffer = ref new Buffer(properties->Size);
			return (*inputStream)->ReadAsync(buffer, properties->Size, InputStreamOptions::None);
		})
		.then([=](IBuffer ^buffer)
		{			
			DataReader ^reader = DataReader::FromBuffer(buffer);
			Array<BYTE> ^bytes = ref new Array<BYTE>(buffer->Length);
			reader->ReadBytes(bytes);
			BYTE *rawBytes = new BYTE[buffer->Length];
			for (int i = 0; i < buffer->Length; i++)
			{
				rawBytes[i] = bytes[i]; 
			}
			                                                                                                                                                                                                             
			ROMData data;
			data.Length = buffer->Length;
			data.ROM = rawBytes;

			return data;
		});
	}
}