//
// GeneralSettingsPane.xaml.cpp
// Implementation of the GeneralSettingsPane class
//

#include "pch.h"
#include "GeneralSettingsPane.xaml.h"
#include "EmulatorSettings.h"

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

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

GeneralSettingsPane::GeneralSettingsPane()
	: emulator(EmulatorGame::GetInstance()), initdone(false)
{
	InitializeComponent();
	this->loadConfirmationToggle->IsOn = IsLoadConfirmationDisabled();
	this->saveConfirmationToggle->IsOn = IsSaveConfirmationDisabled();
	initdone = true;
}

void Win8Snes9x::GeneralSettingsPane::BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(this->Parent->GetType() == Popup::typeid)
	{
		(safe_cast<Popup ^>(this->Parent))->IsOpen = false;
	}
	SettingsPane::Show();
}


void Win8Snes9x::GeneralSettingsPane::saveConfirmationToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(this->initdone)
	{
		DisableSaveConfirmation(this->saveConfirmationToggle->IsOn);
	}
}


void Win8Snes9x::GeneralSettingsPane::loadConfirmationToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(this->initdone)
	{
		DisableLoadConfirmation(this->loadConfirmationToggle->IsOn);
	}
}
