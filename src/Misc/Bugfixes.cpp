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
#include <TeleportLocomotionClass.h>
#include <TemporalClass.h>
#include <UnitTypeClass.h>
#include <WarheadTypeClass.h>

#include "Debug.h"
#include "Actions.h"
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

#include "../Utilities/TemplateDef.h"

#include <cstdlib>
#include <array>

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

DEFINE_HOOK(53029E, Load_Bootstrap_AresMIX, 5)
{
	Ares::InitOwnResources();
	return 0;
}

DEFINE_HOOK(6BE9BD, sub_6BE1C0, 6)
{
	Ares::UninitOwnResources();
	return 0;
}

DEFINE_HOOK(715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5)
{
	return 0x715876;
}

// bugfix #231: DestroyAnims don't remap and cause reconnection errors
DEFINE_HOOK(441D25, BuildingClass_Destroy, 0A)
{
	return 0x441D37;
}

// bugfix #379: Temporal friendly kills give veterancy
// bugfix #1266: Temporal kills gain double experience
DEFINE_HOOK(71A922, TemporalClass_Update, 6) {
	return 0x71A97D;
}

// bugfix #874 A: Temporal warheads affect Warpable=no units
DEFINE_HOOK(71AF2B, TemporalClass_Fire_UnwarpableA, A) {
	// skip freeing captured and destroying spawned units,
	// as it is not clear here if this is warpable at all.
	return 0x71AF4D;
}

// bugfix #874 B: Temporal warheads affect Warpable=no units
// hook moved. search for 71AF76

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
	return (Transport && Transport->CloakState != CloakState::Uncloaked) ? 0x6FCA4F : 0;
}

// PrismSupportModifier repair
DEFINE_HOOK(671152, RulesClass_Addition_General_PrismSupportModifier, 6)
{
	GET(RulesClass*, pThis, ESI);
	REF_STACK(double, param, 0x0);
	param = pThis->PrismSupportModifier / 100.0;
	return 0x67115B;
}

// Overpowerer no longer just infantry
DEFINE_HOOK(4693B0, BulletClass_Fire_Overpower, 6)
{
	GET(TechnoClass *, pT, ECX);
	switch(pT->WhatAmI())
	{
		case AbstractType::Infantry:
		case AbstractType::Unit:
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

// 42461D, 6
// 42463A, 6
// correct warhead for animation damage
DEFINE_HOOK_AGAIN(42463A, AnimClass_Update_Damage, 6)
DEFINE_HOOK(42461D, AnimClass_Update_Damage, 6)
{
	GET(AnimClass *, Anim, ESI);
	WarheadTypeClass *W = Anim->Type->Warhead;
	if(!W) { // NOT MY HACK
		W = strcmp(Anim->Type->get_ID(), "INVISO")
			? RulesClass::Global()->FlameDamage2
			: RulesClass::Global()->C4Warhead;
	}

	DWORD origin = R->Origin();
	if(origin == 0x42461D) {
		R->ECX(W);
	} else {
		R->EDX(W);
	}

	if (Anim->Owner) {
		R->Stack<HouseClass *>(0x4, Anim->Owner);
	} else {
		if (Anim->OwnerObject) {
			if (TechnoClass* OwnerObject = generic_cast<TechnoClass *>(Anim->OwnerObject)) {
				R->Stack<HouseClass *>(0x4, OwnerObject->Owner);
				//Debug::Log("Info: %s has ownerhouse set to %s.\n",
				//Anim->Type->ID, OwnerObject->Owner->Type->ID);
			}
#ifdef DEBUGBUILD
		} else {
			Debug::Log("Info: Ownerless instance of %s.",
			Anim->Type->ID);
#endif
		}
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
	GET(AbstractType, T, EAX);
	GET(int *, X, ESI);
	R->EAX(*X);
	return T == AbstractType::Aircraft ? 0x6F75B2 : 0x6F7568;
}

// leave this here
#define XL(r) \
	GET(TechnoClass *, T, ECX); \
	Debug::Log("%c: [%s] receiving...\n", r, T->GetType()->ID); \
	Debug::Log("\t Subject = %s\n", (reinterpret_cast<TechnoClass *>(R->get_StackVar32(0x4)))->GetType()->ID); \
	Debug::Log("\t command = %d\n", R->get_StackVar32(0x8)); \
	Debug::Log("\t unknown = %d\n", R->get_StackVar32(0xC)); \
	for(DWORD i = 0x10; i < 0x40; i += 4) { \
		Debug::Log("\t 0x%02x = 0x%08X\n", i, R->get_StackVar32(i)); \
	} \
	return 0;

/*
A_FINE_HOOK(6F4AB0, TechnoClass_ReceivedRadioCommand, 8)
{
	XL('T');
}

A_FINE_HOOK(4D8FB0, FootClass_ReceivedRadioCommand, 5)
{
	XL('F');
}

A_FINE_HOOK(43C2D0, BuildingClass_ReceivedRadioCommand, 5)
{
	XL('B');
}

A_FINE_HOOK(4190B0, AircraftClass_ReceivedRadioCommand, 5)
{
	XL('A');
}

A_FINE_HOOK(737430, UnitClass_ReceivedRadioCommand, 5)
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

	Debug::DumpObj(reinterpret_cast<byte*>(&SwizzleManagerClass::Instance), sizeof(SwizzleManagerClass));

	Debug::DumpStack(R, 0x40);

	Debug::FatalErrorAndExit("Saved data loading failed");

	// return 0; does not return
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
	if(SessionClass::Instance->GameMode == GameMode::Campaign) {
		RulesClass::Global()->Read_Sides(CCINIClass::INI_Rules);
		SideExt::ExtMap.LoadAllFromINI(CCINIClass::INI_Rules);
	}
	R->EAX(0x1180);
	return 0x6873B0;
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

DEFINE_HOOK(56017A, OptionsDlg_WndProc_RemoveResLimit, 5)
{
	return 0x560183;
}

DEFINE_HOOK(5601E3, OptionsDlg_WndProc_RemoveHiResCheck, 0)
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
	GET(UnitClass* const, pUnit, ECX);
	GET(BuildingClass* const, pBay, ESI);

	auto const pType = pBay->Type;
	auto const pUnitType = pUnit->Type;

	bool isNotAcceptable = (pUnitType->BalloonHover
		|| pType->Naval != pUnitType->Naval
		|| pType->Factory == AbstractType::AircraftType
		|| pType->Helipad
		|| pType->HoverPad && !RulesClass::Instance->SeparateAircraft);

	if(isNotAcceptable) {
		return 0x455EDE;
	}

	auto const response = pUnit->SendCommand(
		RadioCommand::QueryCanEnter, pBay);

	// original game accepted any valid answer as a positive one
	return response != RadioCommand::AnswerPositive ? 0x455EDEu : 0x455E5Du;
}

/*
A_FINE_HOOK(67E75B, LoadGame_StallUI, 6)
{
	return 0x67E772;
}
*/


DEFINE_HOOK(4242CA, AnimClass_Update_FixIE_TrailerSeperation, 6)
{
	GET(AnimTypeClass *, AT, EAX);
	int trailSep = AT->TrailerSeperation;
	R->ECX(trailSep);
	return trailSep >= 1
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
	pINI->Clear(nullptr, nullptr);
	return 0;
}

DEFINE_HOOK(7440BD, UnitClass_Remove, 6)
{
	GET(UnitClass *, U, ESI);
	TechnoClass *Bunker = U->BunkerLinkedItem;
	if(auto Bld = abstract_cast<BuildingClass*>(Bunker)) {
		Bld->ClearBunker();
	}
	return 0;
}

DEFINE_HOOK(50928C, HouseClass_Update_Factories_Queues_SkipBrokenDTOR, 5)
{
	return 0x5092A3;
}

/*
issue #1051260:
commented out because of possible issues. if one team update destroys another
team, this would change the list while iterating. because one can never know
how many teams are deleted and whether the deleted teams are before or after
//the current element, it is just safer to not touch it at all. 02-Dec-12 AlexB

//westwood is stupid!
// every frame they create a vector<TeamClass *> , copy all the teams from ::Array into it, iterate with ->Update(), delete
// so this is OMG OPTIMIZED I guess
A_FINE_HOOK(55B502, LogicClass_Update_UpdateAITeamsFaster, 5)
{
	for(int i = TeamClass::Array->Count - 1; i >= 0; --i) {
		TeamClass::Array->GetItem(i)->Update();
	}
	return 0x55B5A1;
}
*/

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
	//GET(TechnoClass *, Item, ECX);

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

	auto Owner = Parasite->Owner;

	bool allowed = false;
	if(UnitClass *U = specific_cast<UnitClass *>(Owner)) {
		allowed = !U->Type->Naval;
	} else if(specific_cast<InfantryClass*>(Owner)) {
		allowed = true;
	}

	if(allowed && Owner->GetHeight() > 200) {
		*XYZ = Owner->Location;
		Owner->IsFallingDown = Owner->IsABomb = true;
	}

	return 0;
}

// update parasite coords along with the host
DEFINE_HOOK(4DB87E, FootClass_SetCoords, 6)
{
	GET(FootClass *, F, ESI);
	if(F->ParasiteEatingMe) {
		F->ParasiteEatingMe->SetLocation(F->Location);
	}
	return 0;
}

// bug 897
DEFINE_HOOK(718871, TeleportLocomotionClass_UnfreezeObject_SinkOrSwim, 7)
{
	GET(TechnoTypeClass *, Type, EAX);

	switch(Type->MovementZone) {
		case MovementZone::Amphibious:
		case MovementZone::AmphibiousCrusher:
		case MovementZone::AmphibiousDestroyer:
		case MovementZone::WaterBeach:
			R->BL(1);
			return 0x7188B1;
	}
	if(Type->SpeedType == SpeedType::Hover) {
		// will set BL to 1 , unless this is a powered unit with no power centers <-- what if we have a powered unit that's not a hover?
		return 0x71887A;
	}
	return 0x7188B1;
}

/*
 * Fixing issue #954
 */
DEFINE_HOOK(621B80, DSurface_FillRectWithColor, 5)
{
	GET(RectangleStruct*, rect, ECX);
	GET(Surface*, surface, EDX);

	int surfaceWidth = surface->GetWidth();
	int surfaceHeight = surface->GetHeight();

	//make sure the rectangle to fill is within the surface's boundaries, this should do the trick
	rect->X = (rect->X >= 0) ? rect->X : 0;
	rect->Y = (rect->Y >= 0) ? rect->Y : 0;
	rect->Width = (rect->X + rect->Width <= surfaceWidth) ? rect->Width : surfaceWidth - rect->X;
	rect->Height = (rect->Y + rect->Height <= surfaceHeight) ? rect->Height : surfaceHeight - rect->Y;

	if(rect->Width == 0 || rect->Height == 0)
		return 0x621D26;
	else
		return 0;
}

DEFINE_HOOK(52BA78, _YR_GameInit_Pre, 5)
{
	// issue #198: animate the paradrop cursor
	MouseCursor::GetCursor(MouseCursorType::ParaDrop).Interval = 4;

	// issue #214: also animate the chronosphere cursor
	MouseCursor::GetCursor(MouseCursorType::Chronosphere).Interval = 4;
	
	// issue #1380: the iron curtain cursor
	MouseCursor::GetCursor(MouseCursorType::IronCurtain).Interval = 4;

	// animate the engineer damage cursor
	MouseCursor::GetCursor(MouseCursorType::Detonate).Interval = 4;

	return 0;
}

DEFINE_HOOK(469467, BulletClass_DetonateAt_CanTemporalTarget, 5)
{
	GET(TechnoClass *, Target, ECX);
	Layer lyr = Target->InWhichLayer();
	switch(lyr) {
		case Layer::Ground:
		case Layer::Air:
		case Layer::Top:
			return 0x469475;
		default:
			return 0x469AA4;
	}
}

/* #183 - cloakable on Buildings and Aircraft */
DEFINE_HOOK(442CE0, BuildingClass_Init_Cloakable, 6)
{
	GET(BuildingClass *, Item, ESI);

	if(Item->Type->Cloakable) {
		Item->Cloakable = true;
	}

	return 0;
}

DEFINE_HOOK(413FA3, AircraftClass_Init_Cloakable, 5)
{
	GET(AircraftClass *, Item, ESI);

	if(Item->Type->Cloakable) {
		Item->Cloakable = true;
	}

	return 0;
}

DEFINE_HOOK(48A507, SelectDamageAnimation_FixNegatives, 5)
{
	GET(int, Damage, EDI);
	Damage = abs(Damage);
	R->EDI(Damage);
	return 0;
}

/* #1354 - Aircraft and empty SovParaDropInf list */
DEFINE_HOOK(41D887, AirstrikeClass_Fire, 6)
{
	if(!RulesClass::Instance->SovParaDropInf.Count) {
		R->ECX(-1);
		return 0x41D895;
	}
	return 0;
}

// issue #1282: remap wall using its owner's colors
DEFINE_HOOK(47F9A4, DrawOverlay_WallRemap, 6) {
	GET(CellClass*, pCell, ESI);
	
	int idx = pCell->WallOwnerIndex;
	
	if(idx >= 0) {
		HouseClass* pOwner = HouseClass::Array->GetItem(idx);
		R->EDX(pOwner);
		return 0x47F9AA;
	}
	return 0;
}


DEFINE_HOOK(418478, AircraftClass_Mi_Attack_Untarget1, 6)
{
	GET(AircraftClass *, A, ESI);
	return A->Target
		? 0
		: 0x4184C2
	;
}

DEFINE_HOOK(4186D7, AircraftClass_Mi_Attack_Untarget2, 6)
{
	GET(AircraftClass *, A, ESI);
	return A->Target
		? 0
		: 0x418720
	;
}

DEFINE_HOOK(418826, AircraftClass_Mi_Attack_Untarget3, 6)
{
	GET(AircraftClass *, A, ESI);
	return A->Target
		? 0
		: 0x418883
	;
}

DEFINE_HOOK(418935, AircraftClass_Mi_Attack_Untarget4, 6)
{
	GET(AircraftClass *, A, ESI);
	return A->Target
		? 0
		: 0x418992
	;
}

DEFINE_HOOK(418A44, AircraftClass_Mi_Attack_Untarget5, 6)
{
	GET(AircraftClass *, A, ESI);
	return A->Target
		? 0
		: 0x418AA1
	;
}

DEFINE_HOOK(418B40, AircraftClass_Mi_Attack_Untarget6, 6)
{
	GET(AircraftClass *, A, ESI);
	return A->Target
		? 0
		: 0x418B8A
	;
}

// issue #1437: crash when warping out buildings infantry wants to garrison
DEFINE_HOOK(71AA52, TemporalClass_Update_AnnounceInvalidPointer, 8) {
	GET(TechnoClass*, pVictim, ECX);
	pVictim->IsAlive = false;
	return 0;
}

// issue 1520: logging stupid shit crashes the game
DEFINE_HOOK(4CA437, FactoryClass_GetCRC, 0)
{
	GET(FactoryClass *, pThis, ECX);
	GET_STACK(DWORD, pCRC, 0xC);

	R->ESI<FactoryClass *>(pThis);
	R->EDI(pCRC);

	return 0x4CA501;
}

// issue #1532
DEFINE_HOOK(749088, Count_ResetWithGivenCount, 6)
{
	GET(unsigned int, Width, EAX);
	if(Width > 512) {
		Debug::Log(Debug::Severity::Warning, "One of the internal counters attempted to overflow "
			"(given width of %d exceeds maximum allowed width of 512).\n"
			"The counter width has been reset to 512, but this can result in unpredictable behaviour and crashes.\n", Width);
		R->EAX(512);
	}
	return 0;
}

// #1260: reinforcements via actions 7 and 80, and chrono reinforcements
// via action 107 cause crash if house doesn't exist
DEFINE_HOOK_AGAIN(65EC4A, TeamTypeClass_ValidateHouse, 6)
DEFINE_HOOK(65D8FB, TeamTypeClass_ValidateHouse, 6)
{
	GET(TeamTypeClass*, pThis, ECX);
	HouseClass* pHouse = pThis->GetHouse();

	// house exists; it's either declared explicitly (not Player@X) or a in campaign mode
	// (we don't second guess those), or it's still alive in a multiplayer game
	if(pHouse && (pThis->Owner || SessionClass::Instance->GameMode == GameMode::Campaign || !pHouse->Defeated)) {
		return 0;
	}

	// no.
	return (R->Origin() == 0x65D8FB) ? 0x65DD1B : 0x65F301;
}

DEFINE_HOOK(70CBDA, TechnoClass_DealParticleDamage, 6)
{
	GET(TechnoClass *, pSource, EDX);
	R->Stack<HouseClass *>(0xC, pSource->Owner);
	return 0;
}

DEFINE_HOOK(62CDE8, ParticleClass_Update_Fire, 5)
{
	GET(ParticleClass *, pParticle, ESI);
	if(auto System = pParticle->ParticleSystem) {
		if(auto Owner = System->Owner) {
			R->Stack<TechnoClass *>(0x4, Owner);
			R->Stack<HouseClass *>(0x10, Owner->Owner);
		}
	}
	return 0;
}

DEFINE_HOOK(62C2ED, ParticleClass_Update_Gas, 6)
{
	GET(ParticleClass *, pParticle, EBP);
	if(auto System = pParticle->ParticleSystem) {
		if(auto Owner = System->Owner) {
			R->Stack<TechnoClass *>(0x0, Owner);
			R->Stack<HouseClass *>(0xC, Owner->Owner);
		}
	}
	return 0;
}

// #1708: this mofo was raising an event without checking whether
// there is a valid tag. this is the only faulty call of this kind.
DEFINE_HOOK(4692A2, BulletClass_DetonateAt_RaiseAttackedByHouse, 6)
{
	GET(ObjectClass*, pVictim, EDI);
	return pVictim->AttachedTag ? 0 : 0x4692BD;
}

// destroying a building (no health left) resulted in a single green pip shown
// in the health bar for a split second. this makes the last pip red.
DEFINE_HOOK(6F661D, TechnoClass_DrawHealthBar_DestroyedBuilding_RedPip, 7)
{
	GET(BuildingClass*, pBld, ESI);
	return (pBld->Health <= 0 || pBld->IsRedHP()) ? 0x6F6628 : 0x6F6630;
}

DEFINE_HOOK(47243F, CaptureManagerClass_DecideUnitFate_BuildingFate, 6) {
	GET(TechnoClass *, pVictim, EBX);
	if(specific_cast<BuildingClass *>(pVictim)) {
		// 1. add to team and other fates don't really make sense for buildings
		// 2. BuildingClass::Mission_Hunt() implementation is to do nothing!
		pVictim->QueueMission(Mission::Guard, 0);
		return 0x472604;
	}
	return 0;
}

DEFINE_HOOK(4471D5, BuildingClass_Sell_DetonateNoBuildup, 6)
{
	GET(BuildingClass *, pStructure, ESI);
	if(auto Bomb = pStructure->AttachedBomb) {
		Bomb->Detonate();
	}

	return 0;
}

DEFINE_HOOK(44A1FF, BuildingClass_Mi_Selling_DetonatePostBuildup, 6) {
	GET(BuildingClass *, pStructure, EBP);
	if(auto Bomb = pStructure->AttachedBomb) {
		Bomb->Detonate();
	}

	return 0;
}

DEFINE_HOOK(4D9F7B, FootClass_Sell_Detonate, 6)
{
	GET(FootClass *, pSellee, ESI);
	if(auto Bomb = pSellee->AttachedBomb) {
		Bomb->Detonate();
	}
	return 0;
}

DEFINE_HOOK(739956, UnitClass_Deploy_TransferIvanBomb, 6)
{
	GET(UnitClass *, pUnit, EBP);
	GET(BuildingClass *, pStructure, EBX);

	TechnoExt::TransferIvanBomb(pUnit, pStructure);
	TechnoExt::TransferAttachedEffects(pUnit, pStructure);
	TechnoExt::TransferOriginalOwner(pUnit, pStructure);

	return 0;
}

DEFINE_HOOK(44A03C, BuildingClass_Mi_Selling_TransferIvanBomb, 6)
{
	GET(BuildingClass *, pStructure, EBP);
	GET(UnitClass *, pUnit, EBX);

	TechnoExt::TransferIvanBomb(pStructure, pUnit);
	TechnoExt::TransferAttachedEffects(pStructure, pUnit);
	TechnoExt::TransferOriginalOwner(pStructure, pUnit);

	return 0;
}

// do not let deactivated teleporter units move, otherwise
// they could block a cell forever 
DEFINE_HOOK(71810D, TeleportLocomotionClass_ILocomotion_MoveTo_Deactivated, 6)
{
	GET(FootClass*, pFoot, ECX);
	return (!pFoot->Deactivated && pFoot->Locomotor->Is_Powered()) ? 0 : 0x71820F;
}

// issues 896173 and 1433804: the teleport locomotor keeps a copy of the last
// coordinates, and unmarks the occupation bits of that place instead of the
// ones the unit was added to after putting it back on the map. that left the
// actual cell blocked. this fix resets the last coords, so the actual position
// is unmarked.
DEFINE_HOOK(51DF27, InfantryClass_Remove_Teleport, 6)
{
	GET(InfantryClass* const, pThis, ECX);

	if(pThis->Type->Teleporter) {
		auto const pLoco = static_cast<LocomotionClass*>(
			pThis->Locomotor.get());

		CLSID clsid;
		if(SUCCEEDED(pLoco->GetClassID(&clsid))
			&& clsid == LocomotionClass::CLSIDs::Teleport)
		{
			auto const pTele = static_cast<TeleportLocomotionClass*>(pLoco);
			pTele->LastCoords = CoordStruct::Empty;
		}
	}

	return 0;
}

// issues 1002020, 896263, 895954: clear stale mind control pointer to prevent
// crashes when accessing properties of the destroyed controllers.
DEFINE_HOOK(7077EE, TechnoClass_PointerGotInvalid_ResetMindControl, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);

	if(pThis->MindControlledBy == ptr) {
		pThis->MindControlledBy = nullptr;
	}

	return 0;
}

// skip theme log lines
DEFINE_HOOK_AGAIN(720C42, Theme_Stop_NoLog, 5) // skip Theme::Stop
DEFINE_HOOK(720DE8, Theme_Stop_NoLog, 5) // skip Theme::PlaySong
{
	return R->Origin() + 5;
}

DEFINE_HOOK(720F37, sub_720EA0_NoLog, 5) // skip Theme::Stop
{
	return 0x720F3C;
}

DEFINE_HOOK(720A61, sub_7209D0_NoLog, 5) // skip Theme::AI
{
	return 0x720A66;
}

// skips the log line "Looping Movie"
DEFINE_HOOK(615BD3, Handle_Static_Messages_LoopingMovie, 5)
{
	return 0x615BE0;
}

// #908369, #1100953: units are still deployable when warping or falling
DEFINE_HOOK(700E47, TechnoClass_CanDeploySlashUnload_Immobile, A)
{
	GET(UnitClass*, pThis, ESI);

	CellClass * pCell = pThis->GetCell();
	CoordStruct crd = pCell->GetCoordsWithBridge();

	// recreate replaced check, and also disallow if unit is still warping or dropping in.
	if(pThis->IsUnderEMP() || pThis->IsWarpingIn() || pThis->IsFallingDown) {
		return 0x700DCE;
	}

	return 0x700E59;
}

// #1156943, #1156937: replace the engineer check, because they were smart
// enough to use the pointer right before checking whether it's null, and
// even if it isn't, they build a possible infinite loop.
DEFINE_HOOK(44A5F0, BuildingClass_Mi_Selling_EngineerFreeze, 6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(InfantryTypeClass*, pType, ESI);
	LEA_STACK(bool*, pEngineerSpawned, 0x13);

	if(*pEngineerSpawned && pType && pType->Engineer) {
		// randomize until probability is below 0.01%
		// for only the Engineer tag being returned.
		for(int i=9; i>=0; --i) {
			pType = !i ? nullptr : pThis->GetCrew();

			if(!pType || !pType->Engineer) {
				break;
			}
		}
	}

	if(pType && pType->Engineer) {
		*pEngineerSpawned = true;
	}

	R->ESI(pType);
	return 0x44A628;
}

// #1156943: they check for type, and for the instance, yet
// the Log call uses the values as if nothing happened.
DEFINE_HOOK(4430E8, BuildingClass_Demolish_LogCrash, 6)
{
	GET(BuildingClass*, pThis, EDI);
	GET(InfantryClass*, pInf, ESI);

	R->EDX(pThis ? pThis->Type->Name : "<none>");
	R->EAX(pInf ? pInf->Type->Name : "<none>");
	return 0x4430FA;
}

// #1171643: keep the last passenger if this is a gunner, not just
// when it has multiple turrets. gattling and charge turret is no
// longer affected by this.
DEFINE_HOOK(73D81E, UnitClass_Mi_Unload_LastPassenger, 5)
{
	GET(UnitClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();
	R->EAX(pType->Gunner ? 1 : 0);
	return 0x73D823;
}

// stop command would still affect units going berzerk
DEFINE_HOOK(730EE5, StopCommandClass_Execute_Berzerk, 6)
{
	GET(TechnoClass*, pTechno, ESI);

	return pTechno->Berzerk ? 0x730EF7 : 0;
}

// do not infiltrate buildings of allies
DEFINE_HOOK(519FF8, InfantryClass_UpdatePosition_PreInfiltrate, 6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	if(!pThis->Type->Agent || pThis->Owner->IsAlliedWith(pBld)) {
		return 0x51A03E;
	}

	return 0x51A002;
}

// replaces entire function (without the pip distortion bug)
DEFINE_HOOK(4748A0, INIClass_GetPipIdx, 7)
{
	GET(INIClass*, pINI, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(int, fallback, 0xC);

	int ret = fallback;
	if(pINI->ReadString(pSection, pKey, Ares::readDefval, Ares::readBuffer)) {

		// find the pip value with the name specified
		auto PipTypes = reinterpret_cast<std::array<const NamedValue, 11>*>(0x81B958);
		auto it = std::find(PipTypes->begin(), PipTypes->end(), Ares::readBuffer);

		if(it != PipTypes->end()) {
			ret = it->Value;

		} else if(!Parser<int>::TryParse(Ares::readBuffer, &ret)) {
			// parsing as integer didn't work either. invalid value
			Debug::Log(Debug::Severity::Warning, "Could not parse pip at [%s]%s=\n", pSection, pKey);
			Debug::RegisterParserError();
			ret = fallback;
		}
	}

	R->EAX(ret);
	return 0x474907;
}

// replaced entire function. error was using delete[] instead of delete.
// it potentially crashed when any of the files were present in the
// game directory.
DEFINE_HOOK(5F77F0, ObjectTypeClass_UnloadPipsSHP, 5)
{
	bool* pAllocated = reinterpret_cast<bool*>(0xAC1488);
	SHPStruct* pShp[] = {FileSystem::PIPBRD_SHP, FileSystem::PIPS_SHP,
		FileSystem::PIPS2_SHP, FileSystem::TALKBUBL_SHP};

	for(int i = 0; i < 4; ++i) {
		if(pAllocated[i] && pShp[i]) {
			GameDelete(pShp[i]);
			pAllocated[i] = false;
		}
	}

	return 0x5F78FB;
}

// #895584: ships not taking damage when repaired in a shipyard. bug
// was that the logic that prevented units from being damaged when
// exiting a war factory applied here, too. added the Naval check.
DEFINE_HOOK(737CE4, UnitClass_ReceiveDamage_ShipyardRepair, 6)
{
	GET(BuildingTypeClass*, pType, ECX);
	return (pType->WeaponsFactory && !pType->Naval) ? 0x737CEE : 0x737D31;
}

DEFINE_HOOK(4B5EB0, DropPodLocomotionClass_ILocomotion_Process_Smoke, 6)
{
	REF_STACK(const CoordStruct, Coords, 0x34);

	// create trailer even without weapon, but only if it is set
	if(!(Unsorted::CurrentFrame % 6)) {
		auto pExt = RulesExt::Global();

		// look up the smoke, if it isn't set yet
		if(!pExt->DropPodTrailer.isset()) {
			pExt->DropPodTrailer = AnimTypeClass::Find("SMOKEY");
		}

		if(AnimTypeClass* pType = pExt->DropPodTrailer) {
			GameCreate<AnimClass>(pType, Coords);
		}
	}

	// copy replaced instruction
	auto pWeap = RulesClass::Instance->DropPodWeapon;
	R->ESI(pWeap);

	// jump behind trailer, or out
	return pWeap ? 0x4B5F14 : 0x4B602D;
}

DEFINE_HOOK(4B5F9E, DropPodLocomotionClass_ILocomotion_Process_Report, 6)
{
	// do not divide by zero
	GET(int, count, EBP);
	return count ? 0 : 0x4B5FAD;
}

DEFINE_HOOK(52070F, InfantryClass_UpdateFiringState_Uncloak, 5)
{
	GET(InfantryClass*, pThis, EBP);
	GET_STACK(int, idxWeapon, STACK_OFFS(0x34, 0x24));

	if(pThis->IsCloseEnough(pThis->Target, idxWeapon)) {
		pThis->Uncloak(false);
	}

	return 0x52094C;
}

DEFINE_HOOK(69281E, DisplayClass_ChooseAction_TogglePower, A)
{
	GET(TechnoClass*, pTarget, ESI);
	REF_STACK(enum class Action, Action, STACK_OFFS(0x20, 0x10));

	bool allowed = false;

	if(auto pBld = abstract_cast<BuildingClass*>(pTarget)) {
		auto pOwner = pBld->GetOwningHouse();

		if(pOwner && pOwner->ControlledByPlayer()) {
			if(pBld->CanBeSelected() && !pBld->IsStrange() && !pBld->IsBeingWarpedOut() && !pBld->IsUnderEMP()) {
				allowed = pBld->Type->CanTogglePower();
			}
		}
	}

	if(allowed) {
		Action = Action::TogglePower;
		Actions::Set(&RulesExt::Global()->TogglePowerCursor);
	} else {
		Action = Action::NoTogglePower;
		Actions::Set(&RulesExt::Global()->TogglePowerNoCursor);
	}

	return 0x69289B;
}

// naive way to fix negative indexes to be generated. proper way would be to replace
// the entire function, and the function consuming the indexes. it is not yet known
// whether the out of bounds read causes desync errors. this function appears to
// have been inlined prominently in 585F40
DEFINE_HOOK(56BC54, ThreatPosedEstimates_GetIndex, 5)
{
	GET(const CellStruct*, pCell, ECX);

	int index = -1;
	if(pCell->X >= 0 && pCell->Y >= 0 && pCell->X < 512 && pCell->Y < 512) {
		index = pCell->X / 4 + 130 * pCell->Y / 4 + 131;
	}

	R->EAX(index);
	return 0x56BC7D;
}

// reject negative indexes. if the index is the result of the function above, this
// catches all invalid cells. otherwise, the game can write of of bounds, which can
// set a field that is supposed to be a pointer, and crash when calling a virtual
// method on it. in worst case, this goes unnoticed.
DEFINE_HOOK(4FA2E0, HouseClass_SetThreat_Bounds, 7)
{
	//GET(HouseClass*, pThis, ESI);
	GET_STACK(int, index, 0x4);
	//GET_STACK(int, threat, 0x8);

	return index < 0 ? 0x4FA347u : 0;
}

// gunners and opentopped together do not support temporals, because the gunner
// takes away the TemporalImUsing from the infantry, and thus it is missing
// when the infantry fires out of the opentopped vehicle
DEFINE_HOOK(6FC339, TechnoClass_GetFireError_OpenToppedGunnerTemporal, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);

	bool ret = true;
	if(pWeapon->Warhead->Temporal && pThis->Transporter) {
		auto const pType = pThis->Transporter->GetTechnoType();
		if(pType->Gunner && pType->OpenTopped) {
			ret = (pThis->TemporalImUsing != nullptr);
		}
	}

	return ret ? 0u : 0x6FCD29u;
}

// invalid or not set edge reads array out of bounds 
DEFINE_HOOK(4759D4, INIClass_WriteEdge, 7)
{
	GET(int const, index, EAX);

	if(index < 0 || index > 4) {
		R->EDX("none");
		return 0x4759DB;
	}

	return 0;
}

// the following three hooks fix 895855, 937938, and 1171659

// prevent invisible mcvs, which shouldn't happen any more as the sell/move
// hack is fixed. thus this one is a double unnecessity
DEFINE_HOOK(449FF8, BuildingClass_Mi_Selling_PutMcv, 7)
{
	GET(UnitClass* const, pUnit, EBX);
	GET(unsigned int, facing, EAX);
	REF_STACK(CoordStruct const, Crd, STACK_OFFS(0xD0, 0xB8));

	// set the override for putting, not just for creation as WW did
	++Unsorted::IKnowWhatImDoing;
	auto const ret = pUnit->Put(Crd, facing);
	--Unsorted::IKnowWhatImDoing;

	// should never happen, but if anything breaks, it's here
	if(!ret) {
		// do not keep the player alive if it couldn't be placed
		GameDelete(pUnit);
	}

	return ret ? 0x44A010u : 0x44A16Bu;
}

// remember that this building ejected its survivors already
DEFINE_HOOK(44A8A2, BuildingClass_Mi_Selling_Crew, A)
{
	GET(BuildingClass* const, pThis, EBP);
	pThis->NoCrew = true;
	return 0;
}

// don't set the focus when selling (true selling, thus no focus set atm)
DEFINE_HOOK(4C6DDB, Networking_RespondToEvent_Selling, 8)
{
	GET(TechnoClass* const, pTechno, EDI);
	GET(AbstractClass* const, pFocus, EAX);

	if(pTechno->CurrentMission != Mission::Selling || pTechno->Focus) {
		pTechno->SetFocus(pFocus);
	}

	return 0x4C6DE3;
}

// #1415844: units in open-topped transports show behind anim
DEFINE_HOOK(6FA2C7, TechnoClass_Update_DrawHidden, 8)
{
	GET(TechnoClass* const, pThis, ESI);
	auto const disallowed = pThis->InOpenToppedTransport;
	return !disallowed ? 0u : 0x6FA30Cu;
}
