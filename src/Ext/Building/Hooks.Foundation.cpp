#include "Body.h"
#include "../BuildingType/Body.h"

CellStruct * BuildingExt::TempFoundationData1 = NULL;
CellStruct * BuildingExt::TempFoundationData2 = NULL;

byte* BuildingExt::TempFoundationOccupyData1 = NULL;
byte* BuildingExt::TempFoundationOccupyData2 = NULL;

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

DEFINE_HOOK(568411, MapClass_AddContentAt_Foundation_P1, 0)
{
	GET(BuildingClass *, pThis, EDI);

	R->EBP<CellStruct *>(pThis->GetFoundationData(false));

	return 0x568432;
}

DEFINE_HOOK(568565, MapClass_AddContentAt_Foundation_P2, 0)
{
	GET(BuildingClass *, pThis, EDI);

	CellStruct * Foundation = pThis->GetFoundationData(false);
	DWORD Len = BuildingExt::FoundationLength(Foundation);
	Len *= 64;
	BuildingExt::TempFoundationOccupyData1 = new byte[Len];
	memset(BuildingExt::TempFoundationOccupyData1, 0, Len * sizeof(byte));

	R->EBX<CellStruct *>(Foundation);

	return 0x568579;
}

DEFINE_HOOK(568605, MapClass_AddContentAt_Foundation_P3, 0)
{
	GET(int, Offset, EAX);
	byte * Ptr = reinterpret_cast<byte *>(BuildingExt::TempFoundationOccupyData1);

	Ptr += Offset;
	R->CL(*Ptr);
	R->EAX<byte *>(Ptr);

	return 0x568613;
}

DEFINE_HOOK(5687E3, MapClass_AddContentAt_Foundation_P4, 6)
{
	delete[] BuildingExt::TempFoundationOccupyData1;
	BuildingExt::TempFoundationOccupyData1 = NULL;

	return 0;
}

DEFINE_HOOK(568841, MapClass_RemoveContentAt_Foundation_P1, 0)
{
	GET(BuildingClass *, pThis, EDI);

	R->EBP<CellStruct *>(pThis->GetFoundationData(false));

	return 0x568862;
}

DEFINE_HOOK(568997, MapClass_RemoveContentAt_Foundation_P2, 0)
{
	GET(BuildingClass *, pThis, EDX);

	CellStruct * Foundation = pThis->GetFoundationData(false);
	DWORD Len = BuildingExt::FoundationLength(Foundation);
	Len *= 64;
	BuildingExt::TempFoundationOccupyData2 = new byte[Len];
	memset(BuildingExt::TempFoundationOccupyData2, 0, Len * sizeof(byte));

	R->EBX<CellStruct *>(Foundation);

	return 0x5689AB;
}

DEFINE_HOOK(568A37, MapClass_RemoveContentAt_Foundation_P3, 0)
{
	GET(int, Offset, EAX);
	byte * Ptr = BuildingExt::TempFoundationOccupyData2 + Offset;
	R->CL(*Ptr);
	R->EAX<byte *>(Ptr);

	return 0x568A45;
}

DEFINE_HOOK(568B98, MapClass_RemoveContentAt_Foundation_P4, 6)
{
	delete[] BuildingExt::TempFoundationOccupyData2;
	BuildingExt::TempFoundationOccupyData2 = NULL;
	return 0;
}

DEFINE_HOOK(4A8C77, MapClass_ProcessFoundation1_UnlimitBuffer, 5)
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

DEFINE_HOOK(4A8DD7, MapClass_ProcessFoundation2_UnlimitBuffer, 5)
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
