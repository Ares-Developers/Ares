#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include <YRPP.h>
#include "Ares.h"
#include <MacroHelpers.h> //basically indicates that this is DCoder country

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
		// Aircraft can't use them right just yet, IE if out of weapon range...
		case abs_Aircraft:
		case abs_Infantry:
		case abs_Unit:
		case abs_Building:
			return 0x438E97;
		default:
			return 0x439022;
	}
}

// 6FA4C6, 5
/* this is a wtf: it unsets target if the unit can no longer affect its current target. 
 * Makes sense, except Aircraft that lose the target so crudely in the middle of the attack
 * (i.e. ivan bomb weapon) go wtfkerboom with an IE
 */
EXPORT_FUNC(TechnoClass_Update_ZeroOutTarget)
{
	GET(TechnoClass *, T, ESI);
	return T->WhatAmI() == abs_Aircraft ? 0x6FA4D1 : 0;
}

// 46934D, 6
EXPORT_FUNC(IvanBombs_Spread)
{
	GET(BulletClass *, bullet, ESI);
	double cSpread = bullet->get_WH()->get_CellSpread();

	RET_UNLESS(bullet->get_Target());
	
	TechnoClass *pOwner = (TechnoClass *)bullet->get_Owner();
	
	TechnoClass *pTarget = (TechnoClass *)bullet->get_Target();
	CoordStruct tgtLoc = *(pTarget->get_Location());

	// just real target
	if(cSpread < 0.5)
	{
		BombListClass::Global()->Plant(pOwner, pTarget);
		return 0;
	}

	int Spread = int(cSpread);

	CoordStruct tgtCoords;
	pTarget->GetCoords(&tgtCoords);
	CellStruct centerCoords;//, xyzTgt;
	centerCoords = *MapClass::Global()->GetCellAt(&tgtCoords)->get_MapCoords();

	int countCells = CellSpread::NumCells(Spread);

	for(int i = 0; i < countCells; ++i)
	{
		CellStruct tmpCell = CellSpread::GetCell(i);
		tmpCell += centerCoords;
		CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);

		for(ObjectClass *curObj = c->get_FirstObject(); curObj; curObj = curObj->get_NextObject())
		{
			if(curObj != pOwner && (curObj->get_AbstractFlags() & ABSFLAGS_ISTECHNO) && !curObj->get_AttachedBomb())
			{
				BombListClass::Global()->Plant(pOwner, (TechnoClass *)curObj);
			}
		}
	}

	return 0;
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

// decouple Yuri UI from soviet
// 534FB1, 5
EXPORT_FUNC(Game_LoadUI)
{
	return 0x534FBB;
}

// fix the 100 unit bug for vehicles
// 4FEA60 - INCOMPLETE, DO NOT HOOK YET
EXPORT Fix100UnitBug_Vehicles(REGISTERS* R)
{
	HouseClass* pThis = (HouseClass*)R->get_ECX();

	if(pThis->get_ProducingUnitTypeIndex() == -1)
	{
		int nParentCountryIndex = HouseTypeClass::FindIndex(pThis->get_Type()->get_ParentCountry());
		DWORD flagsOwner = 1 << nParentCountryIndex;

		UnitTypeClass* pHarvester = NULL;
		for(int i = 0; i < RulesClass::Global()->get_HarvesterUnit()->get_Count(); i++)
		{
			UnitTypeClass* pCurrent = (*RulesClass::Global()->get_HarvesterUnit())[i];
			if(pCurrent->get_OwnerFlags() & flagsOwner)
			{
				pHarvester = pCurrent;
				break;
			}
		}
		
		if(pHarvester)
		{
			//Buildable harvester found
			int nHarvesters = pThis->get_CountResourceGatherers();
			
			int mMaxHarvesters = 
				(*RulesClass::Global()->get_HarvestersPerRefinery())[pThis->get_AIDifficulty()] * pThis->get_CountResourceDestinations();
			if(!pThis->FirstBuildableFromArray(RulesClass::Global()->get_BuildRefinery()))
				mMaxHarvesters = 
					(*RulesClass::Global()->get_AISlaveMinerNumber())[pThis->get_AIDifficulty()];

			if(pThis->get_IQLevel2() >= RulesClass::Global()->get_Harvester() &&
				!pThis->get_unknown_bool_242())
			{
				bool bPlayerControl;

				if(*(eGameMode*)0xA8B238 == gm_Campaign) //TODO: Session::Global()->get_GameMode()
					bPlayerControl = pThis->get_CurrentPlayer() || pThis->get_PlayerControl();
				else
					bPlayerControl = pThis->get_CurrentPlayer();

				if(!bPlayerControl &&
						nHarvesters < mMaxHarvesters &&
						pThis->get_TechLevel() >= pHarvester->get_TechLevel())
				{
					pThis->set_ProducingUnitTypeIndex(pHarvester->get_ArrayIndex());
					goto RETURN; //please slap me :3
				}
			}
		}
		else
		{
			//No buildable harvester found
			int mMaxHarvesters =
				(*RulesClass::Global()->get_AISlaveMinerNumber())[pThis->get_AIDifficulty()];

			if(pThis->get_CountResourceGatherers() < mMaxHarvesters)
			{
				BuildingTypeClass* pBT = pThis->FirstBuildableFromArray(RulesClass::Global()->get_BuildRefinery());
				if(pBT)
				{
					//awesome way to find out whether this building is a slave miner, isn't it? ...
					UnitTypeClass* pSlaveMiner = pBT->get_UndeploysInto();
					if(pSlaveMiner)
					{
						pThis->set_ProducingUnitTypeIndex(pSlaveMiner->get_ArrayIndex());
						goto RETURN; //just accept that goto makes things easier in this case
					}
				}
			}
		}

		//Don't build a harvester
		//TO BE CONTINUED... from here on, we need TeamClass and TaskForceClass definitions. 
	}

RETURN:
	R->set_EAX(0xF);
	return 0x4FEEDA;
}

// 6BB9DD, 5
EXPORT_FUNC(WinMain_LogNonsense)
{
	return 0x6BBE2B;
}

// 701190, 5
EXPORT_FUNC(TechnoClass_IsPowerOnline)
{
	GET(TechnoClass *, Techno, ECX);

	R->set_EAX(Techno->IsActive());
	return 0;
}

// 4D8500, 7
EXPORT_FUNC(FootClass_UpdatePosition)
{
	GET(FootClass *, F, ECX);
	if(((TechnoTypeClass*)F->GetType())->get_GapRadiusInCells() > 0)
	{
		if(F->get_Locomotor()->Is_Moving()) {
			F->DestroyGap();
		}
		else
		{
			F->CreateGap();
		}
	}
	return 0;
}

// bugfix #187: Westwood idiocy
// 531726, 5
EXPORT_FUNC(StupidPips1)
{
	return 0x53173A;
}

// bugfix #187: Westwood idiocy
// 53173F, 5
EXPORT_FUNC(StupidPips2)
{
	return 0x531749;
}

// bugfix #187: Westwood idiocy
// 5F698F, 5
EXPORT_FUNC(ObjectClass_GetCell)
{
	return 0x5F69B2;
}

// UNTESTED!!
// bugfix #388: Units firing from inside a transport do not obey DecloakToFire
// 6FCA30, 6
EXPORT_FUNC(TechnoClass_GetWeaponState)
{
	GET(TechnoClass *, Techno, ESI);
	TechnoClass *Transport = Techno->get_Transporter();
	RET_UNLESS(Transport && Transport->get_CloakState());
	return 0x6FCA4F;
}

// PrismSupportModifier repair
// 671152, 6
EXPORT_FUNC(RulesClass_Addition_General)
{
	GET(RulesClass *, Rules, ESI);
	Rules->set_PrismSupportModifier(Rules->get_PrismSupportModifier() / 100);
	return 0;
}

// 4693B0, 6
// Overpowerer no longer just infantry
EXPORT_FUNC(BulletClass_Fire_Overpower)
{
	GET(TechnoClass *, pT, ECX);
	switch(pT->WhatAmI())
	{
		case abs_Infantry:
		case abs_Unit:
			return 0x4693BC;
		default:
			return 0x469AA4;
	}
}

// 74036E, 5
// I'm tired of getting "Cannot Enter" when something is selected and trying to select an IFV, fixing that...
EXPORT_FUNC(FooClass_GetCursorOverObject)
{
	DWORD orig = R->get_Origin();
	if(orig == 0x74036E)
	{
		R->set_EBX(act_Select);
	}
	else
	{
		R->set_EBP(act_Select);
	}
	return orig + 5;
}

// 4F7E49, 5
// upgrades as prereqs, facepalm of epic proportions
EXPORT_FUNC(HouseClass_CanBuildHowMany_Upgrades)
{
		return R->get_EAX() < 3 ? 0x4F7E41 : 0x4F7E34;
}

// 6D3D10, 6
// dump a list of all loaded MIXes
EXPORT_FUNC(Dump)
{
	Ares::Log("MIX list: \n");
	RET_UNLESS(Unsorted::CurrentFrame == 5);
	MixFileClass* MIX = (MixFileClass*)MixFileClass::MIXes->get_First()->get_Next();
	int idx = 0;
	while(MIX) {
		Ares::Log("MIX file #%d is %s\n", idx, MIX->get_FileName());
		MIX = (MixFileClass*)MIX->get_Next();
		++idx;
	}
	return 0;
}

// 715857, 5
EXPORT_FUNC(TechnoTypeClass_LoadFromINI_LimitPalettes)
{
	return 0x715876;
}

// 4444E2, 6
// alternative factory search - instead of same [Type], use any of same Factory= and Naval=
EXPORT_FUNC(BuildingClass_KickOutUnit)
{
	GET(BuildingClass *, Src, ESI);
	GET(BuildingClass *, Tst, EBP);

	if(Src != Tst
	 && Tst->GetCurrentMission() == mission_Guard
	 && Tst->get_Type()->get_Factory() == Src->get_Type()->get_Factory()
	 && Tst->get_Type()->get_Naval() == Src->get_Type()->get_Naval()
	 && !Tst->get_Factory())
	{
		return 0x44451F;
	}

	return 0x444508;
}
