﻿//
// SelectROMPane.xaml.h
// Declaration of the SelectROMPane class
//

#pragma once

#include "SelectROMPane.g.h"

namespace Win8Snes9x
{
	public delegate void ROMSelectedDelegate(StorageFile ^file, StorageFolder ^folder);

	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class StorageFolderModel sealed
	{
	private:
	public:
		property StorageFolder ^Folder;

		property Platform::String ^Name
		{
			Platform::String ^get()
			{
				return this->Folder->DisplayName;
			}
		}

		property Platform::String ^Path
		{
			Platform::String ^get()
			{
				return this->Folder->Path;
			}
		}

		StorageFolderModel(StorageFolder ^folder);
	};

	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class StorageFileModel sealed
	{
	private:
	public:
		property StorageFile ^File;
		property StorageFolder ^Folder;

		property Platform::String ^Name
		{
			Platform::String ^get()
			{
				return this->File->Name;
			}
		}

		property Platform::String ^Path
		{
			Platform::String ^get()
			{
				return this->Folder->Path;
			}
		}

		StorageFileModel(StorageFile ^file, StorageFolder ^folder);
	};

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SelectROMPane sealed
	{
	public:
		property ROMSelectedDelegate ^ROMSelected;
		property bool Cancelled;

		SelectROMPane();
	private:
		Platform::Collections::Vector<StorageFolderModel ^> ^storageFolderVector;
		Platform::Collections::Vector<StorageFileModel ^> ^storageFileVector;

		void InitializeStorageLists(void);
		void Close(void);
		void BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AddRomDirectoryClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DeleteFolderClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void RemoveFolderByToken(Platform::String ^token);
		void RefreshROMList(void);
		void romList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
	};
}
