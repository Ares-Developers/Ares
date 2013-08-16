#include "LightningStorm.h"
#include "../../Ares.h"

#include <ScenarioClass.h>

SuperClass* SW_LightningStorm::CurrentLightningStorm = NULL;

bool SW_LightningStorm::HandlesType(int type)
{
	return (type == SuperWeaponType::LightningStorm);
}

SuperWeaponFlags::Value SW_LightningStorm::Flags()
{
	return SuperWeaponFlags::NoMessage | SuperWeaponFlags::NoEvent;
}

void SW_LightningStorm::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Defaults to Lightning Storm values
	pData->SW_WidthOrRange = (float)RulesClass::Instance->LightningCellSpread;
	pData->SW_Damage = RulesClass::Instance->LightningDamage;
	pData->SW_Deferment = RulesClass::Instance->LightningDeferment;
	pData->SW_Warhead = &RulesClass::Instance->LightningWarhead;
	pData->SW_ActivationSound = RulesClass::Instance->StormSound;

	pData->Weather_DebrisMin = 2;
	pData->Weather_DebrisMax = 4;
	pData->Weather_IgnoreLightningRod = false;
	pData->Weather_ScatterCount = 1;

	pData->Weather_BoltExplosion = RulesClass::Instance->WeatherConBoltExplosion;
	pData->Weather_Duration = RulesClass::Instance->LightningStormDuration;
	pData->Weather_RadarOutage = RulesClass::Instance->LightningStormDuration;
	pData->Weather_HitDelay = RulesClass::Instance->LightningHitDelay;
	pData->Weather_ScatterDelay = RulesClass::Instance->LightningScatterDelay;
	pData->Weather_Separation = RulesClass::Instance->LightningSeparation;
	pData->Weather_PrintText = RulesClass::Instance->LightningPrintText;

	pData->Weather_RadarOutageAffects = SuperWeaponAffectedHouse::Enemies;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_WeatherDeviceReady");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_LightningStormReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_LightningStormCreated");

	pData->Message_Launch = CSFText("TXT_LIGHTNING_STORM_APPROACHING");
	pData->Message_Activate = CSFText("TXT_LIGHTNING_STORM");
	pData->Message_Abort = CSFText("Msg:LightningStormActive");

	pData->Lighting_Ambient = &ScenarioClass::Instance->IonAmbient;
	pData->Lighting_Red = &ScenarioClass::Instance->IonRed;
	pData->Lighting_Green = &ScenarioClass::Instance->IonGreen;
	pData->Lighting_Blue = &ScenarioClass::Instance->IonBlue;

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::LightningStorm;
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::LightningStorm];
}

void SW_LightningStorm::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->Weather_Duration.Read(&exINI, section, "Lightning.Duration");
	pData->Weather_RadarOutage.Read(&exINI, section, "Lightning.RadarOutage");
	pData->Weather_HitDelay.Read(&exINI, section, "Lightning.HitDelay");
	pData->Weather_ScatterDelay.Read(&exINI, section, "Lightning.ScatterDelay");
	pData->Weather_ScatterCount.Read(&exINI, section, "Lightning.ScatterCount");
	pData->Weather_Separation.Read(&exINI, section, "Lightning.Separation");
	pData->Weather_PrintText.Read(&exINI, section, "Lightning.PrintText");
	pData->Weather_IgnoreLightningRod.Read(&exINI, section, "Lightning.IgnoreLightningRod");
	pData->Weather_DebrisMin.Read(&exINI, section, "Lightning.DebrisMin");
	pData->Weather_DebrisMax.Read(&exINI, section, "Lightning.DebrisMax");
	pData->Weather_CloudHeight.Read(&exINI, section, "Lightning.CloudHeight");
	pData->Weather_BoltExplosion.Parse(&exINI, section, "Lightning.BoltExplosion");
	pData->Weather_RadarOutageAffects.Read(&exINI, section, "Lightning.RadarOutageAffects");
	pData->Weather_Clouds.Read(&exINI, section, "Lightning.Clouds");
	pData->Weather_Bolts.Read(&exINI, section, "Lightning.Bolts");
	pData->Weather_Debris.Read(&exINI, section, "Lightning.Debris");
	pData->Weather_Sounds.Read(&exINI, section, "Lightning.Sounds");
}

bool SW_LightningStorm::AbortFire(SuperClass* pSW, bool IsPlayer) {
	// only one Lightning Storm allowed
	if(LightningStorm::Active() || LightningStorm::HasDeferment()) {
		if(IsPlayer) {
			SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW->Type);
			pData->PrintMessage(pData->Message_Abort, pSW->Owner);
		}
		return true;
	}
	return false;
}

bool SW_LightningStorm::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	if(pThis->IsCharged) {
		// the only thing we do differently is to remember which
		// SW has been fired here. all needed changes are done
		// by hooks.
		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type)) {
			CurrentLightningStorm = pThis;
			LightningStorm::Start(pData->Weather_Duration, pData->SW_Deferment, *pCoords, pThis->Owner);
			return true;
		}
	}
	return false;
}