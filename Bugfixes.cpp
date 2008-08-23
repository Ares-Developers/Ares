#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include <YRPP.h>

// bugfix #379: Temporal friendly kills give veterancy
// 71A92A, 5
EXPORT_FUNC(_Temporal_AvoidFriendlies)
{
	GET(TemporalClass *, Temp, ESI); 

	HouseClass *hv = Temp->get_TargetUnit()->get_Owner();
	HouseClass *ho = Temp->get_OwningUnit()->get_Owner();

	RET_UNLESS(ho->IsAlliedWith(hv));
	return 0x71A97D;
}

// bugfix #385: Only InfantryTypes can use Ivan Bombs
// 438E86, 5
EXPORT_FUNC(IvanBombs_AttachableByAll)
{
	GET(TechnoClass *, Source, EBP);
	switch(Source->WhatAmI())
	{
		case abs_Aircraft:
		case abs_Building:
		case abs_Infantry:
		case abs_Unit:
			return 0x438E97;
		default:
			return 0x439022;
	}
}

// 469393, 7
EXPORT_FUNC(IvanBombs_Spread)
{
	GET(BulletClass *, bullet, ESI);
	double cSpread = bullet->get_WH()->get_CellSpread();

	RET_UNLESS(bullet->get_Target());
	
	CoordStruct tgtLoc = *(bullet->get_Target()->get_Location());
	TechnoClass *thOwner = (TechnoClass *)bullet->get_Owner();

	// just real target
	if(cSpread < 0.5)
	{
		BombListClass::Global()->Plant(thOwner, (TechnoClass *)bullet->get_Target());
		return 0;
	}

	int Spread = int(cSpread);

	int countCells = CellSpread::NumCells(Spread);
	for(int i = 0; i < countCells; ++i)
	{
		CellStruct tmpCell = CellSpread::GetCell(i);
		CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);
		for(ObjectClass *curObj = c->get_FirstObject(); curObj; curObj = curObj->get_NextObject())
		{
			if(!curObj->get_AttachedBomb())
			{
				BombListClass::Global()->Plant(thOwner, (TechnoClass *)curObj);
			}
		}
	}

/*
	for(int i = 0; i < Unsorted::vec_ObjectsInLayers[2]->get_Capacity(); ++i)
	{
		ObjectClass *curObj = Unsorted::vec_ObjectsInLayers[2]->GetItem(i);
		if(!(curObj->get_AbstractFlags() & ABSFLAGS_ISTECHNO))
		{
			continue;
		}
		if(curObj->get_Location()->DistanceFrom(tgtLoc) <= cSpread && !curObj->get_AttachedBomb())
		{
			BombListClass::Global()->Plant(thOwner, (TechnoClass *)curObj);
		}
	}
*/

	return 0x469AA4;
}

// Insignificant=yes or DontScore=yes prevent EVA_UnitLost on unit destruction
// 4D98DD, 6
EXPORT_FUNC(Insignificant_UnitLost)
{
	GET(TechnoClass *, t, ESI);
	TechnoTypeClass *T = (TechnoTypeClass *)t->GetType(); //R->get_EAX(); would work, but let's see if this does as well

	return (T->get_Insignificant() || T->get_DontScore()) ? 0x4D9916 : 0;
}

// bugfix #277: VeteranInfantry and friends don't show promoted cameos
// 71204C, 6
EXPORT_FUNC(TechnoTypeClass_GetCameo)
{
	GET(TechnoTypeClass *, T, ESI);
	HouseTypeClass *Country = ((HouseClass *)R->get_EAX())->get_Type();

	TypeList<TechnoTypeClass*>* vec_Promoted;

	TechnoTypeClass *Item = T;

	switch(T->WhatAmI())
	{
		case abs_InfantryType:
			vec_Promoted = (TypeList<TechnoTypeClass*>*)Country->get_VeteranInfantry();
			break;
/*
			if(Country->get_VeteranInfantry()->FindItemIndex((InfantryTypeClass *)T) == -1) // wth doesn't work
			{
				return 0;
			}
*/
		case abs_UnitType:
			vec_Promoted = (TypeList<TechnoTypeClass*>*)Country->get_VeteranUnits();
			break;
		case abs_AircraftType:
			vec_Promoted = (TypeList<TechnoTypeClass*>*)Country->get_VeteranAircraft();
			break;
		case abs_BuildingType:
			Item = T->get_UndeploysInto();
			if(Item)
			{
				vec_Promoted = (TypeList<TechnoTypeClass*>*)Country->get_VeteranUnits();
				break;
			}
		default:
			return 0;
	}

	SHPStruct *Alt = T->get_AltCameo();
	RET_UNLESS(Alt);

	for(int i = 0; i < vec_Promoted->get_Count(); ++i) {
		if(vec_Promoted->GetItem(i) == Item) {
			R->set_EAX((DWORD)Alt);
			return 0x7120C5;
		}
	}
	return 0;
}

// Naval=yes units show promoted cameos but don't actually get promoted
// for now, show unpromoted cameos
// someday, make em actually promoted (@ 0x735657)
// 71204C, 6
EXPORT_FUNC(TechnoTypeClass_GetCameo2)
{
	GET(TechnoTypeClass *, T, ESI);
	return T->get_Naval() ? 0x7120BF : 0x7120C5;
}

// bugfix #297: Crewed=yes jumpjets spawn parachuted infantry on destruction, not idle
// 7382AB, 6
EXPORT_FUNC(UnitClass_ReceiveDamage)
{
	GET(TechnoClass *, t, ESI);
	GET(InfantryClass *, I, EDI);

	CoordStruct loc = *t->get_Location();
	R->set_EAX(I->SpawnParachuted(&loc));

	// replacing Put() with this call, let's see if that works
	return 0x7382E2;
}

// MakeInfantry that fails to place will just end the source animation and cleanup instead of memleaking to game end
// 424B23, 6
EXPORT_FUNC(AnimClass_Update)
{
	GET(InfantryClass *, I, EDI);
	delete I;
	GET(AnimClass *, A, ESI);
	A->set_TimeToDie(1);
	A->UnInit();
	return 0x424B29;
}

// bugfix #297: Crewed=yes AircraftTypes spawn parachuting infantry on death
// 41668B, 6
EXPORT_FUNC(AircraftClass_ReceiveDamage)
{
	GET(AircraftClass *, A, ESI);
	RET_UNLESS(A->get_Type()->get_Crewed());

	CoordStruct loc = *A->get_Location();
	loc.Z += 64;

	InfantryTypeClass *PilotType = A->GetCrew();
	RET_UNLESS(!PilotType);

	InfantryClass *Pilot = new InfantryClass(PilotType, A->get_Owner());

	Pilot->set_Health(PilotType->get_Strength() >> 1);
	Pilot->get_Veterancy()->Veterancy = A->get_Veterancy()->Veterancy;
	if(Pilot->SpawnParachuted(&loc))
	{
		Pilot->Scatter(0xB1CFE8, 1, 0);
		Pilot->QueueMission(Pilot->get_Owner()->ControlledByHuman() ? mission_Guard : mission_Hunt, 0);
		if(A->get_IsSelected())
		{
			Pilot->Select();
		}
		// TODO: Tag
	}
	else
	{
		delete Pilot;
	}

	return 0;
}

// Ivan bomb cursors
// 417F4F, 0xA
EXPORT_FUNC(AircraftClass_GetCursorOverObject)
{
	GET(AircraftClass *, Source, ESI);
	GET(TechnoClass *, Target, EDI);

	RET_UNLESS(Target);

	int idxWeapon = Source->SelectWeapon(Target);
	WeaponTypeClass *Weapon = Source->GetWeapon(idxWeapon)->WeaponType;

	RET_UNLESS(Weapon->get_Warhead()->get_IvanBomb());

	R->set_EBX(Target->GetType()->get_Bombable() && !Target->get_AttachedBomb()
		? act_IvanBomb
		: act_NoIvanBomb);

	return 0;
}

// Ivan bomb cursors
// 51EB38, 6
EXPORT_FUNC(InfantryClass_GetCursorOverObject)
{
	GET(InfantryClass *, Source, EDI);
	GET(TechnoClass *, Target, ESI);

	RET_UNLESS(Target);

	int idxWeapon = Source->SelectWeapon(Target);
	WeaponTypeClass *Weapon = Source->GetWeapon(idxWeapon)->WeaponType;

	RET_UNLESS(Weapon->get_Warhead()->get_IvanBomb());

	return 0x51EB48;
}

// Ivan bomb cursors
// 740490, 6
EXPORT_FUNC(UnitClass_GetCursorOverObject)
{
	GET(UnitClass *, Source, ESI);
	GET(TechnoClass *, Target, EDI);

	RET_UNLESS(Target);

	int idxWeapon = Source->SelectWeapon(Target);
	WeaponTypeClass *Weapon = Source->GetWeapon(idxWeapon)->WeaponType;

	RET_UNLESS(Weapon->get_Warhead()->get_IvanBomb());

	R->set_EBX(Target->GetType()->get_Bombable() && !Target->get_AttachedBomb()
		? act_IvanBomb
		: act_NoIvanBomb);
	return 0;
}

// Ivan bomb cursors
// 447512, 5
EXPORT_FUNC(BuildingClass_GetCursorOverObject)
{
	GET(BuildingClass *, Source, ESI);
	GET(TechnoClass *, Target, EBP);

	RET_UNLESS(Target);

	int idxWeapon = Source->SelectWeapon(Target);
	WeaponTypeClass *Weapon = Source->GetWeapon(idxWeapon)->WeaponType;

	RET_UNLESS(Weapon->get_Warhead()->get_IvanBomb());

	R->set_EBX(Target->GetType()->get_Bombable() && !Target->get_AttachedBomb()
		? act_IvanBomb
		: act_NoIvanBomb);

	return 0;
}

// decouple Yuri UI from soviet
// 534FB1, 5
EXPORT_FUNC(Game_LoadUI)
{
	return 0x534FBB;
}
