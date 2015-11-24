#include "LightningStorm.h"
#include "../../Ares.h"
#include "../../Utilities/TemplateDef.h"

#include <ScenarioClass.h>
#include <VoxClass.h>

SuperClass* SW_LightningStorm::CurrentLightningStorm = nullptr;

bool SW_LightningStorm::HandlesType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::LightningStorm);
}

SuperWeaponFlags SW_LightningStorm::Flags() const
{
	return SuperWeaponFlags::NoMessage | SuperWeaponFlags::NoEvent;
}

WarheadTypeClass* SW_LightningStorm::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Warhead.Get(RulesClass::Instance->LightningWarhead);
}

int SW_LightningStorm::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->LightningDamage);
}

SWRange SW_LightningStorm::GetRange(const SWTypeExt::ExtData* pData) const
{
	if(pData->SW_Range.empty()) {
		return SWRange(RulesClass::Instance->LightningCellSpread);
	}
	return pData->SW_Range;
}

void SW_LightningStorm::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Defaults to Lightning Storm values
	pData->Weather_DebrisMin = 2;
	pData->Weather_DebrisMax = 4;
	pData->Weather_IgnoreLightningRod = false;
	pData->Weather_ScatterCount = 1;

	pData->Weather_RadarOutageAffects = SuperWeaponAffectedHouse::Enemies;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_WeatherDeviceReady");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_LightningStormReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_LightningStormCreated");

	pData->Message_Launch = "TXT_LIGHTNING_STORM_APPROACHING";
	pData->Message_Activate = "TXT_LIGHTNING_STORM";
	pData->Message_Abort = "Msg:LightningStormActive";

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::LightningStorm;
	pData->SW_Cursor = MouseCursor::GetCursor(MouseCursorType::LightningStorm);
}

void SW_LightningStorm::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->Weather_Duration.Read(exINI, section, "Lightning.Duration");
	pData->Weather_RadarOutage.Read(exINI, section, "Lightning.RadarOutage");
	pData->Weather_HitDelay.Read(exINI, section, "Lightning.HitDelay");
	pData->Weather_ScatterDelay.Read(exINI, section, "Lightning.ScatterDelay");
	pData->Weather_ScatterCount.Read(exINI, section, "Lightning.ScatterCount");
	pData->Weather_Separation.Read(exINI, section, "Lightning.Separation");
	pData->Weather_PrintText.Read(exINI, section, "Lightning.PrintText");
	pData->Weather_IgnoreLightningRod.Read(exINI, section, "Lightning.IgnoreLightningRod");
	pData->Weather_DebrisMin.Read(exINI, section, "Lightning.DebrisMin");
	pData->Weather_DebrisMax.Read(exINI, section, "Lightning.DebrisMax");
	pData->Weather_CloudHeight.Read(exINI, section, "Lightning.CloudHeight");
	pData->Weather_BoltExplosion.Read(exINI, section, "Lightning.BoltExplosion");
	pData->Weather_RadarOutageAffects.Read(exINI, section, "Lightning.RadarOutageAffects");
	pData->Weather_Clouds.Read(exINI, section, "Lightning.Clouds");
	pData->Weather_Bolts.Read(exINI, section, "Lightning.Bolts");
	pData->Weather_Debris.Read(exINI, section, "Lightning.Debris");
	pData->Weather_Sounds.Read(exINI, section, "Lightning.Sounds");
}

bool SW_LightningStorm::AbortFire(SuperClass* pSW, bool IsPlayer)
{
	// only one Lightning Storm allowed
	if(LightningStorm::Active || LightningStorm::HasDeferment()) {
		if(IsPlayer) {
			SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW->Type);
			pData->PrintMessage(pData->Message_Abort, pSW->Owner);
		}
		return true;
	}
	return false;
}

bool SW_LightningStorm::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	if(pThis->IsCharged) {
		// the only thing we do differently is to remember which
		// SW has been fired here. all needed changes are done
		// by hooks.
		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type)) {
			CurrentLightningStorm = pThis;
			auto duration = pData->Weather_Duration.Get(RulesClass::Instance->LightningStormDuration);
			auto deferment = pData->SW_Deferment.Get(RulesClass::Instance->LightningDeferment);
			LightningStorm::Start(duration, deferment, Coords, pThis->Owner);
			return true;
		}
	}
	return false;
}
