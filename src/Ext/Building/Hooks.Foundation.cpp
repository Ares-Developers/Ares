#include "Body.h"
#include "../BuildingType/Body.h"

#include <algorithm>
#include <vector>

CellStruct * BuildingExt::TempFoundationData1 = NULL;
CellStruct * BuildingExt::TempFoundationData2 = NULL;

DEFINE_HOOK(0x45EC90, Foundations_GetFoundationWidth, 0x6)
{
	GET(BuildingTypeClass*, pThis, ECX);
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

	if(pData->IsCustom) {
		R->EAX(pData->CustomWidth);
		return 0x45EC9D;
	}

	return 0;
}

DEFINE_HOOK(0x45ECA0, Foundations_GetFoundationHeight, 0x6)
{
	GET(BuildingTypeClass*, pThis, ECX);
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

	if(pData->IsCustom) {
		bool bIncludeBib = (R->Stack8(0x4) != 0);

		int fH = pData->CustomHeight;
		if(bIncludeBib && pThis->Bib) {
			++fH;
		}

		R->EAX(fH);
		return 0x45ECDA;
	}

	return 0;
}

DEFINE_HOOK(0x568411, MapClass_AddContentAt_Foundation_P1, 0x0)
{
	GET(BuildingClass *, pThis, EDI);

	R->EBP<CellStruct *>(pThis->GetFoundationData(false));

	return 0x568432;
}

DEFINE_HOOK(0x568565, MapClass_AddContentAt_Foundation_OccupyHeight, 0x5)
{
	GET(BuildingClass *, pThis, EDI);
	GET(int, ShadowHeight, EBP);
	GET_STACK(CellStruct *, MainCoords, 0x8B4);

	CellStruct * Foundation = pThis->GetFoundationData(false);
	DWORD Len = BuildingExt::FoundationLength(Foundation);

	std::vector<CellStruct> AffectedCells;

	AffectedCells.reserve(Len * ShadowHeight);

	CellStruct * CurrentFCell = Foundation;

	while(CurrentFCell->X != 32767 && CurrentFCell->Y != 32767) {
		CellStruct ActualCell = *MainCoords + *CurrentFCell;
		for(int i = ShadowHeight; i > 0; --i) {
			AffectedCells.push_back(ActualCell);
			--ActualCell.X;
			--ActualCell.Y;
		}
		++CurrentFCell;
	}

	std::sort(AffectedCells.begin(), AffectedCells.end(), [](const CellStruct &lhs, const CellStruct &rhs) -> bool {
		return lhs.X > rhs.X || lhs.X == rhs.X && lhs.Y > rhs.Y;
	});
	auto it = std::unique(AffectedCells.begin(), AffectedCells.end());
	AffectedCells.resize(it - AffectedCells.begin());

	std::for_each(AffectedCells.begin(), AffectedCells.end(), [pThis](CellStruct coords) {
		CellClass * Cell = MapClass::Instance->GetCellAt(&coords);
		++Cell->OccupyHeightsCoveringMe;
	});

	return 0x568697;
}

DEFINE_HOOK(0x568841, MapClass_RemoveContentAt_Foundation_P1, 0x0)
{
	GET(BuildingClass *, pThis, EDI);

	R->EBP<CellStruct *>(pThis->GetFoundationData(false));

	return 0x568862;
}

DEFINE_HOOK(0x568997, MapClass_RemoveContentAt_Foundation_OccupyHeight, 0x5)
{
	GET(BuildingClass *, pThis, EDX);
	GET(int, ShadowHeight, EBP);
	GET_STACK(CellStruct *, MainCoords, 0x8B4);

	CellStruct * Foundation = pThis->GetFoundationData(false);
	DWORD Len = BuildingExt::FoundationLength(Foundation);

	std::vector<CellStruct> AffectedCells;

	AffectedCells.reserve(Len * ShadowHeight);

	CellStruct * CurrentFCell = Foundation;

	while(CurrentFCell->X != 32767 && CurrentFCell->Y != 32767) {
		CellStruct ActualCell = *MainCoords + *CurrentFCell;
		for(int i = ShadowHeight; i > 0; --i) {
			AffectedCells.push_back(ActualCell);
			--ActualCell.X;
			--ActualCell.Y;
		}
		++CurrentFCell;
	}

	std::sort(AffectedCells.begin(), AffectedCells.end(), [](const CellStruct &lhs, const CellStruct &rhs) -> bool {
		return lhs.X > rhs.X || lhs.X == rhs.X && lhs.Y > rhs.Y;
	});
	auto it = std::unique(AffectedCells.begin(), AffectedCells.end());
	AffectedCells.resize(it - AffectedCells.begin());

	std::for_each(AffectedCells.begin(), AffectedCells.end(), [pThis](CellStruct coords) {
		CellClass * Cell = MapClass::Instance->GetCellAt(&coords);
		if(Cell->OccupyHeightsCoveringMe > 0) {
			--Cell->OccupyHeightsCoveringMe;
		}
	});

	return 0x568ADC;
}


DEFINE_HOOK(0x4A8C77, MapClass_ProcessFoundation1_UnlimitBuffer, 0x5)
{
	GET_STACK(CellStruct *, Foundation, 0x18);
	GET(DisplayClass *, Display, EBX);

	if(BuildingExt::TempFoundationData1) {
		delete[] BuildingExt::TempFoundationData1;
	}

	DWORD Len = BuildingExt::FoundationLength(Foundation);

	BuildingExt::TempFoundationData1 = new CellStruct[Len];
	memcpy(BuildingExt::TempFoundationData1, Foundation, Len * sizeof(CellStruct));

	Display->CurrentFoundation_Data = BuildingExt::TempFoundationData1;

	CellStruct bounds;
	Display->FoundationBoundsSize(&bounds, BuildingExt::TempFoundationData1);
	R->Stack<CellStruct>(0x18, bounds);
	R->EAX<CellStruct *>(R->lea_Stack<CellStruct *>(0x18));

	return 0x4A8C9E;
}

DEFINE_HOOK(0x4A8DD7, MapClass_ProcessFoundation2_UnlimitBuffer, 0x5)
{
	GET_STACK(CellStruct *, Foundation, 0x18);
	GET(DisplayClass *, Display, EBX);

	if(BuildingExt::TempFoundationData2) {
		delete[] BuildingExt::TempFoundationData2;
	}

	DWORD Len = BuildingExt::FoundationLength(Foundation);

	BuildingExt::TempFoundationData2 = new CellStruct[Len];
	memcpy(BuildingExt::TempFoundationData2, Foundation, Len * sizeof(CellStruct));

	Display->CurrentFoundationCopy_Data = BuildingExt::TempFoundationData2;

	CellStruct bounds;
	Display->FoundationBoundsSize(&bounds, BuildingExt::TempFoundationData2);
	R->Stack<CellStruct>(0x18, bounds);
	R->EAX<CellStruct *>(R->lea_Stack<CellStruct *>(0x18));

	return 0x4A8DFE;
}
