#include <CCINIClass.h>
#include <TechnoTypeClass.h>
#include <WeaponTypeClass.h>
#include "Debug.h"

DEFINE_HOOK(477007, INIClass_GetSpeedType, 8)
{
	if(R->get_EAX() == -1) {
		GET_STACK(const char *, Section, 0x8C);
		LEA_STACK(const char *, Value, 0x8);
		GET_STACK(DWORD, caller, 0x88);
		/*
			this func is called from TechnoTypeClass::LoadFromINI and UnitTypeClass::LoadFromINI
			UnitTypeClass::CTOR initializes SpeedType to -1
			UnitTypeClass::LoadFromINI overrides it to (this->Crusher ? Track : Wheel) just before reading its SpeedType
			so we should not alert if we're responding to a TType read and our subject is a UnitType, or all VehicleTypes without an explicit ST declaration will get dinged
		*/
		if(caller != 0x7121E5 || reinterpret_cast<TechnoTypeClass *>(R->get_EBP())->WhatAmI() != abs_UnitType) {
			Debug::DevLog(Debug::Notice, "[%s]SpeedType=%s is not a valid value!\n", Section, Value);
		}
	}
	return 0;
}

DEFINE_HOOK(474E8E, INIClass_GetMovementZone, 5)
{
	if(R->get_EAX() == -1) {
		GET_STACK(const char *, Section, 0x2C);
		LEA_STACK(const char *, Value, 0x8);
//		if(_strcmpi(Value, "<none>")) {
			Debug::DevLog(Debug::Notice, "[%s]MovementZone=%s is not a valid value!\n", Section, Value);
//		}
	}
	return 0;
}

DEFINE_HOOK(47542A, INIClass_GetArmorType, 6)
{
	if(R->get_EAX() == -1) {
		GET_STACK(const char *, Section, 0x8C);
		LEA_STACK(const char *, Value, 0x8);
//		if(_strcmpi(Value, "<none>")) {
			Debug::DevLog(Debug::Notice, "[%s]Armor=%s is not a valid value!\n", Section, Value);
//		}
	}
	return 0;
}

DEFINE_HOOK(474DEE, INIClass_GetFoundation, 7)
{
	if(R->get_EAX() == -1) {
		GET_STACK(const char *, Section, 0x2C);
		LEA_STACK(const char *, Value, 0x8);
//		if(_strcmpi(Value, "<none>")) {
			Debug::DevLog(Debug::Notice, "[%s]Foundation=%s is not a valid value!\n", Section, Value);
//		}
	}
	return 0;
}

DEFINE_HOOK(687C16, INIClass_ReadScenario_ValidateThings, 6)
{
	/*
		all the INI files have been read
		add whatever code you need to validate that objects are in a valid state
		(e.g., to make sure crevio hasn't stuck a MovementZone=Retarded on anything)
		to reduce chances of crashing later
	*/
	for(int i = 0; i < TechnoTypeClass::Array->Count; ++i) {
		TechnoTypeClass *Item = reinterpret_cast<TechnoTypeClass *>(TechnoTypeClass::Array->Items[i]);
		if(ABS_IS_FOOT(Item) && Item->SpeedType == -1) {
			Debug::DevLog(Debug::Error, "[%s]SpeedType is invalid!\n", Item->get_ID());
		}

		if(ABS_IS_FOOT(Item) && Item->MovementZone == -1) {
			Debug::DevLog(Debug::Error, "[%s]MovementZone is invalid!\n", Item->get_ID());
		}

		if(Item->Armor == -1) {
			Debug::DevLog(Debug::Error, "[%s]Armor is invalid!\n", Item->get_ID());
		}
	}

	for(int i = 0; i < WeaponTypeClass::Array->Count; ++i) {
		WeaponTypeClass *Item = WeaponTypeClass::Array->Items[i];
		if(!Item->Warhead) {
			Debug::DevLog(Debug::Error, "Weapon[%s] has no Warhead! This usually indicates one of two things:\n"
				"- The weapon was created too late and its rules weren't read (see WEEDGUY hack);\n"
				"- The weapon's name was misspelled.\n"
			, Item->get_ID());
		}
	}

	return 0;
}
