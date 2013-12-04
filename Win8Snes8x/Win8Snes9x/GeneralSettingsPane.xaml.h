//
// GeneralSettingsPane.xaml.h
// Declaration of the GeneralSettingsPane class
//

#pragma once

#include "GeneralSettingsPane.g.h"

namespace Win8Snes9x
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class GeneralSettingsPane sealed
	{
	public:
		GeneralSettingsPane();
	private:
		bool initdone;
		EmulatorGame *emulator;
		void BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void saveConfirmationToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void loadConfirmationToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
