#include "../Ares.h"

DEFINE_HOOK(60FAD7, Ownerdraw_PostProcessColors, A) {
	// copy original instruction
	*reinterpret_cast<int*>(0xAC1B90) = 0x443716;

	// update colors
	*reinterpret_cast<int*>(0xAC18A4) = Ares::UISettings::uiColorText;
	*reinterpret_cast<int*>(0xAC184C) = Ares::UISettings::uiColorCaret;
	*reinterpret_cast<int*>(0xAC4604) = Ares::UISettings::uiColorSelection;
	*reinterpret_cast<int*>(0xAC1B98) = Ares::UISettings::uiColorBorder1;
	*reinterpret_cast<int*>(0xAC1B94) = Ares::UISettings::uiColorBorder2;
	*reinterpret_cast<int*>(0xAC1AF8) = Ares::UISettings::uiColorDisabledObserver;
	*reinterpret_cast<int*>(0xAC1CB0) = Ares::UISettings::uiColorTextObserver;
	*reinterpret_cast<int*>(0xAC4880) = Ares::UISettings::uiColorSelectionObserver;
	*reinterpret_cast<int*>(0xAC1CB4) = Ares::UISettings::uiColorDisabled;

	// skip initialization
	bool inited = *reinterpret_cast<bool*>(0xAC48D4);
	return inited ? 0x60FB5D : 0x60FAE3;
}

DEFINE_HOOK(612DA9, Handle_Button_Messages_Color, 6) {
	R->EDI(Ares::UISettings::uiColorTextButton);
	return 0x612DAF;
}

DEFINE_HOOK(613072, Handle_Button_Messages_DisabledColor, 7) {
	R->EDI(Ares::UISettings::uiColorDisabledButton);
	return 0x613138;
}

DEFINE_HOOK(61664C, Handle_Checkbox_Messages_Color, 5) {
	R->EAX(Ares::UISettings::uiColorTextCheckbox);
	return 0x616651;
}

DEFINE_HOOK(616655, Handle_Checkbox_Messages_Disabled, 5) {
	R->EAX(Ares::UISettings::uiColorDisabledCheckbox);
	return 0x61665A;
}

DEFINE_HOOK(616AF0, Handle_RadioButton_Messages_Color, 6) {
	R->ECX(Ares::UISettings::uiColorTextRadio);
	return 0x616AF6;
}

DEFINE_HOOK(615DF7, Handle_Static_Messages_Color, 6) {
	R->ECX(Ares::UISettings::uiColorTextLabel);
	return 0x615DFD;
}

DEFINE_HOOK(615AB7, Handle_Static_Messages_Disabled, 6) {
	R->ECX(Ares::UISettings::uiColorDisabledLabel);
	return 0x615ABD;
}

DEFINE_HOOK(619A4F, Handle_Listbox_Messages_Color, 6) {
	R->ESI(Ares::UISettings::uiColorTextList);
	return 0x619A55;
}

DEFINE_HOOK(6198D3, Handle_Listbox_Messages_DisabledA, 6) {
	R->EBX(Ares::UISettings::uiColorDisabledList);
	return 0x6198D9;
}

DEFINE_HOOK(619A5F, Handle_Listbox_Messages_DisabledB, 6) {
	R->ESI(Ares::UISettings::uiColorDisabledList);
	return 0x619A65;
}

DEFINE_HOOK(619270, Handle_Listbox_Messages_SelectionA, 5) {
	R->EAX(Ares::UISettings::uiColorSelectionList);
	return 0x619275;
}

DEFINE_HOOK(619288, Handle_Listbox_Messages_SelectionB, 6) {
	R->DL(BYTE(Ares::UISettings::uiColorSelectionList >> 16));
	return 0x61928E;
}

DEFINE_HOOK(617A2B, Handle_Combobox_Messages_Color, 6) {
	R->EBX(Ares::UISettings::uiColorTextCombobox);
	return 0x617A31;
}

DEFINE_HOOK(617A57, Handle_Combobox_Messages_Disabled, 6) {
	R->EBX(Ares::UISettings::uiColorDisabledCombobox);
	return 0x617A5D;
}

DEFINE_HOOK(60DDA6, Handle_Combobox_Dropdown_Messages_SelectionA, 5) {
	R->EAX(Ares::UISettings::uiColorSelectionCombobox);
	return 0x60DDAB;
}

DEFINE_HOOK(60DDB6, Handle_Combobox_Dropdown_Messages_SelectionB, 6) {
	R->DL(BYTE(Ares::UISettings::uiColorSelectionCombobox >> 16));
	return 0x60DDBC;
}

DEFINE_HOOK(61E2A5, Handle_Slider_Messages_Color, 5) {
	R->EAX(Ares::UISettings::uiColorTextSlider);
	return 0x61E2AA;
}

DEFINE_HOOK(61E2B1, Handle_Slider_Messages_Disabled, 5) {
	R->EAX(Ares::UISettings::uiColorDisabledSlider);
	return 0x61E2B6;
}

DEFINE_HOOK(61E8A0, Handle_GroupBox_Messages_Color, 6) {
	R->ECX(Ares::UISettings::uiColorTextGroupbox);
	return 0x61E8A6;
}

DEFINE_HOOK(614FF2, Handle_NewEdit_Messages_Color, 6) {
	R->EDX(Ares::UISettings::uiColorTextEdit);
	return 0x614FF8;
}
