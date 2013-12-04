//
// SelectStatePane.xaml.h
// Declaration of the SelectStatePane class
//

#pragma once

#include "SelectStatePane.g.h"

using namespace Windows::Foundation::Collections;

namespace Win8Snes9x
{
	public delegate void SavestateSelected(int slot);

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SelectStatePane sealed
	{
	public:
		property SavestateSelected ^SavestateSelected;

		SelectStatePane();
		SelectStatePane(int selectedSlot);
	private:
		IVector<Platform::String ^> ^savestateVector;

		void Init(void);

		void savestateList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
	};
}
