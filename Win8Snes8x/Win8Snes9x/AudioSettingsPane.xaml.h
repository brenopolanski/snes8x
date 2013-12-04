//
// AudioSettingsPane.xaml.h
// Declaration of the AudioSettingsPane class
//

#pragma once

#include "AudioSettingsPane.g.h"

namespace Win8Snes9x
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class AudioSettingsPane sealed
	{
	private:
		bool initDone;
		EmulatorGame *emulator;
	public:
		AudioSettingsPane();
		void BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	private:
		void soundToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void soundSyncToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
