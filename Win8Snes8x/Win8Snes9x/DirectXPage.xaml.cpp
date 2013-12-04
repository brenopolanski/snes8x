//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage.xaml class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include "GeneralSettingsPane.xaml.h"
#include "VideoSettingsPane.xaml.h"
#include "AudioSettingsPane.xaml.h"
#include "AboutSettingsPane.xaml.h"
#include "InputSettingsPane.xaml.h"
#include "SelectROMPane.xaml.h"
#include "SnappedPane.xaml.h"
#include "SelectStatePane.xaml.h"
#include "EmulatorFileHandler.h"
#include "EmulatorSettings.h"
#include <windows.ui.xaml.media.dxinterop.h>
#include <wrl.h>

using namespace std;
using namespace Emulator;
using namespace Win8Snes9x;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Input;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;
using namespace Windows::UI::ViewManagement;

#define SETTINGS_WIDTH			346
#define GENERAL_SETTINGS_ID		"generalSettings"
#define INPUT_SETTINGS_ID		"inputSettings"
#define VIDEO_SETTINGS_ID		"videoSettings"
#define AUDIO_SETTINGS_ID		"audioSettings"
#define ABOUT_SETTINGS_ID		"aboutSettings"

extern bool enableTurboMode;

DirectXPage::DirectXPage()
{
	this->Init(true, false);
}

DirectXPage::DirectXPage(bool restore)
{
	this->Init(true, restore);
}

void DirectXPage::Init(bool showAppBar, bool restore)
{
	if(restore)
		showAppBar = false;

	InitializeComponent();

	this->settingsWidth = SETTINGS_WIDTH;
	this->windowBounds = Window::Current->Bounds;
	
	DisplayProperties::AutoRotationPreferences = 
		DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped;

	Window::Current->CoreWindow->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &DirectXPage::OnWindowSizeChanged);

	Window::Current->CoreWindow->PointerPressed +=
		ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &DirectXPage::OnPointerPressed);

	Window::Current->CoreWindow->PointerMoved +=
		ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &DirectXPage::OnPointerMoved);

	Window::Current->CoreWindow->PointerReleased +=
		ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &DirectXPage::OnPointerReleased);

	Window::Current->CoreWindow->Activated +=
		ref new TypedEventHandler<CoreWindow ^, WindowActivatedEventArgs ^>(this, &DirectXPage::OnCoreWindowActivated);

	PointerVisualizationSettings^ pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
	pointerVisualizationSettings->IsContactFeedbackEnabled = false;
	pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;


	this->EnableButtons(restore);
	
	m_eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &DirectXPage::OnRendering));
	
	// Register for settings panel
	SettingsPane::GetForCurrentView()->CommandsRequested +=	
		ref new TypedEventHandler<SettingsPane ^, SettingsPaneCommandsRequestedEventArgs ^>(this, &DirectXPage::onCommandRequest);
	
	auto settings = ApplicationData::Current->LocalSettings->Values;
	if(!settings->HasKey("FIRSTSTART"))
	{
		settings->Insert("FIRSTSTART", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(false)));	
		this->CopyDemoROM();
	}


	// Initialize emulator
	try
	{
		this->emulator = new EmulatorGame(restore);
		this->emulator->Initialize();
		this->renderer = ref new EmulatorRenderer();
		this->renderer->Initialize(this->SwapChainPanel);
	}catch(std::exception &e)
	{
#if _DEBUG
		string s(e.what());
		OutputDebugStringA(s.c_str());
#endif
	}

	CoreWindow::GetForCurrentThread()->VisibilityChanged += ref new TypedEventHandler<CoreWindow ^, VisibilityChangedEventArgs ^>(this, &DirectXPage::OnVisibilityChanged);

	this->BottomAppBar->IsOpen = showAppBar;

	/*this->startEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
	this->threadAction = ThreadPool::RunAsync(ref new WorkItemHandler([this](IAsyncAction ^action)
	{
		WaitForSingleObjectEx(this->startEvent, INFINITE, false);

		while(true)
		{
			this->renderer->Update();
			this->renderer->Render();
		}
	}), WorkItemPriority::High, WorkItemOptions::None);*/
}

DirectXPage::~DirectXPage(void)
{
	delete this->emulator;
	this->renderer = nullptr;
}

void DirectXPage::CopyDemoROM(void)
{
	StorageFolder ^installDir = Windows::ApplicationModel::Package::Current->InstalledLocation;
	create_task(installDir->GetFolderAsync("Assets/")).then([](task<StorageFolder ^> t)
	{
		StorageFolder ^assetsFolder = t.get();
		return assetsFolder->GetFileAsync("Airwolf.smc");
	}).then([](StorageFile ^file)
	{
		file->CopyAsync(ApplicationData::Current->LocalFolder);	
	}).then([](task<void> t)
	{
		try
		{
			t.get();
		}catch(Platform::Exception ^ex)
		{
#if _DEBUG
			Platform::String ^message = ex->Message;
			wstring wstr(message->Begin(), message->End());
			OutputDebugStringW(wstr.c_str());
#endif
		}
	});
}

void DirectXPage::OnPointerPressed(CoreWindow ^window, PointerEventArgs ^args)
{
	this->emulator->GetVirtualController()->PointerPressed(args->CurrentPoint);
}

void DirectXPage::OnPointerMoved(CoreWindow ^window, PointerEventArgs ^args)
{
	this->emulator->GetVirtualController()->PointerMoved(args->CurrentPoint);
}

void DirectXPage::OnPointerReleased(CoreWindow ^window, PointerEventArgs ^args)
{
	this->emulator->GetVirtualController()->PointerReleased(args->CurrentPoint);
}

void DirectXPage::OnVisibilityChanged(CoreWindow ^window, VisibilityChangedEventArgs ^args)
{
	if(args->Visible)
	{
		this->emulator->Unpause();
	}else
	{
		this->emulator->Pause();
	}
}

void DirectXPage::commandInvoked(IUICommand ^command)
{
	Platform::String ^id = safe_cast<Platform::String ^>(command->Id);
	this->settingsPopup = ref new Popup();
	this->settingsPopup->IsLightDismissEnabled = true;
	this->settingsPopup->Width = this->settingsWidth;
	this->settingsPopup->Height = this->windowBounds.Height;
	this->windowActivationToken = Window::Current->Activated += 
		ref new WindowActivatedEventHandler(this, &DirectXPage::OnWindowActivated);
	this->settingsPopup->Closed +=
		ref new EventHandler<Object ^>(this, &DirectXPage::OnPopupClosed);

	if(id == GENERAL_SETTINGS_ID)
	{
		GeneralSettingsPane ^pane = ref new GeneralSettingsPane();
		pane->Width = this->settingsWidth;
		pane->Height = this->windowBounds.Height;
		this->settingsPopup->Child = pane;
	}
	else if(id == VIDEO_SETTINGS_ID)
	{
		VideoSettingsPane ^pane = ref new VideoSettingsPane();
		pane->Width = this->settingsWidth;
		pane->Height = this->windowBounds.Height;
		this->settingsPopup->Child = pane;
	}
	else if(id == AUDIO_SETTINGS_ID)
	{
		AudioSettingsPane ^pane = ref new AudioSettingsPane();
		pane->Width = this->settingsWidth;
		pane->Height = this->windowBounds.Height;
		this->settingsPopup->Child = pane;
	}
	else if(id == ABOUT_SETTINGS_ID)
	{
		AboutSettingsPane ^pane = ref new AboutSettingsPane();
		pane->Width = this->settingsWidth;
		pane->Height = this->windowBounds.Height;
		this->settingsPopup->Child = pane;
	}else if(id == INPUT_SETTINGS_ID)
	{
		InputSettingsPane ^pane = ref new InputSettingsPane();
		pane->Width = this->settingsWidth;
		pane->Height = this->windowBounds.Height;
		this->settingsPopup->Child = pane;
	}
	this->settingsPopup->SetValue(Canvas::LeftProperty, this->windowBounds.Width - this->settingsWidth);
	this->settingsPopup->SetValue(Canvas::TopProperty, 0);	
	this->settingsPopup->IsOpen = true;
	this->emulator->Pause();
}

void DirectXPage::OnWindowActivated(Object ^sender, WindowActivatedEventArgs ^e)
{
	if(e->WindowActivationState == CoreWindowActivationState::Deactivated)
	{
		this->settingsPopup->IsOpen = false;
	}else
	{
		this->emulator->GetVirtualController()->Reset();
	}
}

void DirectXPage::OnCoreWindowActivated(CoreWindow ^sender, WindowActivatedEventArgs ^e)
{
	this->emulator->FocusChanged(e->WindowActivationState != CoreWindowActivationState::Deactivated);
}

void DirectXPage::OnPopupClosed(Object ^sender, Object ^args)
{
	Window::Current->Activated -= this->windowActivationToken;
	this->emulator->Unpause();
	this->emulator->GetVirtualController()->Reset();
}

void DirectXPage::onCommandRequest(SettingsPane ^settings, SettingsPaneCommandsRequestedEventArgs ^args)
{
	UICommandInvokedHandler^ handler = 
		ref new UICommandInvokedHandler(this, &DirectXPage::commandInvoked);

	SettingsCommand ^general = ref new SettingsCommand(GENERAL_SETTINGS_ID, "General", handler);
	args->Request->ApplicationCommands->Append(general);

	SettingsCommand ^input = ref new SettingsCommand(INPUT_SETTINGS_ID, "Input", handler);
	args->Request->ApplicationCommands->Append(input);

	SettingsCommand ^video = ref new SettingsCommand(VIDEO_SETTINGS_ID, "Video", handler);
	args->Request->ApplicationCommands->Append(video);

	SettingsCommand ^audio = ref new SettingsCommand(AUDIO_SETTINGS_ID, "Audio", handler);
	args->Request->ApplicationCommands->Append(audio);

	SettingsCommand ^about = ref new SettingsCommand(ABOUT_SETTINGS_ID, "About", handler);
	args->Request->ApplicationCommands->Append(about);
}

void DirectXPage::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	Rect bounds = sender->Bounds;
	bounds.Width *= ((int)DisplayProperties::ResolutionScale) / 100.0f;
	bounds.Height *=((int)DisplayProperties::ResolutionScale) / 100.0f;
	this->renderer->UpdateForWindowSizeChange();
	this->emulator->ResizeBuffer(bounds.Width, bounds.Height);
	this->windowBounds = bounds;

	if(ApplicationView::Value == ApplicationViewState::Snapped)
	{
		this->emulator->Pause();
		this->snappedPopup = ref new Popup();
		this->snappedPopup->IsLightDismissEnabled = false;
		this->snappedPopup->Height = this->windowBounds.Height;
		this->snappedPopup->Width = 346;

		SnappedPane ^pane = ref new SnappedPane();
		this->snappedPopup->Child = pane;
		pane->Width = this->snappedPopup->Width;
		pane->Height = this->snappedPopup->Height;
		
		this->snappedPopup->SetValue(Canvas::LeftProperty, 0);
		this->snappedPopup->SetValue(Canvas::TopProperty, 0);	
		this->snappedPopup->IsOpen = true;
	}else
	{
		this->emulator->Unpause();
		if(this->snappedPopup)
		{
			this->snappedPopup->IsOpen = false;
		}
	}
}

void DirectXPage::OnRendering(Object^ sender, Object^ args)
{
	//this->manager->SingleUpdate(this->emulator);

	/*if(!started)
	{
		SetEvent(this->startEvent);
		this->started = true;
	}*/
	this->renderer->Update();
	this->renderer->Render();


	/*this->emulator->Update();
	this->emulator->Draw();*/
}

void Win8Snes9x::DirectXPage::ResetButtonClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ResetAsync();
	this->emulator->GetVirtualController()->Reset();
}


void Win8Snes9x::DirectXPage::LoadROMButtonClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(this->loadingDialogOpen)
		return;

	Popup ^selectRomPopup = ref new Popup();
	selectRomPopup->Width = this->windowBounds.Width - 200;
	selectRomPopup->Height = this->windowBounds.Height - 200;
	selectRomPopup->IsLightDismissEnabled = false;

	SelectROMPane ^pane = ref new SelectROMPane();	
	pane->Width = this->windowBounds.Width - 200;
	pane->Height = this->windowBounds.Height - 200;
	selectRomPopup->Child = pane;

	selectRomPopup->Opened += ref new EventHandler<Object ^>([this](Object ^sender, Object ^args)
	{
		this->emulator->Pause();
	});
	selectRomPopup->Closed += ref new EventHandler<Object ^>([this, pane](Object ^sender, Object ^args)
	{
		if(pane->Cancelled)
		{
			this->emulator->Unpause();
		}
		this->loadingDialogOpen = false;
		this->emulator->GetVirtualController()->Reset();
	});

	pane->ROMSelected = ref new ROMSelectedDelegate([=](StorageFile ^file, StorageFolder ^folder)
	{
		LoadROMAsync(file, folder);
		this->BottomAppBar->IsOpen = false;
		this->loadingDialogOpen = false;
		this->EnableButtons(true);
	});

	selectRomPopup->SetValue(Canvas::LeftProperty, 100);
	selectRomPopup->SetValue(Canvas::TopProperty, 100);	
	selectRomPopup->IsOpen = true;
	this->loadingDialogOpen = true;
}

void DirectXPage::EnableButtons(bool enable)
{
	this->StopButton->IsEnabled = enable;
	this->ResetButton->IsEnabled = enable;
	this->PauseButton->IsEnabled = enable;
	this->TurboButton->IsEnabled = enable;
	this->SelectStateButton->IsEnabled = enable;
	this->LoadStateButton->IsEnabled = enable;
	this->SaveStateButton->IsEnabled = enable;
}


void Win8Snes9x::DirectXPage::FastForwardClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	enableTurboMode = !enableTurboMode;
	this->emulator->GetVirtualController()->Reset();
}


void Win8Snes9x::DirectXPage::PauseClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	static bool paused = false;
	if(paused)
	{
		paused = false;
		this->emulator->Unpause();
	}else
	{
		paused = true;
		this->emulator->Pause();
	}
	this->emulator->GetVirtualController()->Reset();
}

void Win8Snes9x::DirectXPage::StopClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->emulator->StopROMAsync().then([this]()
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->EnableButtons(false);
			this->emulator->GetVirtualController()->Reset();
		}));
	});
}

void DirectXPage::NotifyResume(void)
{
	//this->manager->SyncTime();
	this->renderer->GetGameTime()->SyncTime();
}

void DirectXPage::NotifySuspension(Windows::ApplicationModel::SuspendingDeferral ^deferral)
{
	SuspendAsync().then([deferral, this]()
	{
		deferral->Complete();
	});
}


void Win8Snes9x::DirectXPage::SelectStateButtonClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Popup ^statePopup = ref new Popup();
	statePopup->Width = 346;
	statePopup->MaxHeight = this->windowBounds.Height - 88;
	statePopup->IsLightDismissEnabled = true;

	SelectStatePane ^pane = ref new SelectStatePane(GetSavestateSlot());
	statePopup->Child = pane;
	pane->Width = statePopup->Width;
	pane->MaxHeight = statePopup->MaxHeight;
	pane->MaxHeight = statePopup->MaxHeight;

	pane->Measure(Windows::Foundation::Size(this->windowBounds.Width, this->windowBounds.Height));
	statePopup->SetValue(Canvas::LeftProperty, this->windowBounds.Width - statePopup->Width);
	statePopup->SetValue(Canvas::TopProperty, this->windowBounds.Height - (88 + pane->DesiredSize.Height));

	pane->SavestateSelected = ref new SavestateSelected([statePopup, this](int slot)
	{
		SelectSavestateSlot(slot);
		statePopup->IsOpen = false;
		this->emulator->GetVirtualController()->Reset();
	});

	statePopup->IsOpen = true;
}


void Win8Snes9x::DirectXPage::LoadStateButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(IsLoadConfirmationDisabled())
	{
		LoadStateAsync();
		this->emulator->GetVirtualController()->Reset();
		return;
	}

	MessageDialog ^dialog = ref new MessageDialog("Are you sure you want to load? All unsaved progress will be lost.", "Warning");
	UICommand ^confirm = ref new UICommand("Yes", 
		ref new UICommandInvokedHandler([this](IUICommand ^cmd)
	{
		LoadStateAsync();
		this->emulator->GetVirtualController()->Reset();
	}));

	UICommand ^confirmRemember = ref new UICommand("Yes, don't ask again",
		ref new UICommandInvokedHandler([this](IUICommand ^cmd)
	{
		DisableLoadConfirmation(true);
		LoadStateAsync();
		this->emulator->GetVirtualController()->Reset();
	}));

	UICommand ^no = ref new UICommand("No", 
		ref new UICommandInvokedHandler([this](IUICommand ^cmd)
	{ 
		this->emulator->Unpause();
	}));

	dialog->Commands->Append(confirm);
	dialog->Commands->Append(confirmRemember);
	dialog->Commands->Append(no);
	
	dialog->DefaultCommandIndex = 0;
	dialog->CancelCommandIndex = 2;

	this->emulator->Pause();
	dialog->ShowAsync();
}


void Win8Snes9x::DirectXPage::SaveStateButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{	
	if(IsSaveConfirmationDisabled())
	{
		SaveStateAsync();
		this->emulator->GetVirtualController()->Reset();
		return;
	}

	MessageDialog ^dialog = ref new MessageDialog("Are you sure you want to save? This will overwrite the selected save state.", "Warning");
	UICommand ^confirm = ref new UICommand("Yes", 
		ref new UICommandInvokedHandler([this](IUICommand ^cmd)
	{
		SaveStateAsync();
		this->emulator->GetVirtualController()->Reset();
	}));

	UICommand ^confirmRemember = ref new UICommand("Yes, don't ask again",
		ref new UICommandInvokedHandler([this](IUICommand ^cmd)
	{
		DisableSaveConfirmation(true);
		SaveStateAsync();
		this->emulator->GetVirtualController()->Reset();
	}));

	UICommand ^no = ref new UICommand("No", 
		ref new UICommandInvokedHandler([this](IUICommand ^cmd)
	{ 
		this->emulator->Unpause();
	}));

	dialog->Commands->Append(confirm);
	dialog->Commands->Append(confirmRemember);
	dialog->Commands->Append(no);
	
	dialog->DefaultCommandIndex = 0;
	dialog->CancelCommandIndex = 2;

	this->emulator->Pause();
	dialog->ShowAsync();
}
