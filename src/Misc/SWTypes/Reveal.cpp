#include "Reveal.h"
#include "../../Ares.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/TemplateDef.h"

bool SW_Reveal::HandlesType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::PsychicReveal);
}

int SW_Reveal::GetSound(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Sound.Get(RulesClass::Instance->PsychicRevealActivateSound);
}

SWRange SW_Reveal::GetRange(const SWTypeExt::ExtData* pData) const
{
	if(pData->SW_Range.empty()) {
		int radius = RulesClass::Instance->PsychicRevealRadius;

		// real default values, that is, force max cellspread range of 10
		if(radius > 10) {
			radius = 10;
		}

		return SWRange(radius);
	}
	return pData->SW_Range;
}

void SW_Reveal::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndex("EVA_PsychicRevealReady");

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::ParaDrop;
	pData->SW_Cursor = MouseCursor::GetCursor(MouseCursorType::PsychicReveal);
}

bool SW_Reveal::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);
	
	if(pThis->IsCharged) {
		CellClass *pTarget = MapClass::Instance->GetCellAt(Coords);
		
		CoordStruct Crd = pTarget->GetCoords();
		auto range = GetRange(pData);

		// default way to reveal, but reveal one cell at a time.
		Helpers::Alex::for_each_in_rect_or_range<CellClass>(Coords, range.WidthOrRange, range.Height,
			[&](CellClass* pCell) -> bool {
				CoordStruct Crd2 = pCell->GetCoords();
				MapClass::Instance->RevealArea2(&Crd2, 1, pThis->Owner, 0, 0, 0, 0, 0);
				MapClass::Instance->RevealArea2(&Crd2, 1, pThis->Owner, 0, 0, 0, 0, 1);
				return true;
			}
		);
	}

	return true;
}
