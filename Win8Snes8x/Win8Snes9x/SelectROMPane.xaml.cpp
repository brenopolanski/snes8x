﻿//
// SelectROMPane.xaml.cpp
// Implementation of the SelectROMPane class
//

#include "pch.h"
#include "AboutSettingsPane.xaml.h"
#include "SelectROMPane.xaml.h"
#include <Windows.h>

using namespace Win8Snes9x;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Storage::AccessCache;

StorageFolderModel::StorageFolderModel(StorageFolder ^folder)
{ 
	this->Folder = folder;
}

StorageFileModel::StorageFileModel(StorageFile ^file, StorageFolder ^folder)
{
	this->File = file;
	this->Folder = folder;
}

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236
SelectROMPane::SelectROMPane()
{
	this->ROMSelected = nullptr;
	this->Cancelled = false;
	this->InitializeComponent();
	this->InitializeStorageLists();
}

void SelectROMPane::InitializeStorageLists(void)
{
	this->storageFileVector = ref new Platform::Collections::Vector<StorageFileModel ^>();
	this->storageFolderVector = ref new Platform::Collections::Vector<StorageFolderModel ^>();
	AccessListEntryView ^folderList = StorageApplicationPermissions::FutureAccessList->Entries;

	if(folderList->Size == 0)
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->romDirList->ItemsSource = this->storageFolderVector;
			this->RefreshROMList();
		}));
		return;
	}
	
	task<void> prevTask = create_task([folderList]()
	{
		return StorageApplicationPermissions::FutureAccessList->GetFolderAsync(folderList->GetAt(0).Token);
	}).then([this](StorageFolder ^folder)
	{
		this->storageFolderVector->Append(ref new StorageFolderModel(folder));
	}).then([this](task<void> t)
	{
		try
		{
			t.get();
		}catch(Platform::COMException ^ex)
		{
#if _DEBUG
			OutputDebugStringW(L"Folder in FutureAccessList does not exist anymore.");
#endif
		}
		catch(...)
		{
#if _DEBUG
			OutputDebugStringW(L"Folder in FutureAccessList does not exist anymore.");
#endif
		}
	});

	for (int i = 1; i < folderList->Size; i++)
	{
		prevTask = prevTask.then([this, folderList, i]()
		{
			return StorageApplicationPermissions::FutureAccessList->GetFolderAsync(folderList->GetAt(i).Token);
		}).then([this](StorageFolder ^folder)
		{
			this->storageFolderVector->Append(ref new StorageFolderModel(folder));
		}).then([this](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				OutputDebugStringW(L"Folder in FutureAccessList does not exist anymore.");
#endif
			}
			catch(...)
			{
#if _DEBUG
				OutputDebugStringW(L"Folder in FutureAccessList does not exist anymore.");
#endif
			}
		});
	}

	prevTask.then([this]()
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->romDirList->ItemsSource = this->storageFolderVector;
			this->RefreshROMList();
		}));
	});	
}

void Win8Snes9x::SelectROMPane::BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Cancelled = true;
	this->Close();
}

void SelectROMPane::Close()
{
	try
	{
		(safe_cast<Popup ^>(this->Parent))->IsOpen = false;
	}catch(InvalidCastException ^ex)
	{
#if _DEBUG
		Platform::String ^message = ex->Message;
		wstring wstr(message->Begin(), message->End());
		OutputDebugStringW(L"InvalidCastException");
#endif
	}
}

void Win8Snes9x::SelectROMPane::AddRomDirectoryClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FolderPicker ^picker = ref new FolderPicker();
	picker->FileTypeFilter->Append(".sfc");
	picker->FileTypeFilter->Append(".smc");
	picker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
	picker->ViewMode = PickerViewMode::List;
	picker->CommitButtonText = "Select ROM Directory";
	
	task<void> t = create_task(picker->PickSingleFolderAsync()).then([this](StorageFolder ^folder)
	{
		if(folder)
		{
			bool contained = StorageApplicationPermissions::FutureAccessList->ContainsItem(folder->DisplayName);
			StorageApplicationPermissions::FutureAccessList->AddOrReplace(folder->DisplayName, folder); 
			if(contained)
			{
				this->RemoveFolderByToken(folder->DisplayName);
			}
			this->storageFolderVector->Append(ref new StorageFolderModel(folder));
		}
	}).then([this]()
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->romDirList->ItemsSource = this->storageFolderVector;
			this->RefreshROMList();
		}));
	});
}

void Win8Snes9x::SelectROMPane::DeleteFolderClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button ^button = safe_cast<Button ^>(sender);
	StorageFolderModel ^model = safe_cast<StorageFolderModel ^>(button->DataContext);
	StorageApplicationPermissions::FutureAccessList->Remove(model->Name);
	this->RemoveFolderByToken(model->Name);
	this->romDirList->ItemsSource = this->storageFolderVector;
	this->RefreshROMList();
}

void SelectROMPane::RemoveFolderByToken(Platform::String ^token)
{
	for (int i = 0; i < this->storageFolderVector->Size; i++)
	{
		if(this->storageFolderVector->GetAt(i)->Name == token)
		{
			this->storageFolderVector->RemoveAt(i);
			break;
		}
	}
}

void SelectROMPane::RefreshROMList(void)
{
	this->storageFileVector->Clear();

	if(this->storageFolderVector->Size == 0)
	{
		StorageFolder ^localfolder = ApplicationData::Current->LocalFolder;
		create_task([localfolder]()
		{
			return localfolder->GetFileAsync("Airwolf.smc");
		}).then([this, localfolder](StorageFile ^file)
		{
			StorageFileModel ^model = ref new StorageFileModel(file, localfolder);
			this->storageFileVector->Append(model);
		}).then([this]()
		{
			this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
			{
				this->romList->ItemsSource = this->storageFileVector;
			}));
		}).then([](task<void> t)
		{
			try
			{
				t.get();
			}catch(...)
			{
	#if _DEBUG
				OutputDebugStringW(L"Cannot find demo ROM.");
	#endif
			}
		});
		return;
	}

	Search::QueryOptions ^options = ref new Search::QueryOptions();
	options->FileTypeFilter->Append(".sfc");
	options->FileTypeFilter->Append(".smc");		
	
	StorageFolder ^folder = this->storageFolderVector->GetAt(0)->Folder;
	task<void> t = create_task([this, folder, options]()
	{
		return folder->CreateFileQueryWithOptions(options)->GetFilesAsync();		
	}).then([this, folder](IVectorView<StorageFile ^> ^files)
	{
		for (int i = 0; i < files->Size; i++)
		{
			StorageFileModel ^model = ref new StorageFileModel(files->GetAt(i), folder);
			this->storageFileVector->Append(model);
		}
	});

	for (int i = 1; i < this->storageFolderVector->Size; i++)
	{
		StorageFolder ^folder = this->storageFolderVector->GetAt(i)->Folder;
		t = t.then([this, folder, options]()
		{
			return folder->CreateFileQueryWithOptions(options)->GetFilesAsync();		
		}).then([this, folder](IVectorView<StorageFile ^> ^files)
		{
			for (int i = 0; i < files->Size; i++)
			{
				StorageFileModel ^model = ref new StorageFileModel(files->GetAt(i), folder);
				this->storageFileVector->Append(model);
			}
		});
	}
	
	StorageFolder ^localfolder = ApplicationData::Current->LocalFolder;
	t = t.then([localfolder]()
	{
		return localfolder->GetFileAsync("Airwolf.smc");
	}).then([this, localfolder](StorageFile ^file)
	{
		StorageFileModel ^model = ref new StorageFileModel(file, localfolder);
		this->storageFileVector->Append(model);
	}).then([](task<void> t)
	{
		try
		{
			t.get();
		}catch(...)
		{
#if _DEBUG
			OutputDebugStringW(L"Cannot find demo ROM.");
#endif
		}
	});

	t.then([this]()
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->romList->ItemsSource = this->storageFileVector;
		}));
	});
}


void Win8Snes9x::SelectROMPane::romList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	StorageFileModel ^model =	safe_cast<StorageFileModel ^>(this->romList->SelectedValue);
	this->Close();
	if(this->ROMSelected)
	{
		this->ROMSelected(model->File, model->Folder);
	}
}
