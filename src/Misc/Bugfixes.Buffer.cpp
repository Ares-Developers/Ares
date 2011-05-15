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
#include <SidebarClass.h>
#include <StringTable.h>
#include <TechnoClass.h>
#include <TemporalClass.h>
#include <TerrainTypeClass.h>
#include <UnitTypeClass.h>
#include <WarheadTypeClass.h>

#include "Debug.h"
#include "../Ares.h"
#include "../Ares.CRT.h"

#include "../Utilities/Macro.h"

template<typename T>
static void ParseList(DynamicVectorClass<T> &List, CCINIClass * pINI, const char *section, const char *key) {
	if(pINI->ReadString(section, key, Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		List.Clear();
		for(char *cur = strtok(Ares::readBuffer, Ares::readDelims); cur; cur = strtok(NULL, Ares::readDelims)) {
			if(auto idx = CompoundT<T>::BaseT::Find(cur)) {
				List.AddItem(idx);
			} else {
				Debug::INIParseFailed(section, key, cur);
			}
		}
	}
};

#define PARSE_LIST(obj, key) \
	ParseList(obj->key, pINI, section, #key);

#define PARSE_RULES_LIST(key) \
	PARSE_LIST(pRules, key);


template<>
static void ParseList<int>(DynamicVectorClass<int> &List, CCINIClass * pINI, const char *section, const char *key) {
	if(pINI->ReadString(section, key, Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		List.Clear();
		for(char *cur = strtok(Ares::readBuffer, Ares::readDelims); cur; cur = strtok(NULL, Ares::readDelims)) {
			int idx = atoi(cur);
			List.AddItem(idx);
		}
	}
};



template<typename T>
static void ParseListIndices(DWORD &Composite, CCINIClass * pINI, const char *section, const char *key) {
	if(pINI->ReadString(section, key, Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		Composite = 0;
		for(char *cur = strtok(Ares::readBuffer, Ares::readDelims); cur; cur = strtok(NULL, Ares::readDelims)) {
			int idx = CompoundT<T>::BaseT::FindIndex(cur);
			if(idx > -1) {
				Composite |= (1 << idx);
			} else {
				Debug::INIParseFailed(section, key, cur);
			}
		}
	}
};

#define PARSE_INDICES(obj, key) \
	ParseListIndices(obj->key, pINI, section, #key);

#define PARSE_COUNTRY_INDICES(obj, key) \
	ParseListIndices<HouseTypeClass *>(obj->key, pINI, section, #key);

#define PARSE_COUNTRY_INDICES_INTO(obj, key, attr, type) \
	ParseListIndices<HouseTypeClass *>(obj->attr, pINI, section, #key);

/* issue 193 - increasing the buffer length for certain flag parsing */

DEFINE_HOOK(511D16, Buf_CountryVeteran, 9)
{
	GET(HouseTypeClass *, H, EBX);
	GET(CCINIClass *, pINI, ESI);

	const char *section = H->ID;
	PARSE_LIST(H, VeteranInfantry);
	PARSE_LIST(H, VeteranUnits);
	PARSE_LIST(H, VeteranAircraft);

	return 0x51208C;
}

// one hook to overwrite all lists and in the sequence skip them

// ============= [General] =============
DEFINE_HOOK(66D55E, Buf_General, 6)
{
	GET(RulesClass *, pRules, ESI);
	GET(CCINIClass *, pINI, EDI);

	const char *section = "General";
	PARSE_RULES_LIST(AmerParaDropInf);
	PARSE_RULES_LIST(AllyParaDropInf);
	PARSE_RULES_LIST(SovParaDropInf);
	PARSE_RULES_LIST(YuriParaDropInf);

	PARSE_RULES_LIST(AmerParaDropNum);
	PARSE_RULES_LIST(AllyParaDropNum);
	PARSE_RULES_LIST(SovParaDropNum);
	PARSE_RULES_LIST(YuriParaDropNum);

	PARSE_RULES_LIST(AnimToInfantry);

	PARSE_RULES_LIST(SecretInfantry);
	PARSE_RULES_LIST(SecretUnits);
	PARSE_RULES_LIST(SecretBuildings);
	pRules->SecretSum = pRules->SecretInfantry.Count
		+ pRules->SecretUnits.Count
		+ pRules->SecretBuildings.Count;

	PARSE_RULES_LIST(HarvesterUnit);
	PARSE_RULES_LIST(BaseUnit);
	PARSE_RULES_LIST(PadAircraft);

	PARSE_RULES_LIST(Shipyard);
	PARSE_RULES_LIST(RepairBay);

	PARSE_RULES_LIST(WeatherConClouds);
	PARSE_RULES_LIST(WeatherConBolts);
	PARSE_RULES_LIST(BridgeExplosions);

	PARSE_RULES_LIST(DefaultMirageDisguises);
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
	GET(RulesClass *, pRules, ESI);
	GET(CCINIClass *, pINI, EDI);

	pRules->TiberiumStrength = R->EAX();

	const char *section = "CombatDamage";
	PARSE_RULES_LIST(Scorches);
	PARSE_RULES_LIST(Scorches1);
	PARSE_RULES_LIST(Scorches2);
	PARSE_RULES_LIST(Scorches3);
	PARSE_RULES_LIST(Scorches4);

	PARSE_RULES_LIST(SplashList);
	return 0x66C287;
}

// ============= [AI] =============

DEFINE_HOOK(672B0E, Buf_AI, 6)
{
	GET(RulesClass *, pRules, ESI);
	GET(CCINIClass *, pINI, EDI);

	const char *section = "AI";
	PARSE_RULES_LIST(BuildConst);
	PARSE_RULES_LIST(BuildPower);
	PARSE_RULES_LIST(BuildRefinery);
	PARSE_RULES_LIST(BuildBarracks);
	PARSE_RULES_LIST(BuildTech);
	PARSE_RULES_LIST(BuildWeapons);
	PARSE_RULES_LIST(AlliedBaseDefenses);
	PARSE_RULES_LIST(SovietBaseDefenses);
	PARSE_RULES_LIST(ThirdBaseDefenses);
	PARSE_RULES_LIST(BuildDefense);
	PARSE_RULES_LIST(BuildPDefense);
	PARSE_RULES_LIST(BuildAA);
	PARSE_RULES_LIST(BuildHelipad);
	PARSE_RULES_LIST(BuildRadar);
	PARSE_RULES_LIST(ConcreteWalls);
	PARSE_RULES_LIST(NSGates);
	PARSE_RULES_LIST(EWGates);
	PARSE_RULES_LIST(BuildNavalYard);
	PARSE_RULES_LIST(BuildDummy);
	PARSE_RULES_LIST(NeutralTechBuildings);

	PARSE_RULES_LIST(AIForcePredictionFudge);

	return 0x673950;
}


// == TechnoType ==
DEFINE_HOOK(7121A3, Buf_TechnoType, 6)
{
	GET(TechnoTypeClass *, T, EBP);
	GET(const char *, section, EBX);
	GET(CCINIClass *, pINI, ESI);

	PARSE_COUNTRY_INDICES(T, ForbiddenHouses);
	PARSE_COUNTRY_INDICES(T, SecretHouses);
	PARSE_COUNTRY_INDICES(T, RequiredHouses);
	PARSE_COUNTRY_INDICES_INTO(T, Owner, OwnerFlags, HouseTypeClass);

	PARSE_LIST(T, DamageParticleSystems);
	PARSE_LIST(T, DestroyParticleSystems);

	PARSE_LIST(T, Dock);

// TODO: define VoxelAnimTypeClass
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

DEFINE_HOOK(713171, Buf_Dock, 9)
{
	GET(TechnoTypeClass *, T, EBP);
	GET(int, Category, EAX);

	T->Category = Category;
	return 0x713264;
}

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
	GET(CCINIClass *, pINI, EDI);

	PARSE_LIST(WH, AnimList);

// TODO: define VoxelAnimTypeClass
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


DEFINE_HOOK(6A9348, CameoClass_GetTip_FixLength, 9)
{
	DWORD HideObjectName = R->AL();

	GET(TechnoTypeClass *, Object, ESI);

	int Cost = Object->GetActualCost(HouseClass::Player);
	if(HideObjectName) {
		const wchar_t * Format = StringTable::LoadString("TXT_MONEY_FORMAT_1");
		_snwprintf(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, Format, Cost);
	} else {
		const wchar_t * UIName = Object->UIName;
		const wchar_t * Format = StringTable::LoadString("TXT_MONEY_FORMAT_2");
		_snwprintf(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, Format, UIName, Cost);
	}
	SidebarClass::TooltipBuffer[SidebarClass::TooltipLength - 1] = 0;

	return 0x6A93B2;
}
