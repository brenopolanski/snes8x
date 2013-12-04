//
// AudioSettingsPane.xaml.cpp
// Implementation of the AudioSettingsPane class
//

#include "pch.h"
#include "AudioSettingsPane.xaml.h"
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

AudioSettingsPane::AudioSettingsPane()
	: emulator(EmulatorGame::GetInstance()), initDone(false)
{
	InitializeComponent();
	this->soundToggle->IsOn = SoundEnabled();
	this->soundSyncToggle->IsOn = SynchronizeAudio();

	initDone = true;
}

void Win8Snes9x::AudioSettingsPane::BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(this->Parent->GetType() == Popup::typeid)
	{
		(safe_cast<Popup ^>(this->Parent))->IsOpen = false;
	}
	SettingsPane::Show();
}

void Win8Snes9x::AudioSettingsPane::soundToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(initDone)
	{
		EnableSound(this->soundToggle->IsOn);
	}
}


void Win8Snes9x::AudioSettingsPane::soundSyncToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(initDone)
	{
		SetSynchronizeAudio(this->soundSyncToggle->IsOn);
	}
}
