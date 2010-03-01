#include <AnimClass.h>
#include <BulletClass.h>
#include <CCINIClass.h>
#include <CellSpread.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <LightSourceClass.h>
#include <LocomotionClass.h>
#include <LoadProgressManager.h>
#include <MapClass.h>
#include <MixFileClass.h>
#include <TacticalClass.h>
#include <TechnoClass.h>
#include <TemporalClass.h>
#include <UnitTypeClass.h>
#include <WarheadTypeClass.h>

#include "Debug.h"
#include "../Ares.h"
#include "../Ext/Rules/Body.h"
#include "../Ext/BuildingType/Body.h"
#include "../Ext/BulletType/Body.h"
#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/House/Body.h"
#include "../Ext/Side/Body.h"
#include "../Ext/HouseType/Body.h"
#include "../Ext/WeaponType/Body.h"
#include "../Ext/WarheadType/Body.h"

#include <Utilities/Template.h>

#ifdef DEBUGBUILD
#include "../Ext/WarheadType/Body.h"
#include "../Enum/ArmorTypes.h"
#endif

// fix for ultra-fast processors overrunning the performance evaluator function
DEFINE_HOOK(5CB0B1, QueryPerformance, 5)
{
	if(!R->EAX()) {
		R->EAX(1);
	}
	return 0;
}

DEFINE_HOOK(6BD7E3, Expand_MIX_Reorg, 5)
{
	MixFileClass::Bootstrap();
	return 0;
}

DEFINE_HOOK(52BB64, Expand_MIX_Deorg, 5)
{
	R->AL(1);
	return 0x52BB69;
}

DEFINE_HOOK(715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5)
{
	return 0x715876;
}

// bugfix #471: InfantryTypes and BuildingTypes don't reload their ammo properly
DEFINE_HOOK(43FE8E, BuildingClass_SkipStupidAmmoCode, 6)
{
	return 0x43FEBE;
}

DEFINE_HOOK(6F9E5B, TechnoClass_ReloadOverride, 6)
{
	GET(TechnoClass *, T, ESI);
	if(T->WhatAmI() == abs_Infantry || T->WhatAmI() == abs_Building) {
		T->Reload();
	}
	return 0;
}

// bugfix #231: DestroyAnims don't remap and cause reconnection errors
DEFINE_HOOK(441D25, BuildingClass_Destroy, 0A)
{
	return 0x441D37;
}

// bugfix #379: Temporal friendly kills give veterancy
DEFINE_HOOK(71A92A, _Temporal_AvoidFriendlies, 5)
{
	GET(TemporalClass *, Temp, ESI);

	HouseClass *hv = Temp->Target->Owner;
	HouseClass *ho = Temp->Owner->Owner;

	return ho->IsAlliedWith(hv) ? 0x71A97D : 0;
}

// Insignificant=yes or DontScore=yes prevent EVA_UnitLost on unit destruction
DEFINE_HOOK(4D98DD, Insignificant_UnitLost, 6)
{
	GET(TechnoClass *, t, ESI);
	TechnoTypeClass *T = t->GetTechnoType();

	return (T->Insignificant || T->DontScore) ? 0x4D9916 : 0;
}

// bugfix #277 revisited: VeteranInfantry and friends don't show promoted cameos
DEFINE_HOOK(712045, TechnoTypeClass_GetCameo, 5)
{
	// egads and gadzooks
	retfunc<SHPStruct *> ret(R, 0x7120C6);

	GET(TechnoTypeClass *, T, ECX);
	GET(HouseClass *, House, EAX);
	HouseTypeClass *Country = House->Type;

	SHPStruct *Cameo = T->Cameo;
	SHPStruct *Alt = T->AltCameo;
	if(!Alt) {
		return ret(Cameo);
	}

	switch(T->WhatAmI()) {
		case abs_InfantryType:
			if(House->BarracksInfiltrated && !T->Naval && T->Trainable) {
				return ret(Alt);
			} else {
				return ret(Country->VeteranInfantry.GetItemIndex((InfantryTypeClass **)&T) == -1 ? Cameo : Alt);
			}
		case abs_UnitType:
			if(House->WarFactoryInfiltrated && !T->Naval && T->Trainable) {
				return ret(Alt);
			} else {
				return ret(Country->VeteranUnits.GetItemIndex((UnitTypeClass **)&T) == -1 ? Cameo : Alt);
			}
		case abs_AircraftType:
			return ret(Country->VeteranAircraft.GetItemIndex((AircraftTypeClass **)&T) == -1 ? Cameo : Alt);
		case abs_BuildingType:
			if(TechnoTypeClass *Item = T->UndeploysInto) {
				return ret(Country->VeteranUnits.GetItemIndex((UnitTypeClass **)&Item) == -1 ? Cameo : Alt);
			}
	}

	return ret(Cameo);
}

// MakeInfantry that fails to place will just end the source animation and cleanup instead of memleaking to game end
DEFINE_HOOK(424B23, AnimClass_Update, 6)
{
	GET(InfantryClass *, I, EDI);
	I->UnInit();
	GET(AnimClass *, A, ESI);
	A->TimeToDie = 1;
	A->UnInit();
	return 0x424B29;
}

DEFINE_HOOK(6BB9DD, WinMain_LogNonsense, 5)
{
	return 0x6BBE2B;
}

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
	TechnoClass *Transport = Techno->Transporter;
	RET_UNLESS(Transport && Transport->CloakState);
	return 0x6FCA4F;
}

// PrismSupportModifier repair
DEFINE_HOOK(671152, RulesClass_Addition_General, 6)
{
	GET(RulesClass *, Rules, ESI);
	Rules->PrismSupportModifier /= 100;
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
/*
A_FINE_HOOK(74036E, FooClass_GetCursorOverObject, 5)
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
*/

// alternative factory search - instead of same [Type], use any of same Factory= and Naval=
DEFINE_HOOK(4444E2, BuildingClass_KickOutUnit, 6)
{
	GET(BuildingClass *, Src, ESI);
	GET(BuildingClass *, Tst, EBP);

	if(Src != Tst
	 && Tst->GetCurrentMission() == mission_Guard
	 && Tst->Type->Factory == Src->Type->Factory
	 && Tst->Type->Naval == Src->Type->Naval
	 && !Tst->Factory)
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
	WarheadTypeClass *W = Anim->Type->Warhead;
	if(!W) { // NOT MY HACK
		W = strcmp(Anim->Type->get_ID(), "INVISO")
			? RulesClass::Global()->FlameDamage2
			: RulesClass::Global()->C4Warhead;
	}
	DWORD WH = (DWORD)W;

	DWORD origin = R->get_Origin();
	if(origin == 0x42461D) {
		R->ECX(WH);
	} else {
		R->EDX(WH);
	}
	return 0; // WHAT? origin + 6;
}

/*
// 7090D0, 5
XPORT_FUNC(TechnoClass_SelectFiringVoice)
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
DEFINE_HOOK(414D36, AACombat, 5)
{
	return 0x414D4D;
}

// westwood does firingUnit->WhatAmI() == abs_AircraftType
// which naturally never works
// let's see what this change does
DEFINE_HOOK(6F7561, Arcing_Aircraft, 5)
{
	GET(int, T, EAX);
	GET(int *, X, ESI);
	R->EAX(*X);
	return T == abs_Aircraft ? 0x6F75B2 : 0x6F7568;
}

// leave this here
#define XL(r) \
	GET(TechnoClass *, T, ECX); \
	Debug::Log("%c: [%s] receiving...\n", r, T->GetType()->ID); \
	Debug::Log("\t Subject = %s\n", ((TechnoClass *)R->get_StackVar32(0x4))->GetType()->ID); \
	Debug::Log("\t command = %d\n", R->get_StackVar32(0x8)); \
	Debug::Log("\t unknown = %d\n", R->get_StackVar32(0xC)); \
	for(DWORD i = 0x10; i < 0x40; i += 4) { \
		Debug::Log("\t 0x%02x = 0x%08X\n", i, R->get_StackVar32(i)); \
	} \
	return 0;

/*
DEFINE_HOOK(6F4AB0, TechnoClass_ReceivedRadioCommand, 8)
{
	XL('T');
}

DEFINE_HOOK(4D8FB0, FootClass_ReceivedRadioCommand, 5)
{
	XL('F');
}

DEFINE_HOOK(43C2D0, BuildingClass_ReceivedRadioCommand, 5)
{
	XL('B');
}

DEFINE_HOOK(4190B0, AircraftClass_ReceivedRadioCommand, 5)
{
	XL('A');
}

DEFINE_HOOK(737430, UnitClass_ReceivedRadioCommand, 5)
{
	XL('U');
}
//*/

/*
A_FINE_HOOK(69AE90, GameData_SetProgress, 5)
{
	GET_STACK(int, progress, 0x4);
	Debug::Log("Progress is now %d%%\n", progress);

	return 0;
}
*/
/*
A_FINE_HOOK(447348, BuildingClass_GetCursorOverObject_CY, 6)
{
	GET(BuildingClass *, B, ESI);
	eAbstractType Fact = B->Type->Factory;
	return (Fact && Fact != abs_BuildingType)
	  ? 0x4472EC
	  : 0x447358;
}
*/

/*
in void BuildingClass::Destroy(BuildingClass *this) {
	v19 = this->Type->GetActualCost(this->OwningPlayer) / Rules->ShakeScreen;
	if ( v19 > 0 )
		TacticalClass::ShakeScreen(v19);
}
//------------
in void UnitClass::Destroy(UnitClass *this) {
	v7 = this->Type->Strength;
	v6 = Rules->ShakeScreen;
	if ( v7 > v6 ) {
		v8 = v7 / (v6 / 2) + 3;
		if ( v8 >= 6 )
			v8 = 6;
		TacticalClass::ShakeScreen(v8);
	}
}*/
/*
A_FINE_HOOK(48DED0, ShakeScreen, 1)
{
	GET(int, Force, ECX);
	// shake the screen
	return 0;
}
*/

DEFINE_HOOK(6CF3CF, sub_6CF350, 8)
{
	GET(DWORD, A, EAX);
	GET(DWORD *, B, ECX);

	Debug::Log("Swizzle comparison failed - %X != %X\n", A, *B);

	Debug::DumpObj((byte *)&SwizzleManagerClass::Instance, sizeof(SwizzleManagerClass));

	Debug::DumpStack(R, 0x40);

	Debug::FatalError("Saved data loading failed\n", true);

	return 0;
}

/*
A_FINE_HOOK(6CF2C0, SwizzleManagerClass_Here_I_Am, 5)
{
//	Debug::Log("Swizzle::Here_I_Am(%X, %X)\n", R->get_StackVar32(0x8), R->get_StackVar32(0xC));
//	Debug::DumpStack(R, 0x40);
	return 0;
}

A_FINE_HOOK(6CF240, SwizzleManagerClass_Swizzle, 6)
{
//	Debug::Log("Swizzle::Swizzle(%X)\n", R->get_StackVar32(0x8));
//	Debug::DumpStack(R, 0x40);
	return 0;
}

A_FINE_HOOK(6CF350, SwizzleManagerClass_Convert, 7)
{
//	Debug::Log("Swizzle::Convert()\n");
//	Debug::DumpStack(R, 0x100);
	return 0;
}
*/

// testing lightpost draw changes
// the constants are the same as in the unmodded game - modify them to alter the way the lightposts illuminate cells
// see http://dc0d3r.name/src2/CellClass/GetColourComponents.cpp for context
/*
A_FINE_HOOK(48439A, CellClass_GetColourComponents, 5)
{
	GET(int, Distance, EAX);
	GET(LightSourceClass *, LS, ESI);
	TintStruct *LSTint = LS->get_LightTint();

	GET_STACK(int*, Intensity, 0x44);
	GET_STACK(int*, Tint_Red, 0x54);
	GET_STACK(int*, Tint_Green, 0x58);
	GET_STACK(int*, Tint_Blue, 0x5C);

	const int RangeVisibilityFactor = 1000;
	const int RangeDistanceFactor = 1000;
	const int LightMultiplier = 1000;

	int LSEffect = (RangeVisibilityFactor * LS->LightVisibility - RangeDistanceFactor * Distance) / LS->LightVisibility;
	*Intensity  += int(LSEffect * LS->LightIntensity  / LightMultiplier);
	*Tint_Red   += int(LSEffect * LSTint->Red   / LightMultiplier);
	*Tint_Green += int(LSEffect * LSTint->Green / LightMultiplier);
	*Tint_Blue  += int(LSEffect * LSTint->Blue  / LightMultiplier);

	return 0x484440;
}
*/

DEFINE_HOOK(6873AB, INIClass_ReadScenario_EarlyLoadRules, 5)
{
	switch(Unsorted::GameMode) {
		case gm_Campaign:
			RulesClass::Global()->Read_Sides(CCINIClass::INI_Rules);
			SideExt::ExtMap.LoadAllFromINI(CCINIClass::INI_Rules);
		default:
			R->EAX(0x1180);
			return 0x6873B0;
	}
}

// allowhiresmodes
/*
A_FINE_HOOK(5FA41D, GameOptionsClass_CTOR, 5)
{
	GET(byte *, Options, EAX);
	Options[0x35] = 0; // zero out the hires flag entirely
	return 0;
}
*/

DEFINE_HOOK(5601E3, OptionsDlg_WndProc, 0)
{
	// skip the allowhires check entirely - all supported 16bit modes are accepted, should make net resolution limit stfu
	return 0x5601FC;
}

//yikes
//sidebar on the left - westwood's leftover code, doesn't work, enable at own risk, etc. etc.
/*
A_FINE_HOOK(5FAD09, Options_LoadFromINI, 5)
{
	GET(byte *, Options, ESI);
	Options[0x1C] = 0;
	return 0;
}
*/

DEFINE_HOOK(455E4C, HouseClass_FindRepairBay, 9)
{
	GET(UnitClass *, Unit, ECX);
	GET(BuildingClass *, Bay, ESI);

	UnitTypeClass *UT = Unit->Type;
	BuildingTypeClass *BT = Bay->Type;

	bool isNotAcceptable = (UT->BalloonHover
	 || BT->Naval != UT->Naval
	 || BT->Factory == abs_AircraftType
	 || BT->Helipad
	 || BT->HoverPad && !RulesClass::Instance->SeparateAircraft
	);

	if(isNotAcceptable) {
		return 0x455EDE;
	}

	eRadioCommands Response = Unit->SendCommand(rc_CanEnter, Bay);

	return Response == 0
	 ? 0x455EDE
	 : 0x455E5D
	;
}

/*
A_FINE_HOOK(67E75B, LoadGame_StallUI, 6)
{
	return 0x67E772;
}
*/


DEFINE_HOOK(505B36, HouseClass_GenerateAIBuildList_C0, 8)
{
	LEA_STACK(DynamicVectorClass<BuildingTypeClass *> *, PlannedBase, 0x14);
	return PlannedBase->Count < 1
	 ? 0x505C95
	 : 0
	;
}

DEFINE_HOOK(505B92, HouseClass_GenerateAIBuildList_C1, 7)
{
	LEA_STACK(DynamicVectorClass<BuildingTypeClass *> *, PlannedBase, 0x14);
	return PlannedBase->Count < 2
	 ? 0x505C95
	 : 0
	;
}

DEFINE_HOOK(505BE1, HouseClass_GenerateAIBuildList_C2, 7)
{
	LEA_STACK(DynamicVectorClass<BuildingTypeClass *> *, PlannedBase, 0x14);
	return PlannedBase->Count < 3
	 ? 0x505C95
	 : 0
	;
}

DEFINE_HOOK(4242CA, AnimClass_Update_FixIE_TrailerSeperation, 6)
{
	GET(AnimTypeClass *, AT, EAX);
	return AT->TrailerSeperation >= 1
	 ? 0x4242D5
	 : 0x424322
	;
}

DEFINE_HOOK(441C21, BuildingClass_Destroy_ShakeScreenZero, 6)
{
	return RulesClass::Instance->ShakeScreen
	 ? 0
	 : 0x441C39
	;
}

DEFINE_HOOK(699C1C, Game_ParsePKTs_ClearFile, 7)
{
	LEA_STACK(CCINIClass *, pINI, 0x24);
	pINI->Clear(NULL, NULL);
	return 0;
}

DEFINE_HOOK(7440BD, UnitClass_Remove, 6)
{
	GET(UnitClass *, U, ESI);
	TechnoClass *Bunker = U->BunkerLinkedItem;
	if(Bunker && Bunker->WhatAmI() == abs_Building) {
		reinterpret_cast<BuildingClass *>(Bunker)->ClearBunker();
	}
	return 0;
}

DEFINE_HOOK(50928C, HouseClass_Update_Factories_Queues_SkipBrokenDTOR, 5)
{
	return 0x5092A3;
}

// removing hardcoded references to GAWALL and NAWALL
DEFINE_HOOK(440709, BuildingClass_Put, 6)
{
	GET(CellClass *, Cell, EDI);
	int idxOverlay = Cell->OverlayTypeIndex;
	bool Sellable = true;
	if(idxOverlay > -1) {
		Sellable = OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;
	}
	return Sellable ? 0x44071A : 0x440725;
}

DEFINE_HOOK(480534, sub_480510, 5)
{
	GET(int, idxOverlay, EAX);
	bool Wall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;
	return Wall ? 0x480549 : 0x480552;
}

//westwood is stupid!
// every frame they create a vector<TeamClass *> , copy all the teams from ::Array into it, iterate with ->Update(), delete
// so this is OMG OPTIMIZED I guess
DEFINE_HOOK(55B502, LogicClass_Update_UpdateAITeamsFaster, 5)
{
	for(int i = TeamClass::Array->Count - 1; i >= 0; --i) {
		TeamClass::Array->GetItem(i)->Update();
	}
	return 0x55B5A1;
}

// Guard command failure
DEFINE_HOOK(730DB0, GuardCommandClass_Execute, 0)
{
	GET(TechnoClass *, T, ESI);
	return (T->Owner != HouseClass::Player || !T->IsControllable())
		? 0x730E62
		: 0x730DBE
	;
}

/* #367 - do we need to draw a link to this victim */
DEFINE_HOOK(472198, CaptureManagerClass_DrawLinks, 6)
{
	enum { Draw_Maybe = 0, Draw_Yes = 0x4721E6, Draw_No = 0x472287} decision = Draw_Maybe;
	GET(CaptureManagerClass *, Controlled, EDI);
	GET(TechnoClass *, Item, ECX);

	if(FootClass *F = generic_cast<FootClass *>(Controlled->Owner)) {
		if(F->ParasiteImUsing && F->InLimbo) {
			decision = Draw_No;
		}
	}

	return decision;
}

/* #746 - don't set parasite eject point to cell center, but set it to fall and explode like a bomb */
DEFINE_HOOK(62A2F8, ParasiteClass_PointerGotInvalid, 6)
{
	GET(ParasiteClass *, Parasite, ESI);
	GET(CoordStruct *, XYZ, EAX);

	if(UnitClass *U = specific_cast<UnitClass *>(Parasite->Owner)) {
		if(!U->Type->Naval && U->GetHeight() > 200) {
			*XYZ = U->Location;
			U->FallingDown = U->IsABomb = true;
		}
	}

	return 0;
}
