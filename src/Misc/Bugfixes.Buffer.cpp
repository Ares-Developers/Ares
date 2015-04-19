#include <vector>

#include <AircraftTypeClass.h>
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
#include <ScenarioClass.h>
#include <SmudgeTypeClass.h>
#include <SidebarClass.h>
#include <StringTable.h>
#include <TActionClass.h>
#include <TechnoClass.h>
#include <TemporalClass.h>
#include <TerrainTypeClass.h>
#include <UnitTypeClass.h>
#include <VocClass.h>
#include <WarheadTypeClass.h>
#include <VoxelAnimTypeClass.h>

#include "Debug.h"
#include "../Ares.h"
#include "../Ares.CRT.h"

#include "../Utilities/Macro.h"
#include "../Utilities/Parser.h"

#include <type_traits>

template<typename T>
static void ParseList(DynamicVectorClass<T> &List, CCINIClass * pINI, const char *section, const char *key) {
	if(pINI->ReadString(section, key, Ares::readDefval, Ares::readBuffer)) {
		List.Clear();

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
			T buffer = T();
			if(Parser<T>::TryParse(cur, &buffer)) {
				List.AddItem(buffer);
			} else if(!std::is_pointer<T>() || !INIClass::IsBlank(cur)) {
				Debug::INIParseFailed(section, key, cur);
			}
		}
	}
};

/* issue 193 - increasing the buffer length for certain flag parsing */

DEFINE_HOOK(511D16, Buf_CountryVeteran, 9)
{
	GET(HouseTypeClass *, H, EBX);
	GET(CCINIClass *, pINI, ESI);

	const char *section = H->ID;
	ParseList(H->VeteranInfantry, pINI, section, "VeteranInfantry");
	ParseList(H->VeteranUnits, pINI, section, "VeteranUnits");
	ParseList(H->VeteranAircraft, pINI, section, "VeteranAircraft");

	return 0x51208C;
}

// one hook to overwrite all lists and in the sequence skip them

// ============= [General] =============
DEFINE_HOOK(66D55E, Buf_General, 6)
{
	GET(RulesClass *, pRules, ESI);
	GET(CCINIClass *, pINI, EDI);

	const char *section = "General";
	ParseList(pRules->AmerParaDropInf, pINI, section, "AmerParaDropInf");
	ParseList(pRules->AllyParaDropInf, pINI, section, "AllyParaDropInf");
	ParseList(pRules->SovParaDropInf, pINI, section, "SovParaDropInf");
	ParseList(pRules->YuriParaDropInf, pINI, section, "YuriParaDropInf");

	ParseList(pRules->AmerParaDropNum, pINI, section, "AmerParaDropNum");
	ParseList(pRules->AllyParaDropNum, pINI, section, "AllyParaDropNum");
	ParseList(pRules->SovParaDropNum, pINI, section, "SovParaDropNum");
	ParseList(pRules->YuriParaDropNum, pINI, section, "YuriParaDropNum");

	ParseList(pRules->AnimToInfantry, pINI, section, "AnimToInfantry");

	ParseList(pRules->SecretInfantry, pINI, section, "SecretInfantry");
	ParseList(pRules->SecretUnits, pINI, section, "SecretUnits");
	ParseList(pRules->SecretBuildings, pINI, section, "SecretBuildings");
	pRules->SecretSum = pRules->SecretInfantry.Count
		+ pRules->SecretUnits.Count
		+ pRules->SecretBuildings.Count;

	ParseList(pRules->HarvesterUnit, pINI, section, "HarvesterUnit");
	ParseList(pRules->BaseUnit, pINI, section, "BaseUnit");
	ParseList(pRules->PadAircraft, pINI, section, "PadAircraft");

	ParseList(pRules->Shipyard, pINI, section, "Shipyard");
	ParseList(pRules->RepairBay, pINI, section, "RepairBay");

	ParseList(pRules->WeatherConClouds, pINI, section, "WeatherConClouds");
	ParseList(pRules->WeatherConBolts, pINI, section, "WeatherConBolts");
	ParseList(pRules->BridgeExplosions, pINI, section, "BridgeExplosions");

	ParseList(pRules->DefaultMirageDisguises, pINI, section, "DefaultMirageDisguises");
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

	Rules->NoParachuteMaxFallRate = R->EAX<int>();

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

	pRules->TiberiumStrength = R->EAX<int>();

	const char *section = "CombatDamage";
	ParseList(pRules->Scorches, pINI, section, "Scorches");
	ParseList(pRules->Scorches1, pINI, section, "Scorches1");
	ParseList(pRules->Scorches2, pINI, section, "Scorches2");
	ParseList(pRules->Scorches3, pINI, section, "Scorches3");
	ParseList(pRules->Scorches4, pINI, section, "Scorches4");

	ParseList(pRules->SplashList, pINI, section, "SplashList");
	return 0x66C287;
}

// ============= [AI] =============

DEFINE_HOOK(672B0E, Buf_AI, 6)
{
	GET(RulesClass *, pRules, ESI);
	GET(CCINIClass *, pINI, EDI);

	const char *section = "AI";
	ParseList(pRules->BuildConst, pINI, section, "BuildConst");
	ParseList(pRules->BuildPower, pINI, section, "BuildPower");
	ParseList(pRules->BuildRefinery, pINI, section, "BuildRefinery");
	ParseList(pRules->BuildBarracks, pINI, section, "BuildBarracks");
	ParseList(pRules->BuildTech, pINI, section, "BuildTech");
	ParseList(pRules->BuildWeapons, pINI, section, "BuildWeapons");
	ParseList(pRules->AlliedBaseDefenses, pINI, section, "AlliedBaseDefenses");
	ParseList(pRules->SovietBaseDefenses, pINI, section, "SovietBaseDefenses");
	ParseList(pRules->ThirdBaseDefenses, pINI, section, "ThirdBaseDefenses");
	ParseList(pRules->BuildDefense, pINI, section, "BuildDefense");
	ParseList(pRules->BuildPDefense, pINI, section, "BuildPDefense");
	ParseList(pRules->BuildAA, pINI, section, "BuildAA");
	ParseList(pRules->BuildHelipad, pINI, section, "BuildHelipad");
	ParseList(pRules->BuildRadar, pINI, section, "BuildRadar");
	ParseList(pRules->ConcreteWalls, pINI, section, "ConcreteWalls");
	ParseList(pRules->NSGates, pINI, section, "NSGates");
	ParseList(pRules->EWGates, pINI, section, "EWGates");
	ParseList(pRules->BuildNavalYard, pINI, section, "BuildNavalYard");
	ParseList(pRules->BuildDummy, pINI, section, "BuildDummy");
	ParseList(pRules->NeutralTechBuildings, pINI, section, "NeutralTechBuildings");

	ParseList(pRules->AIForcePredictionFudge, pINI, section, "AIForcePredictionFudge");

	return 0x673950;
}


// == TechnoType ==
DEFINE_HOOK(7125DF, TechnoTypeClass_LoadFromINI_ListLength, 7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(const char*, pSection, EBX);
	GET(CCINIClass*, pINI, ESI);

	ParseList(pThis->DamageParticleSystems, pINI, pSection, "DamageParticleSystems");
	ParseList(pThis->DestroyParticleSystems, pINI, pSection, "DestroyParticleSystems");

	ParseList(pThis->Dock, pINI, pSection, "Dock");

	ParseList(pThis->DebrisMaximums, pINI, pSection, "DebrisMaximums");
	ParseList(pThis->DebrisTypes, pINI, pSection, "DebrisTypes");
	ParseList(pThis->DebrisAnims, pINI, pSection, "DebrisAnims");

	return 0x712830;
}

DEFINE_HOOK(713171, TechnoTypeClass_LoadFromINI_SkipLists1, 9)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(Category, category, EAX);

	pThis->Category = category;

	return 0x713264;
}

DEFINE_HOOK(713C10, TechnoTypeClass_LoadFromINI_SkipLists2, 7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(const CoordStruct*, pResult, EAX);

	pThis->NaturalParticleSystemLocation = *pResult;

	return 0x713E1A;
}


// == WarheadType ==
DEFINE_HOOK(75D660, WarheadTypeClass_LoadFromINI_ListLength, 9)
{
	GET(WarheadTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBP);
	GET(CCINIClass*, pINI, EDI);

	ParseList(pThis->AnimList, pINI, pSection, "AnimList");

	ParseList(pThis->DebrisMaximums, pINI, pSection, "DebrisMaximums");
	ParseList(pThis->DebrisTypes, pINI, pSection, "DebrisTypes");

	return 0x75D75D;
}

DEFINE_HOOK(75DAE6, WarheadTypeClass_LoadFromINI_SkipLists, 4)
{
	return 0x75DDCC;
}


// == WeaponType ==
DEFINE_HOOK(772462, WeaponTypeClass_LoadFromINI_ListLength, 4)
{
	GET(WeaponTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBX);
	GET(CCINIClass*, pINI, EDI);

	ParseList(pThis->Anim, pINI, pSection, "Anim");

	return 0x77255F;
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

DEFINE_HOOK(4750EC, INIClass_ReadHouseTypesList, 7)
{
	R->Stack(0x0, Ares::readBuffer);
	R->Stack(0x4, Ares::readLength);
	return 0;
}

DEFINE_HOOK(475107, INIClass_ReadHouseTypesList_Strtok, 5)
{
	R->ECX(Ares::readBuffer);
	return 0;
}

DEFINE_HOOK(47527C, INIClass_GetAlliesBitfield, 7)
{
	R->Stack(0x0, Ares::readBuffer);
	R->Stack(0x4, Ares::readLength);
	return 0;
}

DEFINE_HOOK(475297, INIClass_GetAlliesBitfield_Strtok, 5)
{
	R->ECX(Ares::readBuffer);
	return 0;
}

DEFINE_HOOK(6A9348, CameoClass_GetTip_FixLength, 9)
{
	DWORD HideObjectName = R->AL();

	GET(TechnoTypeClass *, Object, ESI);

	int Cost = Object->GetActualCost(HouseClass::Player);
	if(HideObjectName) {
		const wchar_t * Format = StringTable::LoadString("TXT_MONEY_FORMAT_1");
		_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, Cost);
	} else {
		const wchar_t * UIName = Object->UIName;
		const wchar_t * Format = StringTable::LoadString("TXT_MONEY_FORMAT_2");
		_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, UIName, Cost);
	}
	SidebarClass::TooltipBuffer[SidebarClass::TooltipLength - 1] = 0;

	return 0x6A93B2;
}

DEFINE_HOOK(70CAD8, TechnoClass_DealParticleDamage_DontDestroyCliff, 9)
{
	return 0x70CB30;
}

DEFINE_HOOK(489562, DamageArea_DestroyCliff, 6)
{
	GET(CellClass *, pCell, EAX);

	if(pCell->Tile_Is_DestroyableCliff()) {
		if(ScenarioClass::Instance->Random.RandomRanged(0, 99) < RulesClass::Instance->CollapseChance) {
			MapClass::Instance->DestroyCliff(pCell);
		}
	}

	return 0;
}
