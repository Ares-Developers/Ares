#include "Body.h"
#include "..\..\Misc\SWTypes.h"

DEFINE_HOOK(6CEF84, SuperWeaponTypeClass_GetCursorOverObject, 7)
{
	//FUCK THIS, no macros in my code xD -pd
	GET(SuperWeaponTypeClass*, pThis, ECX);

	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);

	if(pThis->Action >= 0x7E) {
		SWTypeExt::CurrentSWType = pThis;

		CellStruct* pMapCoords = (CellStruct*)R->get_StackVar32(0x0C);

		int Action = SW_YES_CURSOR;
		
		if(!pData->SW_FireToShroud) {
			CellClass* pCell = MapClass::Global()->GetCellAt(pMapCoords);
			CoordStruct Crd;

			if(MapClass::Global()->IsLocationShrouded(pCell->GetCoords(&Crd))) {
				Action = SW_NO_CURSOR;
			}
		}

		if(pThis->Type >= FIRST_SW_TYPE && !NewSWType::GetNthItem(pThis->Type)->CanFireAt(pMapCoords)) {
			Action = SW_NO_CURSOR;
		}

		R->set_EAX(Action);

		Actions::Set(Action == SW_YES_CURSOR ? &pData->SW_Cursor : &pData->SW_NoCursor);
		return 0x6CEFD9;
	}
	return 0;
}

/*
// 6AAF92, 6
XPORT_FUNC(SidebarClass_ProcessCameoClick)
{
	DWORD idx = R->get_ESI();
	SuperWeaponTypeClass *pThis = SuperWeaponTypeClass::Array->GetItem(idx);
//	int TypeIdx = pThis->get_Type();

	SWTypeExt::CurrentSWType = pThis;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);

//	Actions::Set(&pData->SW_Cursor);

	return 0;
}
*/

// decouple SpyPlane from SPYP
DEFINE_HOOK(6CD67A, SuperClass_Launch_SpyPlane_FindType, 0)
{
	GET(SuperClass *, Super, EBX);
	SuperWeaponTypeClass *pThis = Super->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);

	R->set_EAX(pData->SpyPlane_TypeIndex);
	return 0x6CD684;
}

// decouple SpyPlane from allied paradrop counts
DEFINE_HOOK(6CD6A6, SuperClass_Launch_SpyPlane_Fire, 6)
{
	GET(SuperClass *, Super, EBX);
	GET(CellClass *,TargetCell, EDI);
	SuperWeaponTypeClass *pThis = Super->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);

	Super->Owner->SendSpyPlanes(
		pData->SpyPlane_TypeIndex, pData->SpyPlane_Count, pData->SpyPlane_Mission, TargetCell, NULL);

	return 0x6CD6E9;
}

// decouple nuke siren from DigSound
DEFINE_HOOK(6CDDE3, SuperClass_Launch_Nuke_Siren, 6)
{
	GET(SuperClass *, Super, EBX);
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(Super->Type);

	R->set_ECX(pData->Nuke_Siren);
	return 0x6CDDE9;
}

// 6CEE96, 5
DEFINE_HOOK(6CEE96, SuperWeaponTypeClass_GetTypeIndex, 5)
{
	GET(const char *, TypeStr, EDI);
	int customType = NewSWType::FindIndex(TypeStr);
	if(customType > -1) {
		R->set_ESI(customType);
		return 0x6CEE9C;
	}
	return 0;
}

// 4AC20C, 7
// translates SW click to type
DEFINE_HOOK(4AC20C, DisplayClass_LMBUp, 7)
{
	int Action = R->get_StackVar32(0x9C);
	if(Action < SW_NO_CURSOR) {
		int idx = (DWORD)SuperWeaponTypeClass::FindFirstOfAction(Action);
		R->set_EAX(idx);
		return idx ? 0x4AC21C : 0x4AC294;
	}
	else if(Action == SW_NO_CURSOR) {
		R->set_EAX(0);
		return 0x4AC294;
	}

	R->set_EAX((DWORD)SWTypeExt::CurrentSWType);

	return 0x4AC21C;
}

// decoupling sw anims from types
// 446418, 6
DEFINE_HOOK(446418, BuildingClass_Place1, 6)
{
	GET(BuildingClass *, pBuild, EBP);
	GET(HouseClass *, pHouse, EAX);
	int swTIdx = pBuild->Type->SuperWeapon;
	if(swTIdx == -1) {
		swTIdx = pBuild->Type->SuperWeapon2;
		if(swTIdx == -1) {
			return 0x446580;
		}
	}

	SuperClass *pSuper = pHouse->get_Supers()->GetItem(swTIdx);
	R->set_EAX((DWORD)pSuper);
	return 0x44643E;
}

// 44656D, 6
DEFINE_HOOK(44656D, BuildingClass_Place2, 6)
{
	return 0x446580;
}

// 45100A, 6
DEFINE_HOOK(45100A, BuildingClass_ProcessAnims1, 6)
{
	GET(BuildingClass *, pBuild, ESI);
	GET(HouseClass *, pHouse, EAX);
	int swTIdx = pBuild->Type->SuperWeapon;
	if(swTIdx == -1) {
		swTIdx = pBuild->Type->SuperWeapon2;
		if(swTIdx == -1) {
			return 0x451145;
		}
	}

	SuperClass *pSuper = pHouse->get_Supers()->GetItem(swTIdx);
	R->set_EDI((DWORD)pBuild->GetTechnoType());
	R->set_EAX((DWORD)pSuper);
	return 0x451030;
}

// 451132, 6
DEFINE_HOOK(451132, BuildingClass_ProcessAnims2, 6)
{
	return 0x451145;
}

// EVA_Detected
// 446937, 6
DEFINE_HOOK(446937, BuildingClass_AnnounceSW, 6)
{
	GET(BuildingClass *, pBuild, EBP);
	int swTIdx = pBuild->Type->SuperWeapon;
	if(swTIdx == -1) {
		swTIdx = pBuild->Type->SuperWeapon2;
		if(swTIdx == -1) {
			return 0x44699A;
		}
	}

	SuperWeaponTypeClass *pSW = SuperWeaponTypeClass::Array->GetItem(swTIdx);
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	if(pData->EVA_Detected != -1) {
		VoxClass::PlayIndex(pData->EVA_Detected);
		return 0x44699A;
	}
	return 0;
}

// EVA_Ready
// 6CBDD7, 6
DEFINE_HOOK(6CBDD7, SuperClass_AnnounceReady, 6)
{
	GET(SuperWeaponTypeClass *, pThis, EAX);
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);

	if(pData->EVA_Ready != -1) {
		VoxClass::PlayIndex(pData->EVA_Ready);
		return 0x6CBE68;
	}
	return 0;
}

// 6CC0EA, 9
DEFINE_HOOK(6CC0EA, SuperClass_AnnounceQuantity, 9)
{
	GET(SuperClass *, pThis, ESI);
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	if(pData->EVA_Ready != -1) {
		VoxClass::PlayIndex(pData->EVA_Ready);
		return 0x6CC17E;
	}
	return 0;
}

// 50B319, 6
DEFINE_HOOK(50B319, HouseClass_UpdateSWs, 6)
{
//	GET(SuperClass *, Super, ECX);
	GET(HouseClass *, H, EBP);
	GET(int, Index, EDI);
	SuperClass *Super = H->get_Supers()->GetItem(Index);
	SuperWeaponTypeClass *pSW = Super->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);
	RET_UNLESS(pData->SW_AutoFire);

//	Fire! requires networking event woodoo or fake mouse clicks, bah
	return 0;
}

// EVA_Activated is complex, will do later

// AI SW targeting submarines
DEFINE_HOOK(50CFAA, HouseClass_PickOffensiveSWTarget, 0)
{
	R->set_ESI(0);
	R->set_StackVar8(0x13, 1);
	return 0x50CFC9;
}

