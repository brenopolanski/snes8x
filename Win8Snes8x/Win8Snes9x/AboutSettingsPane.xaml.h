//
// HelpSettingsPane.xaml.h
// Declaration of the HelpSettingsPane class
//

#pragma once

#include "AboutSettingsPane.g.h"

namespace Win8Snes9x
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class AboutSettingsPane sealed
	{
	public:
		AboutSettingsPane();
		void BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
