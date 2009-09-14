#include "SonarPulse.h"
#include "..\..\Ext\Techno\Body.h"

void SW_SonarPulse::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	pData->Sonar_Range = pINI->ReadInteger(section, "SonarPulse.Range", pData->Sonar_Range);
	pData->Sonar_Delay = pINI->ReadInteger(section, "SonarPulse.Delay", pData->Sonar_Delay);
}

bool SW_SonarPulse::CanFireAt(CellStruct *pCoords)
{
	CellClass *pCell = MapClass::Global()->GetCellAt(pCoords);
	return (pCell && pCell->LandType == lt_Water);
}

bool SW_SonarPulse::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	SuperWeaponTypeClass *pType = pThis->get_Type();
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

	if(!pData) {
		return 0;
	}

	CoordStruct coords;
	MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);

	int countCells = CellSpread::NumCells(pData->Sonar_Range);
	for(int i = 0; i < countCells; ++i) {
		CellStruct tmpCell = CellSpread::GetCell(i);
		tmpCell += *pCoords;
		CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);
		if(c->LandType != lt_Water) {
			continue;
		}
		for(ObjectClass *curObj = c->FirstObject; curObj; curObj = curObj->NextObject) {
			if(!(curObj->AbstractFlags & ABSFLAGS_ISTECHNO)) {
				continue;
			}
			TechnoClass *curT = reinterpret_cast<TechnoClass *>(curObj);
			if(curT->CloakState) {
				curT->Uncloak(1);
				curT->IsSensed = 1;
				curT->Cloakable = 0;
				TechnoExt::ExtData *pTechno = TechnoExt::ExtMap.Find(curT);
				if(pTechno) {
					pTechno->CloakSkipTimer.Start(pData->Sonar_Delay);
				}
			}
		}
	}

	Unsorted::CurrentSWType = -1;
	return 1;
}
