#include "Reveal.h"
#include "../../Ares.h"
#include "../../Utilities/Helpers.Alex.h"

bool SW_Reveal::HandlesType(int type)
{
	return (type == SuperWeaponType::PsychicReveal);
}

void SW_Reveal::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	pData->SW_WidthOrRange = (float)RulesClass::Instance->PsychicRevealRadius;
	pData->SW_Sound = RulesClass::Instance->PsychicRevealActivateSound;

	if(pSW->Type == SuperWeaponType::PsychicReveal) {
		// real default values, that is, force max cellspread range of 10
		pData->SW_WidthOrRange = std::max<float>(pData->SW_WidthOrRange, 10);
	}

	pData->EVA_Ready = VoxClass::FindIndex("EVA_PsychicRevealReady");
	
	pData->SW_AITargetingType = SuperWeaponAITargetingMode::ParaDrop;
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::PsychicReveal];
}

bool SW_Reveal::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);
	
	if(pThis->IsCharged) {
		CellClass *pTarget = MapClass::Instance->GetCellAt(pCoords);
		
		CoordStruct Crd;
		pTarget->GetCoords(&Crd);

		float width = pData->SW_WidthOrRange;
		int height = pData->SW_Height;

		// default way to reveal, but reveal one cell at a time.
		Helpers::Alex::forEachCellInRange(pCoords, width, height,
			[&](CellClass* pCell) -> bool {
				CoordStruct Crd2;
				pCell->GetCoords(&Crd2);
				MapClass::Instance->RevealArea2(&Crd2, 1, pThis->Owner, 0, 0, 0, 0, 0);
				MapClass::Instance->RevealArea2(&Crd2, 1, pThis->Owner, 0, 0, 0, 0, 1);
				return true;
			}
		);
		
		//bool isRectangle = ((height > 0) && ((height * width) > 0));

		//// is this a circular range?
		//// then the radius goes in every direction.
		//if(!isRectangle) {
		//	width *= 2;
		//	height = width;
		//}

		//// the cursor marks the center of the area
		//CellStruct Offset;
		//Offset.X = (short)(pCoords->X - (width / 2));
		//Offset.Y = (short)(pCoords->Y - (height / 2));

		//// take a look at every cell in the rectangle
		//for(int i=0; i<(width*height); ++i) {
		//	// get the specific cell coordinates
		//	CellStruct Cell;
		//	Cell.X = (short)(i % width);
		//	Cell.Y = (short)(i / width);
		//	Cell += Offset;

		//	if(!isRectangle) {
		//		// only if cell is in range
		//		if(pCoords->DistanceFrom(Cell) > pData->SW_WidthOrRange.Get()) {
		//			continue;
		//		}
		//	}

		//	CellClass* pCell = MapClass::Instance->GetCellAt(&Cell);
		//	CoordStruct Crd2;
		//	pCell->GetCoords(&Crd2);

		//	// default way to reveal, but reveal one cell at a time.
		//	if(pCell != MapClass::InvalidCell()) {
		//		MapClass::Instance->RevealArea2(&Crd2, 1, pThis->Owner, 0, 0, 0, 0, 0);
		//		MapClass::Instance->RevealArea2(&Crd2, 1, pThis->Owner, 0, 0, 0, 0, 1);
		//	}
		//}
	}

	return true;
}