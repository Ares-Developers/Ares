#include "Actions.h"

MouseCursor Actions::MP = {0, 1, 0, 1, 1, 0, 0};
MouseCursor* Actions::MPCurrent = NULL;
MouseCursor* Actions::MPCustom = NULL;
MouseCursor* Actions::TempCursor = NULL;
bool Actions::MPCustomAllowShroud = true;

void Actions::Set(MouseCursor *pCursor, bool bAllowShroud)
{
	Actions::MPCustom = pCursor;
	Actions::MPCustomAllowShroud = bAllowShroud;
}

void Actions::Set(MouseCursor *pCursor)
{
	Set(pCursor, true);
}

//4AB44A
EXPORT Actions_CustomCursor_NonShrouded(REGISTERS* R)
{
	MouseCursor* pCursor = Actions::MPCustom;
	if(pCursor)
	{
		//TODO: rewrite once MouseClass is defined!
		MouseClass* Mouse = (MouseClass*)R->get_ESI();

		//don't try this at home
		Mouse->QueryCursor((int)pCursor, R->get_StackVar32(0x34));

		return 0x4AB78F;
	}

	//overwrote the ja, need to replicate it
	unsigned int CursorIndex = R->get_EAX();
	if(CursorIndex > 0x47)
		return 0x4AB781;
	else
		return 0x4AB453;
}

//4AB366
EXPORT Actions_CustomCursor_Shrouded(REGISTERS* R)
{
	MouseCursor* pCursor = Actions::MPCustom;
	if(pCursor)
	{
		if(Actions::MPCustomAllowShroud)
		{
			//TODO: rewrite once MouseClass is defined!
			GScreenClass* Mouse = (GScreenClass*)R->get_ESI();

			//don't try this at home
			Mouse->QueryCursor((int)pCursor, R->get_StackVar32(0x34));

			return 0x4AB78F;
		}
		else return 0x4AB781;
	}

	//overwrote the ja, need to replicate it
	unsigned int CursorIndex = R->get_EAX();
	if(CursorIndex > 0x48)
		return 0x4AB781;
	else
		return 0x4AB36F;
}

//5BDC8C, 7
EXPORT Actions_PrepareCursor(REGISTERS* R)
{
	MouseCursor* pCursor;

	unsigned int CursorIndex = R->get_EAX();
	if(CursorIndex > 0x56) //no idea why it's 0x56...
	{
		//don't try this at home
		pCursor = (MouseCursor*)CursorIndex;
	}
	else
		pCursor = MouseCursor::First + CursorIndex;

	Actions::TempCursor = pCursor; //setting temp cursor for use in Actions_SetCursor!
	R->set_ESI((DWORD)pCursor);
	R->set_ECX(pCursor->MiniFrame);

	return 0x5BDCA3;
}

//5BDD9F, 6
EXPORT Actions_SetCursor(REGISTERS* R)
{
	Actions::MPCurrent = Actions::TempCursor; //got set in Actions_UseCustomCursor
	Actions::MP = *Actions::MPCurrent;
	return 0;
}

//5BDADF, 7
EXPORT Actions_UseCursor(REGISTERS* R)
{
	R->set_EBP((DWORD)&Actions::MP);

	bool bMini = (R->get_DL() != 0);

	if(bMini)
		return 0x5BDAEC;
	else
		return 0x5BDAFA;
}

//5BDDC8, 6
EXPORT Actions_AnimateCursor(REGISTERS* R)
{
	R->set_EBX((DWORD)&Actions::MP);

	if(Actions::MP.Interval == 0)
		return 0x5BDF13; //no animation
	else
		return 0x5BDDED;
}

//5BDE64, 6
EXPORT Actions_AnimateCursor2(REGISTERS* R)
{
	R->set_ECX((DWORD)&Actions::MP);

	bool* pMouse = (bool*)R->get_ESI();
	if(pMouse[0x555C])
		return 0x5BDE84; //minimap
	else
		return 0x5BDE92;
}

//4D7524
EXPORT Actions_AllowForFootClass(REGISTERS* R)
{
	//overwrote the ja, need to replicate it
	unsigned int CursorIndex = R->get_EAX();
	if(CursorIndex > 0x46)
	{
		if(CursorIndex == 0x7E || CursorIndex == 0x7D)
			return 0x4D769F;
		else
			return 0x4D7CC0;
	}
	else
		return 0x4D752D;
}

//653CA6
EXPORT Actions_AllowMinimap(REGISTERS* R)
{
	MouseCursor* pCursor = Actions::MPCustom;
	if(pCursor)
	{
		if(pCursor->MiniFrame >= 0)
			return 0x653CC0;
		else
			return 0x653CBA;
	}
	else
	{
		//overwrote the ja, need to replicate it
		unsigned int CursorIndex = R->get_EAX();
		if(CursorIndex > 0x47)
			return 0x653CBA;
		else
			return 0x653CAB;
	}
}

//5BDDC0, 5
EXPORT Actions_Reset(REGISTERS* R)
{
	Actions::MPCustom = NULL;
	return 0;
}

//DCoder's version
//void Actions::Set(MouseCursor *pCursor)
//{
//	if(pCursor != Actions::LastCustomCursor)
//	{
//		Actions::LastFrameIndex = 0;
//		Actions::LastTimerFrame = Unsorted::CurrentFrame;
//	}
//	Actions::CustomCursor = pCursor;
//};
//
//
//// 5BDDC8, 6
//// reset cursor
//EXPORT_FUNC(MouseClass_Update)
//{
//	if(Actions::CustomCursor)
//	{
//		Actions::LastCustomCursor = Actions::CustomCursor;
//	}
//	Actions::CustomCursor = NULL;
//	return 0;
//}
//
//// 5BDC8C, 7
//// reset cursor
//// EAX <= current Cursor index
//// ESI => &cursor
//EXPORT_FUNC(MouseClass_SetCursor)
//{
//	RET_UNLESS(Actions::CustomCursor);
//
//	MouseCursor *pCursor = Actions::CustomCursor;
//
//	if(pCursor->MiniFrame != -1)
//	{
//		R->set_BL(R->get_StackVar8(0x24));
//	}
//	else
//	{
//		R->set_StackVar8(0x24, 0);
//		R->set_BL(0);
//	}
//
//	R->set_EAX(0xFF); // Actions::LastCustomCursor == Actions::CustomCursor ? 0x7F : 0xFF); // DOESN'T WORK
//	R->set_ESI((DWORD)pCursor);
//
//	return 0x5BDCB4;
//}
//
//// 5BDD86, 5
//EXPORT_FUNC(MouseClass_SetCursor2)
//{
//	RET_UNLESS(Actions::CustomCursor);
//
//	MouseCursor *pCursor = Actions::CustomCursor;
//
//	if(Actions::LastTimerFrame - Unsorted::CurrentFrame > pCursor->Interval)
//	{
//		++Actions::LastFrameIndex;
//		Actions::LastFrameIndex %= pCursor->Count;
//		Actions::LastTimerFrame = Unsorted::CurrentFrame;
//	}
//
//	R->set_EDX(pCursor->Frame + Actions::LastFrameIndex);
//
//	return 0;
//}
//
//// ----------------------------------------
////   CUSTOM WEAPON CURSORS
//// ----------------------------------------
//
//// 417F4F, 0xA
//EXPORT_FUNC(AircraftClass_GetCursorOverObject)
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
//EXPORT_FUNC(InfantryClass_GetCursorOverObject)
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
//EXPORT_FUNC(UnitClass_GetCursorOverObject)
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
//EXPORT_FUNC(BuildingClass_GetCursorOverObject)
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
