//
// VideoSettingsPane.xaml.h
// Declaration of the VideoSettingsPane class
//

#pragma once

#include "VideoSettingsPane.g.h"

namespace Win8Snes9x
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class VideoSettingsPane sealed
	{
	private:
		EmulatorGame *emulator;
		bool initDone;
	public:
		VideoSettingsPane();
		void BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	private:
		void fpsToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		//void hzToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void skipComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void powerSkipComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void turboSkipComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void monitorComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
	};
}
