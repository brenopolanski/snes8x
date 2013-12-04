//
// InputSettingsPane.xaml.h
// Declaration of the InputSettingsPane class
//

#pragma once

#include "InputSettingsPane.g.h"

namespace Win8Snes9x
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class InputSettingsPane sealed
	{
	private:
		EmulatorGame *emulator;
		bool initdone;
	public:
		InputSettingsPane();
		void BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	private:
		void UpdateTextBox(Windows::UI::Xaml::Controls::TextBox ^box, VirtualKey vk);
		void touchToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void keyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void leftKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void upKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void rightKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void downKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void bKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void aKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void yKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void xKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void lKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void rKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void startKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void selectKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void touchControlPosToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void assignControlToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void touchButtonStyleToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void buttonStyleToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void controllerScaleSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void controllerOpacitySlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void turboModeBox_KeyUp_1(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void dpadComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void deadzoneSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
	};
}
