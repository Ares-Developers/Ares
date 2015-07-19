#include "Reveal.h"
#include "../MapRevealer.h"
#include "../../Ares.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/TemplateDef.h"

#include <algorithm>

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
		// real default values, that is, force max cellspread range of 10
		auto const radius = std::min(RulesClass::Instance->PsychicRevealRadius, 10);
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

void SW_Reveal::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	pSW->Action = (GetRange(pData).WidthOrRange < 0.0) ? Action::None : Actions::SuperWeaponAllowed;
}

bool SW_Reveal::Activate(SuperClass* const pThis, const CellStruct &Coords, bool const IsPlayer)
{
	auto const pSW = pThis->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pSW);
	
	if(pThis->IsCharged) {
		MapRevealer const revealer(Coords);

		if(revealer.AffectsHouse(pThis->Owner)) {
			auto Apply = [=, &revealer](bool add) {
				auto const range = GetRange(pData);

				if(range.WidthOrRange < 0.0) {
					// reveal all cells without hundred thousands function calls
					auto const Map = MapClass::Instance;
					Map->CellIteratorReset();
					while(auto const pCell = Map->CellIteratorNext()) {
						if(revealer.IsCellAvailable(pCell->MapCoords) && revealer.IsCellAllowed(pCell->MapCoords)) {
							revealer.Process1(pCell, false, add);
						}
					}
				} else {
					// default way to reveal, but reveal one cell at a time.
					auto const& base = revealer.Base();

					Helpers::Alex::for_each_in_rect_or_range<CellClass>(base, range.WidthOrRange, range.Height,
						[=, &revealer](CellClass* pCell) -> bool
					{
						auto const& cell = pCell->MapCoords;
						if(revealer.IsCellAvailable(cell) && revealer.IsCellAllowed(cell)) {
							if(range.height() > 0 || cell.DistanceFrom(base) < range.range()) {
								revealer.Process1(pCell, false, add);
							}
						}
						return true;
					});
				}
			};

			Apply(false);
			Apply(true);

			MapClass::Instance->MarkNeedsRedraw(1);
		}
	}

	return true;
}
