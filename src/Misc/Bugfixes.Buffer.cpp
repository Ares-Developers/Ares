#include <AnimClass.h>
#include <BombListClass.h>
#include <BulletClass.h>
#include <CCINIClass.h>
#include <CellSpread.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <LocomotionClass.h>
#include <MapClass.h>
#include <MixFileClass.h>
#include <ParticleSystemTypeClass.h>
#include <SmudgeTypeClass.h>
#include <TechnoClass.h>
#include <TemporalClass.h>
#include <TerrainTypeClass.h>
#include <UnitTypeClass.h>
#include <WarheadTypeClass.h>

#include "Debug.h"
#include "../Ares.h"

#include "../Utilities/Macro.h"

/* issue 193 - increasing the buffer length for certain flag parsing */

DEFINE_HOOK(511D16, Buf_CountryVeteran, 9)
{
	GET(HouseTypeClass *, H, EBX);
	GET(CCINIClass *, INI, ESI);

	const char *section = H->ID;
	PARSE_VECTOR_N(section, H, VeteranInfantry, InfantryTypeClass);
	PARSE_VECTOR_N(section, H, VeteranUnits, UnitTypeClass);
	PARSE_VECTOR_N(section, H, VeteranAircraft, AircraftTypeClass);

	return 0x51208C;
}

// one hook to overwrite all lists and in the sequence skip them

// ============= [General] =============
DEFINE_HOOK(66D55E, Buf_General, 6)
{
	GET(RulesClass *, Rules, ESI);
	GET(CCINIClass *, INI, EDI);

	const char *section = "General";
	PARSE_VECTOR_N(section, Rules, AmerParaDropInf, InfantryTypeClass);
	PARSE_VECTOR_N(section, Rules, AllyParaDropInf, InfantryTypeClass);
	PARSE_VECTOR_N(section, Rules, SovParaDropInf, InfantryTypeClass);
	PARSE_VECTOR_N(section, Rules, YuriParaDropInf, InfantryTypeClass);

	PARSE_VECTOR_INT(section, AmerParaDropNum, Rules);
	PARSE_VECTOR_INT(section, AllyParaDropNum, Rules);
	PARSE_VECTOR_INT(section, SovParaDropNum, Rules);
	PARSE_VECTOR_INT(section, YuriParaDropNum, Rules);

	PARSE_VECTOR_N(section, Rules, AnimToInfantry, InfantryTypeClass);

	PARSE_VECTOR_N(section, Rules, SecretInfantry, InfantryTypeClass);
	PARSE_VECTOR_N(section, Rules, SecretUnits, UnitTypeClass);
	PARSE_VECTOR_N(section, Rules, SecretBuildings, BuildingTypeClass);
	DWORD Sum = Rules->SecretInfantry.Count
		+ Rules->SecretUnits.Count
		+ Rules->SecretBuildings.Count;

	Rules->SecretSum = Sum;

	PARSE_VECTOR_N(section, Rules, HarvesterUnit, UnitTypeClass);
	PARSE_VECTOR_N(section, Rules, BaseUnit, UnitTypeClass);
	PARSE_VECTOR_N(section, Rules, PadAircraft, AircraftTypeClass);

	PARSE_VECTOR_N(section, Rules, Shipyard, BuildingTypeClass);
	PARSE_VECTOR_N(section, Rules, RepairBay, BuildingTypeClass);

	PARSE_VECTOR_N(section, Rules, WeatherConClouds, AnimTypeClass);
	PARSE_VECTOR_N(section, Rules, WeatherConBolts, AnimTypeClass);
	PARSE_VECTOR_N(section, Rules, BridgeExplosions, AnimTypeClass);

	PARSE_VECTOR_N(section, Rules, DefaultMirageDisguises, TerrainTypeClass);
	return 0;
}

DEFINE_HOOK(67062F, Buf_AnimToInf_Paradrop, 6)
{
	return 0x6707FE;
}

DEFINE_HOOK(66FA13, Buf_SecretBoons, 6)
{
	return 0x66FAD6;
}

DEFINE_HOOK(66F7C0, Buf_PPA, 9)
{
	GET(RulesClass *, Rules, ESI);

	GET(UnitTypeClass *, Pt, EAX); // recreating overwritten bits
	Rules->PrerequisiteProcAlternate = Pt;

	return 0x66F9FA;
}

DEFINE_HOOK(66F589, Buf_Shipyard, 6)
{
	return 0x66F68C;
}


DEFINE_HOOK(66F34B, Buf_RepairBay, 5)
{
	GET(RulesClass *, Rules, ESI);

	Rules->NoParachuteMaxFallRate = R->EAX();

	return 0x66F450;
}


DEFINE_HOOK(66DD13, Buf_WeatherArt, 6)
{
	return 0x66DF19;
}

DEFINE_HOOK(66DB93, Buf_BridgeExplosions, 6)
{
	return 0x66DC96;
}

// ============= [CombatDamage] =============
DEFINE_HOOK(66BC71, Buf_CombatDamage, 9)
{
	GET(RulesClass *, Rules, ESI);
	GET(CCINIClass *, INI, EDI);

	Rules->TiberiumStrength = R->EAX();

	const char *section = "CombatDamage";
	PARSE_VECTOR_N(section, Rules, Scorches, SmudgeTypeClass);
	PARSE_VECTOR_N(section, Rules, Scorches1, SmudgeTypeClass);
	PARSE_VECTOR_N(section, Rules, Scorches2, SmudgeTypeClass);
	PARSE_VECTOR_N(section, Rules, Scorches3, SmudgeTypeClass);
	PARSE_VECTOR_N(section, Rules, Scorches4, SmudgeTypeClass);

	PARSE_VECTOR_N(section, Rules, SplashList, AnimTypeClass);
	return 0x66C287;
}

// ============= [AI] =============

#define P_VEC_B(key) \
	PARSE_VECTOR_N(section, Rules, key, BuildingTypeClass);

DEFINE_HOOK(672B0E, Buf_AI, 6)
{
	GET(RulesClass *, Rules, ESI);
	GET(CCINIClass *, INI, EDI);

	const char *section = "AI";
	P_VEC_B(BuildConst);
	P_VEC_B(BuildPower);
	P_VEC_B(BuildRefinery);
	P_VEC_B(BuildBarracks);
	P_VEC_B(BuildTech);
	P_VEC_B(BuildWeapons);
	P_VEC_B(AlliedBaseDefenses);
	P_VEC_B(SovietBaseDefenses);
	P_VEC_B(ThirdBaseDefenses);
	P_VEC_B(BuildDefense);
	P_VEC_B(BuildPDefense);
	P_VEC_B(BuildAA);
	P_VEC_B(BuildHelipad);
	P_VEC_B(BuildRadar);
	P_VEC_B(ConcreteWalls);
	P_VEC_B(NSGates);
	P_VEC_B(EWGates);
	P_VEC_B(BuildNavalYard);
	P_VEC_B(BuildDummy);
	P_VEC_B(NeutralTechBuildings);

	PARSE_VECTOR_INT(section, AIForcePredictionFudge, Rules);

	return 0x673950;
}


// == TechnoType ==
DEFINE_HOOK(7121A3, Buf_TechnoType, 6)
{
	GET(TechnoTypeClass *, T, EBP);
	GET(const char *, section, EBX);
	GET(CCINIClass *, INI, ESI);

	PARSE_VECTOR_BIT(section, T, ForbiddenHouses, HouseTypeClass, ForbiddenHouses);
	PARSE_VECTOR_BIT(section, T, SecretHouses, HouseTypeClass, SecretHouses);
	PARSE_VECTOR_BIT(section, T, RequiredHouses, HouseTypeClass, RequiredHouses);
	PARSE_VECTOR_BIT(section, T, Owner, HouseTypeClass, OwnerFlags);

	PARSE_VECTOR_N(section, T, DamageParticleSystems, ParticleSystemTypeClass);
	PARSE_VECTOR_N(section, T, DestroyParticleSystems, ParticleSystemTypeClass);

//	PARSE_VECTOR_N(section, T, PrerequisiteOverride, BuildingTypeClass);

// no VoxelAnimTypeClass def yet, and therefore ints can be left alone as well
//	PARSE_VECTOR_INT(section, T, DebrisMaximums);
//	PARSE_VECTOR_N(section, T, DebrisTypes, VoxelAnimTypeClass);

	return 0;
}

DEFINE_HOOK(7149E1, Buf_Owner, 6)
{
	return 0x7149FB;
}

DEFINE_HOOK(714522, Buf_OwnHouses, 6)
{
	return 0x714570;
}

/*FINE_HOOK(71420D, Buf_PrereqOverride, 6)
{
	return 0x714293;
}
*/

DEFINE_HOOK(713BF1, Buf_DmgpartSys, 6)
{
	GET(TechnoTypeClass *, T, EBP);
	GET(ParticleSystemTypeClass *, SPS, EAX);

	T->RefinerySmokeParticleSystem = SPS;
	return 0x713E1A;
}


// == WarheadType ==
DEFINE_HOOK(75D660, Buf_Warhead, 9)
{
	GET(WarheadTypeClass *, WH, ESI);
	GET(const char *, section, EBP);
	GET(CCINIClass *, INI, EDI);

	PARSE_VECTOR_N(section, WH, AnimList, AnimTypeClass);

// no VoxelAnimTypeClass def yet, and therefore ints can be left alone as well
//	PARSE_VECTOR_INT(section, T, DebrisMaximums);
//	PARSE_VECTOR_N(section, T, DebrisTypes, VoxelAnimTypeClass);

	return 0;
}

// == Map Scripting ==
DEFINE_HOOK(7274AF, TriggerTypeClass_LoadFromINI_Read_Events, 5)
{
	R->Stack(0x0, Ares::readBuffer);
	R->Stack(0x4, Ares::readLength);
	return 0;
}

DEFINE_HOOK(7274C8, TriggerTypeClass_LoadFromINI_Strtok_Events, 5)
{
	R->ECX(Ares::readBuffer);
	return 0;
}


DEFINE_HOOK(727529, TriggerTypeClass_LoadFromINI_Read_Actions, 5)
{
	R->Stack(0x0, Ares::readBuffer);
	R->Stack(0x4, Ares::readLength);
	return 0;
}

DEFINE_HOOK(727544, TriggerTypeClass_LoadFromINI_Strtok_Actions, 5)
{
	R->EDX(Ares::readBuffer);
	return 0;
}
