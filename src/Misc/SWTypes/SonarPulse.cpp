#include "SonarPulse.h"
#include "../../Ext/Techno/Body.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/TemplateDef.h"

#include <RadarEventClass.h>

SuperWeaponFlags SW_SonarPulse::Flags() const
{
	return SuperWeaponFlags::NoEvent;
}

SWRange SW_SonarPulse::GetRange(const SWTypeExt::ExtData* pData) const
{
	if(pData->SW_Range.empty()) {
		return SWRange(10);
	}
	return pData->SW_Range;
}

void SW_SonarPulse::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// some defaults
	pData->SW_RadarEvent = false;

	pData->Sonar_Delay = 60;

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::Stealth;
	pData->SW_AffectsHouse = SuperWeaponAffectedHouse::Enemies;
	pData->SW_AffectsTarget = SuperWeaponTarget::Water;
	pData->SW_RequiresTarget = SuperWeaponTarget::Water;
	pData->SW_AIRequiresTarget = SuperWeaponTarget::Water;
}

void SW_SonarPulse::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	pData->Sonar_Delay = pINI->ReadInteger(section, "SonarPulse.Delay", pData->Sonar_Delay);

	// full map detection?
	pSW->Action = (GetRange(pData).WidthOrRange < 0) ? Action::None : Actions::SuperWeaponAllowed;
}

bool SW_SonarPulse::Activate(SuperClass* const pThis, const CellStruct &Coords, bool const IsPlayer)
{
	auto const pType = pThis->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pType);

	if(!pData) {
		return false;
	}

	auto Detect = [pThis, pData](TechnoClass* const pTechno) -> bool {
		// is this thing affected at all?
		if(!pData->IsHouseAffected(pThis->Owner, pTechno->Owner)) {
			return true;
		}

		if(!pData->IsTechnoAffected(pTechno)) {
			return true;
		}

		auto const pExt = TechnoExt::ExtMap.Find(pTechno);
		auto const delay = Math::max(
			pExt->CloakSkipTimer.GetTimeLeft(), pData->Sonar_Delay);
		pExt->CloakSkipTimer.Start(delay);

		// actually detect this
		if(pTechno->CloakState != CloakState::Uncloaked) {
			pTechno->Uncloak(true);
			pTechno->NeedsRedraw = true;
		}

		return true;
	};

	auto const range = GetRange(pData);

	if(range.WidthOrRange < 0) {
		// decloak everything regardless of ranges
		for(auto const& pTechno : *TechnoClass::Array) {
			Detect(pTechno);
		}

	} else {
		// decloak everything in range
		using namespace Helpers::Alex;
		DistinctCollector<TechnoClass*> items;
		for_each_in_rect_or_range<TechnoClass>(Coords, range.WidthOrRange, range.Height, items);
		items.for_each(Detect);

		// radar event only if this isn't full map sonar
		if(pData->SW_RadarEvent) {
			RadarEventClass::Create(RadarEventType::SuperweaponActivated, Coords);
		}
	}

	return true;
}
