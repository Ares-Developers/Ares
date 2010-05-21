#include "Body.h"
#include "../../Misc/SWTypes.h"

DEFINE_HOOK(6CEF84, SuperWeaponTypeClass_GetCursorOverObject, 7)
{
	GET(SuperWeaponTypeClass*, pThis, ECX);

	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);

	if(pThis->Action == SW_YES_CURSOR) {
		GET_STACK(CellStruct *, pMapCoords, 0x0C);

		int Action = SW_YES_CURSOR;

		if(!pData->SW_FireToShroud.Get()) {
			CellClass* pCell = MapClass::Instance->GetCellAt(pMapCoords);
			CoordStruct Crd;

			if(MapClass::Instance->IsLocationShrouded(pCell->GetCoords(&Crd))) {
				Action = SW_NO_CURSOR;
			}
		}

		if(pThis->Type >= FIRST_SW_TYPE && !NewSWType::GetNthItem(pThis->Type)->CanFireAt(pMapCoords)) {
			Action = SW_NO_CURSOR;
		}

		R->EAX(Action);

		if(Action == SW_YES_CURSOR) {
			SWTypeExt::CurrentSWType = pThis;
			Actions::Set(&pData->SW_Cursor, pData->SW_FireToShroud);
		} else {
			SWTypeExt::CurrentSWType = NULL;
			Actions::Set(&pData->SW_NoCursor, pData->SW_FireToShroud);
		}
		return 0x6CEFD9;
	}
	return 0;
}


DEFINE_HOOK(653B3A, RadarClass_GetMouseAction_CustomSWAction, 5)
{
	int idxSWType = Unsorted::CurrentSWType;
	if(idxSWType > -1) {
		GET_STACK(byte, EventFlags, 0x58);

		MouseEvent::Value E(EventFlags);
		if(E & (MouseEvent::RightDown | MouseEvent::RightUp)) {
			return 0x653D6F;
		}

		SuperWeaponTypeClass *pThis = SuperWeaponTypeClass::Array->GetItem(idxSWType);
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);

		if(pThis->Action == SW_YES_CURSOR) {
			GET_STACK(CellStruct, pMapCoords, STACK_OFFS(0x54, 0x3C));

			int Action = SW_YES_CURSOR;

			if(!pData->SW_FireToShroud.Get()) {
				CellClass* pCell = MapClass::Instance->GetCellAt(&pMapCoords);
				CoordStruct Crd;

				if(MapClass::Instance->IsLocationShrouded(pCell->GetCoords(&Crd))) {
					Action = SW_NO_CURSOR;
				}
			}

			if(pThis->Type >= FIRST_SW_TYPE && !NewSWType::GetNthItem(pThis->Type)->CanFireAt(&pMapCoords)) {
				Action = SW_NO_CURSOR;
			}

			R->ESI(Action);

			if(Action == SW_YES_CURSOR) {
				SWTypeExt::CurrentSWType = pThis;
				Actions::Set(&pData->SW_Cursor, pData->SW_FireToShroud);
			} else {
				SWTypeExt::CurrentSWType = NULL;
				Actions::Set(&pData->SW_NoCursor, pData->SW_FireToShroud);
			}
			return 0x653CA3;
		}
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
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(Super->Type);

	R->EAX<int>(pData->SpyPlane_TypeIndex);
	return 0x6CD684;
}

// decouple SpyPlane from allied paradrop counts
DEFINE_HOOK(6CD6A6, SuperClass_Launch_SpyPlane_Fire, 6)
{
	GET(SuperClass *, Super, EBX);
	GET(CellClass *,TargetCell, EDI);
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(Super->Type);

	Super->Owner->SendSpyPlanes(
		pData->SpyPlane_TypeIndex, pData->SpyPlane_Count, pData->SpyPlane_Mission, TargetCell, NULL);

	return 0x6CD6E9;
}

// decouple nuke siren from DigSound
DEFINE_HOOK(6CDDE3, SuperClass_Launch_Nuke_Siren, 6)
{
	GET(SuperClass *, Super, EBX);
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(Super->Type);

	R->ECX(pData->Nuke_Siren);
	return 0x6CDDE9;
}

// 6CEE96, 5
DEFINE_HOOK(6CEE96, SuperWeaponTypeClass_GetTypeIndex, 5)
{
	GET(const char *, TypeStr, EDI);
	int customType = NewSWType::FindIndex(TypeStr);
	if(customType > -1) {
		R->ESI(customType);
		return 0x6CEE9C;
	}
	return 0;
}

// 4AC20C, 7
// translates SW click to type
DEFINE_HOOK(4AC20C, DisplayClass_LMBUp, 7)
{
	int Action = R->Stack32(0x9C);
	if(Action < SW_NO_CURSOR) {
		SuperWeaponTypeClass * pSW = SuperWeaponTypeClass::FindFirstOfAction(Action);
		R->EAX(pSW);
		return pSW ? 0x4AC21C : 0x4AC294;
	}
	else if(Action == SW_NO_CURSOR) {
		R->EAX(0);
		return 0x4AC294;
	}

	R->EAX(SWTypeExt::CurrentSWType);

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

	R->EAX(pHouse->Supers.GetItem(swTIdx));
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

	R->EDI(pBuild->Type);
	R->EAX(pHouse->Supers.GetItem(swTIdx));
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

DEFINE_HOOK(50B319, HouseClass_UpdateSWs, 6)
{
//	GET(SuperClass *, Super, ECX);
	GET(HouseClass *, H, EBP);
	GET(int, Index, EDI);
	SuperClass *Super = H->Supers.GetItem(Index);
	SuperWeaponTypeClass *pSW = Super->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);
	if(pData->SW_AutoFire) {
	//	Fire! requires networking event woodoo or fake mouse clicks, bah
	}
	return 0;
}

// EVA_Activated is complex, will do later

// AI SW targeting submarines
DEFINE_HOOK(50CFAA, HouseClass_PickOffensiveSWTarget, 0)
{
	R->ESI(0);
	R->Stack8(0x13, 1);
	return 0x50CFC9;
}

// ARGH!
DEFINE_HOOK(6CC390, SuperClass_Launch, 6)
{
	GET(SuperClass *, pSuper, ECX);
	GET_STACK(CellStruct*, pCoords, 0x4);
	GET_STACK(byte, IsPlayer, 0x8);
	bool handled = SWTypeExt::SuperClass_Launch(pSuper, pCoords, IsPlayer);

	return handled ? 0x6CDE40 : 0;
}

DEFINE_HOOK(6CC360, SuperClass_IsReadyToFire, 5)
{
	GET(SuperClass *, pThis, ECX);
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	if(pData->Money_Amount) {
		if(pThis->Owner->Available_Money() < pData->Money_Amount) {
			return 0x6CC381;
		}
	}
	return 0;
}

DEFINE_HOOK(44691B, BuildingClass_4DC_SWAvailable, 6)
{
	GET(BuildingClass *, Structure, EBP);
	GET(BuildingTypeClass *, AuxBuilding, EAX);
	return Structure->Owner->CountOwnedNow(AuxBuilding) > 0
		? 0x446937
		: 0x44699A
	;
}

DEFINE_HOOK(45765A, BuildingClass_SWAvailable, 6)
{
	GET(BuildingClass *, Structure, ESI);
	GET(BuildingTypeClass *, AuxBuilding, EAX);
	return Structure->Owner->CountOwnedNow(AuxBuilding) > 0
		? 0x457676
		: 0x45767B
	;
}

DEFINE_HOOK(4576BA, BuildingClass_SW2Available, 6)
{
	GET(BuildingClass *, Structure, ESI);
	GET(BuildingTypeClass *, AuxBuilding, EAX);
	return Structure->Owner->CountOwnedNow(AuxBuilding) > 0
		? 0x4576D6
		: 0x4576DB
	;
}
