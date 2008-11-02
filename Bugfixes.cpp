#include <AnimClass.h>
#include <BombListClass.h>
#include <BulletClass.h>
#include <CellSpread.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <LocomotionClass.h>
#include <MapClass.h>
#include <TechnoClass.h>
#include <TemporalClass.h>
#include <UnitTypeClass.h>
#include <WarheadTypeClass.h>

#ifdef DEBUGBUILD
#include "WarheadTypeExt.h"
#include "ArmorTypes.h"
#include "Debug.h"
#endif

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include "TechnoTypeExt.h"

// bugfix #231: DestroyAnims don't remap and cause reconnection errors
DEFINE_HOOK(441D25,BuildingClass_Destroy,0A)
{
	return 0x441D37;
}

// bugfix #379: Temporal friendly kills give veterancy
DEFINE_HOOK(71A92A, _Temporal_AvoidFriendlies, 5)
{
	GET(TemporalClass *, Temp, ESI); 

	HouseClass *hv = Temp->get_Target()->get_Owner();
	HouseClass *ho = Temp->get_Owner()->get_Owner();

	RET_UNLESS(ho->IsAlliedWith(hv));
	return 0x71A97D;
}

// bugfix #385: Only InfantryTypes can use Ivan Bombs
DEFINE_HOOK(438E86, IvanBombs_AttachableByAll, 5)
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

/* this is a wtf: it unsets target if the unit can no longer affect its current target. 
 * Makes sense, except Aircraft that lose the target so crudely in the middle of the attack
 * (i.e. ivan bomb weapon) go wtfkerboom with an IE
 */
DEFINE_HOOK(6FA4C6, TechnoClass_Update_ZeroOutTarget, 5)
{
	GET(TechnoClass *, T, ESI);
	return (T->WhatAmI() == abs_Aircraft) ? 0x6FA4D1 : 0;
}

DEFINE_HOOK(46934D, IvanBombs_Spread, 6)
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
DEFINE_HOOK(4D98DD, Insignificant_UnitLost, 6)
{
	GET(TechnoClass *, t, ESI);
	TechnoTypeClass *T = t->GetTechnoType(); //R->get_EAX(); would work, but let's see if this does as well

	return (T->get_Insignificant() || T->get_DontScore()) ? 0x4D9916 : 0;
}

// bugfix #277: VeteranInfantry and friends don't show promoted cameos
DEFINE_HOOK(71204C, TechnoTypeClass_GetCameo, 6)
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
DEFINE_HOOK(71204C, TechnoTypeClass_GetCameo2, 6)
{
	GET(TechnoTypeClass *, T, ESI);
	return T->get_Naval() ? 0x7120BF : 0x7120C5;
}

// MakeInfantry that fails to place will just end the source animation and cleanup instead of memleaking to game end
DEFINE_HOOK(424B23, AnimClass_Update, 6)
{
	GET(InfantryClass *, I, EDI);
	I->UnInit();
	GET(AnimClass *, A, ESI);
	A->set_TimeToDie(1);
	A->UnInit();
	return 0x424B29;
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

DEFINE_HOOK(6BB9DD, WinMain_LogNonsense, 5)
{
	return 0x6BBE2B;
}

/*
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
*/

// bugfix #187: Westwood idiocy
DEFINE_HOOK(531726, StupidPips1, 5)
{
	return 0x53173A;
}

// bugfix #187: Westwood idiocy
DEFINE_HOOK(53173F, StupidPips2, 5)
{
	return 0x531749;
}

// bugfix #187: Westwood idiocy
DEFINE_HOOK(5F698F, ObjectClass_GetCell, 5)
{
	return 0x5F69B2;
}

// UNTESTED!!
// bugfix #388: Units firing from inside a transport do not obey DecloakToFire
DEFINE_HOOK(6FCA30, TechnoClass_GetWeaponState, 6)
{
	GET(TechnoClass *, Techno, ESI);
	TechnoClass *Transport = Techno->get_Transporter();
	RET_UNLESS(Transport && Transport->get_CloakState());
	return 0x6FCA4F;
}

// PrismSupportModifier repair
DEFINE_HOOK(671152, RulesClass_Addition_General, 6)
{
	GET(RulesClass *, Rules, ESI);
	Rules->set_PrismSupportModifier(Rules->get_PrismSupportModifier() / 100);
	return 0;
}

// Overpowerer no longer just infantry
DEFINE_HOOK(4693B0, BulletClass_Fire_Overpower, 6)
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

// upgrades as prereqs, facepalm of epic proportions
DEFINE_HOOK(4F7E49, HouseClass_CanBuildHowMany_Upgrades, 5)
{
		return R->get_EAX() < 3 ? 0x4F7E41 : 0x4F7E34;
}

#ifdef DEBUGBUILD
DEFINE_HOOK(6D3D10, Dump, 6)
{
	/*
	if(Unsorted::CurrentFrame == 5) {
		// do something for debugging - generic debug state report hook
		DEBUGLOG("Verses Against Armor Types:        ");
		for(int j = 0; j < ArmorType::Array.get_Count(); ++j)
		{
			DEBUGLOG("|%10s", ArmorType::Array[j]->Title);
		}
		DEBUGLOG("\n");
		for(int i = 0; i < WarheadTypeClass::Array->get_Count(); ++i)
		{
			WarheadTypeClass *WH = WarheadTypeClass::Array->GetItem(i);
			WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[WH];
			DEBUGLOG("[%24s]Verses =", WH->get_ID());
			for(int j = 0; j < pData->Verses.get_Count(); ++j)
			{
				DEBUGLOG("%c %9.6lf", (j ? ',' : ' '), pData->Verses[j]);
			}
			DEBUGLOG("\n");
		}

	}

	else */

	static DWORD lastFrame = 0;
	if(Unsorted::CurrentFrame == 15 && lastFrame != 15) { // this hook gets triggered 3 times per frame, so hack
		DEBUGLOG("Weapons loaded: \n");
		for(int i = 0; i < TechnoTypeClass::Array->get_Count(); ++i)
		{
			TechnoTypeClass *T = (TechnoTypeClass *)TechnoTypeClass::Array->GetItem(i);
			DEBUGLOG("[%s]\n", T->get_ID());
			TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[T];

			DEBUGLOG("\tWeapons:\n");
			for(int j = 0; j < pData->Weapons.get_Count(); ++j)
			{
				WeaponTypeClass *W = pData->Weapons[j].WeaponType;
				if(W)
				{
					DEBUGLOG("\t\t#%02d: %s\n", j, W->get_ID());
				}
				else
				{
					DEBUGLOG("\t\t#%02d: No WeaponType\n", j);
				}
			}

			DEBUGLOG("\tEliteWeapons:\n");
			for(int j = 0; j < pData->EliteWeapons.get_Count(); ++j)
			{
				WeaponTypeClass *W = pData->EliteWeapons[j].WeaponType;
				if(W)
				{
					DEBUGLOG("\t\t#%02d: %s\n", j, W->get_ID());
				}
				else
				{
					DEBUGLOG("\t\t#%02d: No WeaponType\n", j);
				}
			}

			DEBUGLOG("\n");
		}

	}

	lastFrame = Unsorted::CurrentFrame;

	return 0;
}
#endif

DEFINE_HOOK(715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5)
{
	return 0x715876;
}

// alternative factory search - instead of same [Type], use any of same Factory= and Naval=
DEFINE_HOOK(4444E2, BuildingClass_KickOutUnit, 6)
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

// 42461D, 6
// 42463A, 6
// correct warhead for animation damage
DEFINE_HOOK(42461D, AnimClass_Update_Damage, 6)
DEFINE_HOOK_AGAIN(42463A, AnimClass_Update_Damage, 6)
{
	GET(AnimClass *, Anim, ESI);
	WarheadTypeClass *W = Anim->get_Type()->get_Warhead();
	if(!W) // NOT MY HACK
	{
		W = strcmp(Anim->get_Type()->get_ID(), "INVISO")
			? RulesClass::Global()->get_FlameDamage2()
			: RulesClass::Global()->get_C4Warhead();
	}
	DWORD WH = (DWORD)W;

	DWORD origin = R->get_Origin();
	if(origin == 0x42461D)
	{
		R->set_ECX(WH);
	}
	else
	{
		R->set_EDX(WH);
	}
	return origin + 6;
}

DEFINE_HOOK(51F76D, InfantryClass_Unload, 5)
{
	GET(TechnoClass *, I, ESI);
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[I->GetTechnoType()];
	return pData->Is_Deso ? 0x51F77D : 0x51F792;
}

DEFINE_HOOK(51CE9A, InfantryClass_Idle, 5)
{
	GET(InfantryClass *, I, ESI);
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[I->GetTechnoType()];
	return pData->Is_Cow ? 0x51CEAE : 0x51CECD;
}

DEFINE_HOOK(747BBD, UnitTypeClass_LoadFromINI, 5)
{
	GET(UnitTypeClass *, U, ESI);

	U->set_AltImage((SHPStruct *)R->get_EAX()); // jumping over, so replicated
	return U->get_Gunner()
		? 0x74BDD7
		: 0x747E90;
}

/*
// 7090D0, 5
EXPORT_FUNC(TechnoClass_SelectFiringVoice)
{
	GET(TechnoClass *, T, ESI);
	if(T->WhatAmI() == abs_Unit && (UnitClass *)T->get_Gunner())
	{
		R->set_EDI(T::VoiceRepair);
		return 0x70914A;
	}
	return 0x7090ED;
}
*/

// stop aircraft from losing target when it's in air
DEFINE_HOOK(414D36, AACombat, 6)
{
	return 0x414D4D;
}

// godawful hack - Desolator deploy fire is triggered by ImmuneToRadiation !
DEFINE_HOOK(5215F9, InfantryClass_UpdateDeploy, 6)
{
	GET(TechnoClass *, I, ESI);
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[I->GetTechnoType()];
	return pData->Is_Deso ? 0x5216B6 : 0x52160D;
}

// 52138C, 6
// godawful hack 2 - Desolator deploy fire is triggered by ImmuneToRadiation !
// DON'T USE
EXPORT_FUNC(InfantryClass_UpdateDeploy2)
{
/*
	GET(TechnoClass *, I, ESI);
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[I->GetTechnoType()];
	return pData->Is_Deso_Radiation ? 0x52139A : 0x5214B9;
	WRONG: needs more code to reimplement weapon shooting without rad checks
*/
	return 0;
}

// westwood does firingUnit->WhatAmI() == abs_AircraftType
// which naturally never works
// let's see what this change does
DEFINE_HOOK(6F7561, Arcing_Aircraft, 5)
{
	int T = R->get_EAX();
	int *X = (int *)R->get_ESI();
	R->set_EAX(*X);
	return T == abs_Aircraft ? 0x6F75B2 : 0x6F7568;
}

