#include "SpyPlane.h"
#include "../../Ares.h"
#include "../../Utilities/TemplateDef.h"

#include <HouseClass.h>

bool SW_SpyPlane::HandlesType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::SpyPlane);
}

void SW_SpyPlane::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Defaults to Spy Plane values
	pData->SpyPlane_TypeIndex = AircraftTypeClass::FindIndex("SPYP");
	pData->SpyPlane_Count = 1;
	pData->SpyPlane_Mission = Mission::SpyplaneApproach;

	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndex("EVA_SpyPlaneReady");
	
	pData->SW_AITargetingType = SuperWeaponAITargetingMode::ParaDrop;
	pData->SW_Cursor = MouseCursor::GetCursor(MouseCursorType::SpyPlane);
}

void SW_SpyPlane::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->SpyPlane_Count.Read(exINI, section, "SpyPlane.Count");
	pData->SpyPlane_TypeIndex.Read(exINI, section, "SpyPlane.Type");
	pData->SpyPlane_Mission.Read(exINI, section, "SpyPlane.Mission");
}

bool SW_SpyPlane::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);
	
	if(pThis->IsCharged) {
		// launch all at once
		CellClass *pTarget = MapClass::Instance->GetCellAt(Coords);
		pThis->Owner->SendSpyPlanes(pData->SpyPlane_TypeIndex, pData->SpyPlane_Count,
			pData->SpyPlane_Mission, pTarget, nullptr);
	}

	return true;
}
