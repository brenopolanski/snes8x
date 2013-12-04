//
// InputSettingsPane.xaml.cpp
// Implementation of the InputSettingsPane class
//

#include "pch.h"
#include "InputSettingsPane.xaml.h"
#include "EmulatorSettings.h"
#include <string>
#include <sstream>

using namespace std;

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

InputSettingsPane::InputSettingsPane()
	: initdone(false), emulator(EmulatorGame::GetInstance())
{
	InitializeComponent();
	this->touchToggle->IsOn = TouchControlsEnabled();
	this->touchButtonStyleToggle->IsOn = TouchButtonsIntersecting();
	this->touchControlPosToggle->IsOn = IsTouchControllerOnTop();
	this->assignControlToggle->IsOn = AssignedToPlayer2();
	this->buttonStyleToggle->IsOn = ButtonsGray();
	this->UpdateTextBox(this->leftKeyBox, GetLeftKeyBinding());
	this->UpdateTextBox(this->rightKeyBox, GetRightKeyBinding());
	this->UpdateTextBox(this->upKeyBox, GetUpKeyBinding());
	this->UpdateTextBox(this->downKeyBox, GetDownKeyBinding());
	this->UpdateTextBox(this->startKeyBox, GetStartKeyBinding());
	this->UpdateTextBox(this->selectKeyBox, GetSelectKeyBinding());
	this->UpdateTextBox(this->aKeyBox, GetAKeyBinding());
	this->UpdateTextBox(this->bKeyBox, GetBKeyBinding());
	this->UpdateTextBox(this->xKeyBox, GetXKeyBinding());
	this->UpdateTextBox(this->yKeyBox, GetYKeyBinding());
	this->UpdateTextBox(this->lKeyBox, GetLKeyBinding());
	this->UpdateTextBox(this->rKeyBox, GetRKeyBinding());
	this->UpdateTextBox(this->turboModeBox, GetTurboKeyBinding());
	this->controllerScaleSlider->Value = (double) GetControllerScaling();
	this->controllerOpacitySlider->Value = (double) GetControllerOpacity();
	this->deadzoneSlider->Value = (double) GetDeadzone();
	this->dpadComboBox->SelectedIndex = GetDPadStyle();
	initdone = true;
}

void Win8Snes9x::InputSettingsPane::BackClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(this->Parent->GetType() == Popup::typeid)
	{
		(safe_cast<Popup ^>(this->Parent))->IsOpen = false;
	}
	SettingsPane::Show();
}

void Win8Snes9x::InputSettingsPane::touchToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(initdone)
	{
		EnableTouchControls(this->touchToggle->IsOn);
	}
	this->touchControlPosToggle->IsEnabled = this->touchToggle->IsOn;
	this->touchButtonStyleToggle->IsEnabled = this->touchToggle->IsOn;
	this->buttonStyleToggle->IsEnabled = this->touchToggle->IsOn;
	this->controllerScaleSlider->IsEnabled = this->touchToggle->IsOn;
	this->controllerOpacitySlider->IsEnabled = this->touchToggle->IsOn;
	this->deadzoneSlider->IsEnabled = this->touchToggle->IsOn;
	this->dpadComboBox->IsEnabled = this->touchToggle->IsOn;
}

void InputSettingsPane::UpdateTextBox(Windows::UI::Xaml::Controls::TextBox ^box, VirtualKey vk)
{
	auto s = vk.ToString();
	if(s->Length() >= 20)
	{
		s = "Unknown";
	}

	box->Text = s;
}


void Win8Snes9x::InputSettingsPane::keyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	e->Handled = true;
}

void Win8Snes9x::InputSettingsPane::leftKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->leftKeyBox, e->Key);
	SetLeftKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::upKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->upKeyBox, e->Key);
	SetUpKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::rightKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->rightKeyBox, e->Key);
	SetRightKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::downKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->downKeyBox, e->Key);
	SetDownKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::bKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->bKeyBox, e->Key);
	SetBKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::aKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->aKeyBox, e->Key);
	SetAKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::yKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->yKeyBox, e->Key);
	SetYKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::xKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->xKeyBox, e->Key);
	SetXKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::lKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->lKeyBox, e->Key);
	SetLKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::rKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->rKeyBox, e->Key);
	SetRKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::startKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->startKeyBox, e->Key);
	SetStartKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::selectKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->selectKeyBox, e->Key);
	SetSelectKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::turboModeBox_KeyUp_1(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->turboModeBox, e->Key);
	SetTurboKeyBinding(e->Key);
}


void Win8Snes9x::InputSettingsPane::touchControlPosToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(initdone)
	{
		SetTouchControllerOnTop(this->touchControlPosToggle->IsOn);
	}
}


void Win8Snes9x::InputSettingsPane::assignControlToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(initdone)
	{
		AssignToPlayer2(this->assignControlToggle->IsOn);
	}
}


void Win8Snes9x::InputSettingsPane::touchButtonStyleToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(initdone)
	{
		SetTouchButtonsIntersecting(this->touchButtonStyleToggle->IsOn);
	}
}


void Win8Snes9x::InputSettingsPane::buttonStyleToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(initdone)
	{
		SetButtonsGray(this->buttonStyleToggle->IsOn);
	}
}


void Win8Snes9x::InputSettingsPane::controllerScaleSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if(this->valueLabel)
	{
		wstringstream wss;
		wss << (int) e->NewValue;

		this->valueLabel->Text = ref new Platform::String(wss.str().c_str());

		if(initdone)
		{
			SetControllerScaling((int) e->NewValue);
		}
	}
}


void Win8Snes9x::InputSettingsPane::controllerOpacitySlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if(this->opacityValueLabel)
	{
		wstringstream wss;
		wss << (int) e->NewValue;

		this->opacityValueLabel->Text = ref new Platform::String(wss.str().c_str());

		if(initdone)
		{
			SetControllerOpacity((int) e->NewValue);
		}
	}
}


void Win8Snes9x::InputSettingsPane::dpadComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if(this->initdone)
	{
		SetDPadStyle(this->dpadComboBox->SelectedIndex);
	}
}


void Win8Snes9x::InputSettingsPane::deadzoneSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if(this->deadzoneLabel)
	{
		wstringstream wss;
		wss << (((int)(e->NewValue * 10.0)) / 10.0f);

		this->deadzoneLabel->Text = ref new Platform::String(wss.str().c_str());

		if(initdone)
		{
			SetDeadzone((float) e->NewValue);
		}
	}
}
