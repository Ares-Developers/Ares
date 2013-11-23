#include "SonarPulse.h"
#include "../../Ext/Techno/Body.h"
#include "../../Utilities/Helpers.Alex.h"

SuperWeaponFlags::Value SW_SonarPulse::Flags()
{
	return SuperWeaponFlags::NoEvent;
}

void SW_SonarPulse::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// some defaults
	pData->SW_WidthOrRange = 10;
	pData->SW_Height = -1;
	pData->SW_RadarEvent = false;

	pData->Sonar_Delay = 60;

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::Stealth;
	pData->SW_AffectsHouse = SuperWeaponAffectedHouse::Enemies;
	pData->SW_AffectsTarget = SuperWeaponTarget::Water;
	pData->SW_RequiresTarget = SuperWeaponTarget::Water;
}

void SW_SonarPulse::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	pData->Sonar_Delay = pINI->ReadInteger(section, "SonarPulse.Delay", pData->Sonar_Delay);

	// full map detection?
	if(pData->SW_WidthOrRange < 0) {
		pSW->Action = 0;
	}
}

bool SW_SonarPulse::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	SuperWeaponTypeClass *pType = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

	if(!pData) {
		return false;
	}

	auto Detect = [&](TechnoClass* pTechno) -> bool {
		// is this thing affected at all?
		if(!pData->IsHouseAffected(pThis->Owner, pTechno->Owner)) {
			return true;
		}

		if(!pData->IsTechnoAffected(pTechno)) {
			return true;
		}

		// actually detect this
		if(pTechno->CloakState) {
			pTechno->Uncloak(1);
			pTechno->NeedsRedraw = 1;
			TechnoExt::ExtData *pExt = TechnoExt::ExtMap.Find(pTechno);
			if(pTechno) {
				pExt->CloakSkipTimer.Start(pData->Sonar_Delay);
			}
		}

		return true;
	};

	float width = pData->SW_WidthOrRange;
	int height = pData->SW_Height;

	if(width < 0) {
		// decloak everything regardless of ranges
		for(int i=0; i<TechnoClass::Array->Count; ++i) {
			Detect(TechnoClass::Array->GetItem(i));
		}

	} else {
		// decloak everything in range
		Helpers::Alex::DistinctCollector<TechnoClass*> items;
		Helpers::Alex::for_each_in_rect_or_range<TechnoClass>(*pCoords, width, height, std::ref(items));
		items.for_each(Detect);

		// radar event only if this isn't full map sonar
		if(pData->SW_RadarEvent.Get()) {
			RadarEventClass::Create(RadarEventType::SuperweaponActivated, *pCoords);
		}
	}

	return true;
}
