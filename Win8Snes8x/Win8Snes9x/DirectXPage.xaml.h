//
// BlankPage.xaml.h
// Declaration of the BlankPage.xaml class.
//

#pragma once

#include "DirectXPage.g.h"
#include "Emulator.h"
#include "EmulatorRenderer.h"

using namespace Emulator;
//using namespace Windows::Foundation;
using namespace Windows::UI::ApplicationSettings;
using namespace Windows::UI::Popups;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::Storage;
using namespace Windows::UI::Input;
using namespace Platform::Collections;

namespace Win8Snes9x
{
	/// <summary>
	/// A DirectX page that can be used on its own.  Note that it may not be used within a Frame.
	/// </summary>
    [Windows::Foundation::Metadata::WebHostHidden]
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();
		DirectXPage(bool restore);
		virtual ~DirectXPage(void);

		void NotifyResume(void);
		void NotifySuspension(Windows::ApplicationModel::SuspendingDeferral ^deferral);
		void EnableButtons(bool enable);
	private:
		HANDLE startEvent;
		bool started;
		Windows::Foundation::IAsyncAction ^threadAction;
		Windows::Foundation::Rect windowBounds;
		uint32_t settingsWidth;
		Popup ^settingsPopup;
		Popup ^snappedPopup;
		bool loadingDialogOpen;
		Windows::Foundation::EventRegistrationToken windowActivationToken;

		EmulatorGame *emulator;
		EmulatorRenderer ^renderer;
		
		void CopyDemoROM(void);

		void Init(bool showAppBar, bool restore);

		void commandInvoked(IUICommand ^command);
		void onCommandRequest(SettingsPane ^settings, SettingsPaneCommandsRequestedEventArgs ^args);
		void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
		void OnRendering(Object^ sender, Object^ args);
		void OnWindowActivated(Object ^sender, WindowActivatedEventArgs ^e);
		void OnCoreWindowActivated(CoreWindow ^sender, WindowActivatedEventArgs ^e);
		void OnPopupClosed(Object ^sender, Object ^e);
		void OnVisibilityChanged(CoreWindow ^window, VisibilityChangedEventArgs ^args);

		void OnPointerPressed(CoreWindow ^window, PointerEventArgs ^args);
		void OnPointerMoved(CoreWindow ^window, PointerEventArgs ^args);
		void OnPointerReleased(CoreWindow ^window, PointerEventArgs ^args);

		Windows::Foundation::EventRegistrationToken m_eventToken;
		void ResetButtonClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void LoadROMButtonClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void FastForwardClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PauseClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void StopClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SelectStateButtonClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void LoadStateButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SaveStateButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
