﻿

//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------
#include "pch.h"
#include "AudioSettingsPane.xaml.h"




void ::Win8Snes9x::AudioSettingsPane::InitializeComponent()
{
    if (_contentLoaded)
        return;

    _contentLoaded = true;

    // Call LoadComponent on ms-appx:///AudioSettingsPane.xaml
    ::Windows::UI::Xaml::Application::LoadComponent(this, ref new ::Windows::Foundation::Uri(L"ms-appx:///AudioSettingsPane.xaml"), ::Windows::UI::Xaml::Controls::Primitives::ComponentResourceLocation::Application);

    // Get the ToggleSwitch named 'soundToggle'
    soundToggle = safe_cast<::Windows::UI::Xaml::Controls::ToggleSwitch^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"soundToggle"));
    // Get the ToggleSwitch named 'soundSyncToggle'
    soundSyncToggle = safe_cast<::Windows::UI::Xaml::Controls::ToggleSwitch^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"soundSyncToggle"));
}

void ::Win8Snes9x::AudioSettingsPane::Connect(int connectionId, Platform::Object^ target)
{
    switch (connectionId)
    {
    case 1:
        (safe_cast<::Windows::UI::Xaml::Controls::Primitives::ButtonBase^>(target))->Click +=
            ref new ::Windows::UI::Xaml::RoutedEventHandler(this, (void (::Win8Snes9x::AudioSettingsPane::*)(Platform::Object^, Windows::UI::Xaml::RoutedEventArgs^))&AudioSettingsPane::BackClick);
        break;
    case 2:
        (safe_cast<::Windows::UI::Xaml::Controls::ToggleSwitch^>(target))->Toggled +=
            ref new ::Windows::UI::Xaml::RoutedEventHandler(this, (void (::Win8Snes9x::AudioSettingsPane::*)(Platform::Object^, Windows::UI::Xaml::RoutedEventArgs^))&AudioSettingsPane::soundToggle_Toggled);
        break;
    case 3:
        (safe_cast<::Windows::UI::Xaml::Controls::ToggleSwitch^>(target))->Toggled +=
            ref new ::Windows::UI::Xaml::RoutedEventHandler(this, (void (::Win8Snes9x::AudioSettingsPane::*)(Platform::Object^, Windows::UI::Xaml::RoutedEventArgs^))&AudioSettingsPane::soundSyncToggle_Toggled_1);
        break;
    }
    (void)connectionId; // Unused parameter
    (void)target; // Unused parameter
    _contentLoaded = true;
}

