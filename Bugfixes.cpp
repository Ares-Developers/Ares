#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include <YRPP.h>

// bugfix #379
// 71A92A, 5
EXPORT_FUNC(_Temporal_AvoidFriendlies)
{
	TemporalClass *m = (TemporalClass *)R->get_ESI(); 

	HouseClass *hv = m->get_TargetUnit()->get_Owner();
	HouseClass *ho = m->get_OwningUnit()->get_Owner();

	if(ho->IsAlliedWith(hv)) {
		return 0x71A97D;
	}

	return 0;
}

// bugfix #385
// 438E86, 5
EXPORT_FUNC(IvanBombs_AttachableByAll)
{
	TechnoClass *Source = (TechnoClass *)R->get_EBP();
	switch(Source->What_Am_I())
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
	BulletClass *bullet = (BulletClass *)R->get_ESI();
	double cSpread = bullet->get_WH()->get_CellSpread();

	if(!bullet->get_Target())
	{
		return 0;
	}
	
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
	TechnoClass *t = (TechnoClass *)R->get_ESI();
	TechnoTypeClass *T = (TechnoTypeClass *)t->GetType(); //R->get_EAX(); would work, but let's see if this does as well

	return (T->get_Insignificant() || T->get_DontScore()) ? 0x4D9916 : 0;
}

// bugfix #277
// 71204C, 6
EXPORT_FUNC(TechnoTypeClass_GetCameo)
{
	TechnoTypeClass *T = (TechnoTypeClass *)R->get_ESI();
	HouseClass *Player = (HouseClass *)R->get_EAX();

	switch(T->What_Am_I())
	{
		case abs_InfantryType:
			if(Player->get_Type()->get_VeteranInfantry()->FindItemIndex((InfantryTypeClass *)T) != -1)
			{
				R->set_EAX((DWORD)T->get_AltCameo());
				return 0x7120C5;
			}
			break;
		case abs_UnitType:
			if(Player->get_Type()->get_VeteranUnits()->FindItemIndex((UnitTypeClass *)T) != -1)
			{
				R->set_EAX((DWORD)T->get_AltCameo());
				return 0x7120C5;
			}
			break;
		default:
			return 0;
	}
	return 0;
}

// Crewed=yes jumpjets spawn parachuted infantry on destruction, not idle
// 7382EA, 6
EXPORT_FUNC(UnitClass_ReceiveDamage)
{
	GET(InfantryClass *, I)R->get_EDI();
	if(I->GetHeight() > 0)
	{
		I->set_FallingDown(1);
		I->set_HasParachute(1);
		I->set_FallRate(0);
	}
	return 0;
}

// MakeInfantry that fails to place will just end the source animation and cleanup instead of memleaking to game end
// 424B23, 6
EXPORT_FUNC(AnimClass_Update)
{
	GET(InfantryClass *, I)R->get_EDI();
	I->UnInit();
	GET(AnimClass *, A)R->get_ESI();
	A->set_TimeToDie(1);
	A->UnInit();
	return 0x424B29;
}
