#include "Actions.h"
#include "../Misc/Debug.h"
#include <HouseClass.h>
#include <TechnoClass.h>

#include <Helpers\Macro.h>

MouseCursor Actions::MP(0, 1, 0, 1, 1, MouseHotSpotX::Center, MouseHotSpotY::Middle);
MouseCursor* Actions::MPCurrent = nullptr;
MouseCursor* Actions::MPCustom = nullptr;
MouseCursor* Actions::TempCursor = nullptr;
bool Actions::MPCustomAllowShroud = true;

void Actions::Set(MouseCursor *pCursor, bool bAllowShroud)
{
	Actions::MPCustom = pCursor;
	Actions::MPCustomAllowShroud = bAllowShroud;
}

//4AB44A
DEFINE_HOOK(4AB44A, Actions_CustomCursor_NonShrouded, 9)
{
	MouseCursor* pCursor = Actions::MPCustom;
	if(pCursor) {
		GET(MouseClass*, pMouse, ESI);
		GET_STACK(bool, minimap, 0x34);

		//don't try this at home
		pMouse->SetCursor(reinterpret_cast<MouseCursorType&>(pCursor), minimap);

		return 0x4AB78F;
	}

	//overwrote the ja, need to replicate it
	unsigned int CursorIndex = R->EAX();
	return (CursorIndex > 0x47)
		? 0x4AB781
		: 0x4AB453
	;
}

//4AB366
DEFINE_HOOK(4AB366, Actions_CustomCursor_Shrouded, 9)
{
	MouseCursor* pCursor = Actions::MPCustom;
	if(pCursor) {
		if(Actions::MPCustomAllowShroud) {
			GET(MouseClass*, pMouse, ESI);
			GET_STACK(bool, minimap, 0x34);

			//don't try this at home
			pMouse->SetCursor(reinterpret_cast<MouseCursorType&>(pCursor), minimap);

			return 0x4AB78F;
		} else {
			return 0x4AB781;
		}
	}

	//overwrote the ja, need to replicate it
	unsigned int CursorIndex = R->EAX();
	return (CursorIndex > 0x48)
		? 0x4AB781
		: 0x4AB36F
	;
}

//5BDC8C, 7
DEFINE_HOOK(5BDC8C, Actions_PrepareCursor, 7)
{
	MouseCursor* pCursor;

	unsigned int CursorIndex = R->EAX();
	if(CursorIndex > MouseCursor::CursorCount) {
		//don't try this at home
		pCursor = reinterpret_cast<MouseCursor*>(CursorIndex);
	} else {
		pCursor = &MouseCursor::First[CursorIndex];
	}

	Actions::TempCursor = pCursor; //setting temp cursor for use in Actions_SetCursor!
	R->ESI(pCursor);
	R->ECX(pCursor->MiniFrame);

	return 0x5BDCA3;
}

//5BDD9F, 6
DEFINE_HOOK(5BDD9F, Actions_SetCursor, 6)
{
	Actions::MPCurrent = Actions::TempCursor; //got set in Actions_UseCustomCursor
	if(Actions::MPCurrent) {
		Actions::MP = *Actions::MPCurrent;
	}
	return 0;
}

//5BDADF, B
DEFINE_HOOK(5BDADF, Actions_UseCursor, 0)
{
	R->EBP(&Actions::MP);

	return R->DL()
		? 0x5BDAEC
		: 0x5BDAFA
	;
}

//5BDDC8, 6
DEFINE_HOOK(5BDDC8, Actions_AnimateCursor, 6)
{
	R->EBX(&Actions::MP);

	return (Actions::MP.Interval == 0)
		? 0x5BDF13  //no animation
		: 0x5BDDED
	;
}

//5BDE64, 6
DEFINE_HOOK(5BDE64, Actions_AnimateCursor2, 6)
{
	R->ECX(&Actions::MP);

	// TODO: MouseClass
	GET(byte *, pMouse, ESI);
	return (pMouse[0x555C])
		? 0x5BDE84 //minimap
		: 0x5BDE92
	;
}

//4D7524
DEFINE_HOOK(4D7524, Actions_AllowForFootClass, 9)
{
	//overwrote the ja, need to replicate it
	GET(Action, CursorIndex, EBP);
	if(CursorIndex == Action::None || CursorIndex > Action::Airstrike) {
		if(CursorIndex == Actions::SuperWeaponAllowed || CursorIndex == Actions::SuperWeaponDisallowed) {
			return 0x4D769F;
		} else {
			return 0x4D7CC0;
		}
	} else {
		return 0x4D752D;
	}
}

//653CA6
DEFINE_HOOK(653CA6, Actions_AllowMinimap, 5)
{
	MouseCursor* pCursor = Actions::MPCustom;
	if(pCursor) {
		if(pCursor->MiniFrame >= 0) {
			return 0x653CC0;
		} else {
			return 0x653CBA;
		}
	} else {
		unsigned int CursorIndex = R->EAX();
		//overwrote the ja, need to replicate it
		if(CursorIndex > 0x47) {
			return 0x653CBA;
		} else {
			return 0x653CAB;
		}
	}
}

//5BDDC0, 5
DEFINE_HOOK(5BDDC0, Actions_Reset, 5)
{
	Actions::MPCustom = nullptr;
	return 0;
}

//// ----------------------------------------
////   CUSTOM WEAPON CURSORS
//// ----------------------------------------
//
//// 417F4F, 0xA
//XPORT_FUNC(AircraftClass_GetCursorOverObject)
//{
//	// set Actions::CustomCursor to something
//	return 0;
///*
//	GET(AircraftClass *, Source, ESI);
//	GET(TechnoClass *, Target, EDI);
//
//	RET_UNLESS(Target);
//
//	int idxWeapon = Source->SelectWeapon(Target);
//	WeaponTypeClass *Weapon = Source->GetWeapon(idxWeapon)->WeaponType;
//
//	RET_UNLESS(Weapon->get_Warhead()->get_IvanBomb());
//
//	R->set_EBX(!Target->GetType()->get_Immune() && Target->GetType()->get_Bombable() && !Target->get_AttachedBomb()
//		? act_IvanBomb
//		: act_NoIvanBomb);
//	return 0x417F68;
//*/
//}
//
//// 51EB38, 6
//XPORT_FUNC(InfantryClass_GetCursorOverObject)
//{
//	// set Actions::CustomCursor to something
//	return 0;
///*
//	GET(InfantryClass *, Source, EDI);
//	GET(TechnoClass *, Target, ESI);
//
//	RET_UNLESS(Target);
//
//	int idxWeapon = Source->SelectWeapon(Target);
//	WeaponTypeClass *Weapon = Source->GetWeapon(idxWeapon)->WeaponType;
//
//	RET_UNLESS(Weapon->get_Warhead()->get_IvanBomb());
//
//	return 0x51EB48;
//*/
//}
//
//// 740490, 6
//XPORT_FUNC(UnitClass_GetCursorOverObject)
//{
//	// set Actions::CustomCursor to something
//	return 0;
///*
//	GET(UnitClass *, Source, ESI);
//	GET(TechnoClass *, Target, EDI);
//
//	int idxWeapon = Source->SelectWeapon(Target);
//	WeaponTypeClass *Weapon = Source->GetWeapon(idxWeapon)->WeaponType;
//
//	RET_UNLESS(Weapon->get_Warhead()->get_IvanBomb());
//
//	R->set_EBX(Target->GetType()->get_Bombable() && !Target->get_AttachedBomb()
//		? act_IvanBomb
//		: act_NoIvanBomb);
//	return 0;
//*/
//}
//
//// 447512, 5
//XPORT_FUNC(BuildingClass_GetCursorOverObject)
//{
//	// set Actions::CustomCursor to something
//	return 0;
///*
//	GET(BuildingClass *, Source, ESI);
//	GET(TechnoClass *, Target, EBP);
//
//	int idxWeapon = Source->SelectWeapon(Target);
//	WeaponTypeClass *Weapon = Source->GetWeapon(idxWeapon)->WeaponType;
//
//	if((Target->get_AbstractFlags() & ABSFLAGS_ISTECHNO) && Weapon->get_Warhead()->get_IvanBomb())
//	{
//		R->set_EBX(!Target->GetType()->get_Immune() && Target->GetType()->get_Bombable() && !Target->get_AttachedBomb()
//			? act_IvanBomb
//			: act_NoIvanBomb);
//	}
//
//	return 0x44752C;
//*/
//}

DEFINE_HOOK(6929FC, DisplayClass_ChooseAction_CanSell, 7)
{
	GET(TechnoClass *, Target, ESI);
	switch(Target->WhatAmI()) {
		case AbstractType::Aircraft:
		case AbstractType::Unit:
			R->Stack(0x10, Action::SellUnit);
			return 0x692B06;
		case AbstractType::Building:
			R->Stack(0x10, Target->IsStrange() ? Action::NoSell : Action::Sell);
			return 0x692B06;
		default:
			return 0x692AFE;
	}
}

DEFINE_HOOK(4ABFBE, DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7)
{
	GET(TechnoClass *, Target, ESI);
	return (Target && Target->Owner->ControlledByHuman() && Target->WhatAmI() == AbstractType::Building)
	 ? 0x4ABFCE
	 : 0x4AC294
	;
}
