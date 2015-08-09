#include "Body.h"

#include "../BuildingType/Body.h"

#include <MouseClass.h>

DEFINE_HOOK(45EC90, Foundations_GetFoundationWidth, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

	if(pData->IsCustom) {
		R->EAX(pData->CustomWidth);
		return 0x45EC9D;
	}

	return 0;
}

DEFINE_HOOK(45ECA0, Foundations_GetFoundationHeight, 6)
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

DEFINE_HOOK(656584, MapClass_GetFoundationShape, 6)
{
	GET(RadarClass*, pThis, ECX);
	GET(BuildingTypeClass*, pType, EAX);

	auto fnd = pType->Foundation;
	DynamicVectorClass<Point2D>* ret = nullptr;

	if(fnd >= fnd_1x1 && fnd <= fnd_0x0) {
		// in range of default foundations
		ret = &pThis->FoundationTypePixels[fnd];
	} else if(auto pExt = BuildingTypeExt::ExtMap.Find(pType)) {
		// custom foundation
		ret = &pExt->FoundationRadarShape;
	} else {
		// default if everything fails
		ret = &pThis->FoundationTypePixels[fnd_2x2];
	}

	R->EAX(ret);
	return 0x656595;
}

DEFINE_HOOK(6563B0, RadarClass_UpdateFoundationShapes_Custom, 5)
{
	// update each building type foundation
	for(auto const& pType : *BuildingTypeClass::Array) {
		if(auto pExt = BuildingTypeExt::ExtMap.Find(pType)) {
			pExt->UpdateFoundationRadarShape();
		}
	}

	return 0;
}

DEFINE_HOOK(568411, MapClass_AddContentAt_Foundation_P1, 0)
{
	GET(BuildingClass *, pThis, EDI);

	R->EBP(pThis->GetFoundationData(false));

	return 0x568432;
}

DEFINE_HOOK(568565, MapClass_AddContentAt_Foundation_OccupyHeight, 5)
{
	GET(BuildingClass *, pThis, EDI);
	GET(int, ShadowHeight, EBP);
	GET_STACK(CellStruct *, MainCoords, 0x8B4);

	auto const& AffectedCells = BuildingExt::GetCoveredCells(
		pThis, *MainCoords, ShadowHeight);

	auto &Map = MapClass::Instance;

	for(auto const& cell : AffectedCells) {
		if(auto pCell = Map->TryGetCellAt(cell)) {
			++pCell->OccupyHeightsCoveringMe;
		}
	}

	return 0x568697;
}

DEFINE_HOOK(568841, MapClass_RemoveContentAt_Foundation_P1, 0)
{
	GET(BuildingClass *, pThis, EDI);

	R->EBP(pThis->GetFoundationData(false));

	return 0x568862;
}

DEFINE_HOOK(568997, MapClass_RemoveContentAt_Foundation_OccupyHeight, 5)
{
	GET(BuildingClass *, pThis, EDX);
	GET(int, ShadowHeight, EBP);
	GET_STACK(CellStruct *, MainCoords, 0x8B4);

	auto const& AffectedCells = BuildingExt::GetCoveredCells(
		pThis, *MainCoords, ShadowHeight);

	auto &Map = MapClass::Instance;

	for(auto const& cell : AffectedCells) {
		if(auto const pCell = Map->TryGetCellAt(cell)) {
			if(pCell->OccupyHeightsCoveringMe > 0) {
				--pCell->OccupyHeightsCoveringMe;
			}
		}
	}

	return 0x568ADC;
}


DEFINE_HOOK(4A8C77, MapClass_ProcessFoundation1_UnlimitBuffer, 5)
{
	GET_STACK(CellStruct const*, Foundation, 0x18);
	GET(DisplayClass *, Display, EBX);

	DWORD Len = BuildingExt::FoundationLength(Foundation);

	BuildingExt::TempFoundationData1.assign(Foundation, Foundation + Len);

	Display->CurrentFoundation_Data = BuildingExt::TempFoundationData1.data();

	auto const bounds = Display->FoundationBoundsSize(
		BuildingExt::TempFoundationData1.data());

	R->Stack<CellStruct>(0x18, bounds);
	R->EAX<CellStruct *>(R->lea_Stack<CellStruct *>(0x18));

	return 0x4A8C9E;
}

DEFINE_HOOK(4A8DD7, MapClass_ProcessFoundation2_UnlimitBuffer, 5)
{
	GET_STACK(CellStruct const*, Foundation, 0x18);
	GET(DisplayClass *, Display, EBX);

	DWORD Len = BuildingExt::FoundationLength(Foundation);

	BuildingExt::TempFoundationData2.assign(Foundation, Foundation + Len);

	Display->CurrentFoundationCopy_Data = BuildingExt::TempFoundationData2.data();

	auto const bounds = Display->FoundationBoundsSize(
		BuildingExt::TempFoundationData2.data());

	R->Stack<CellStruct>(0x18, bounds);
	R->EAX<CellStruct *>(R->lea_Stack<CellStruct *>(0x18));

	return 0x4A8DFE;
}
