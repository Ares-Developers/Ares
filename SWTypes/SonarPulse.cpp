#include "SonarPulse.h"
#include "../TechnoExt.h"

void SW_SonarPulse::LoadFromINI(
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->get_ID();

	if(!CONTAINS(SuperWeaponTypeClassExt::Ext_p, pSW) || !pINI->GetSection(section))
	{
		return;
	}

	if(!pData->SW_Initialized)
	{
		pData->Initialize();
	}

	char buffer[256];

	PARSE_ANIM("SonarPulse.Animation", pData->Sonar_Anim);
	pData->Sonar_Range = pINI->ReadInteger(section, "SonarPulse.Range", pData->Sonar_Range);
	pData->Sonar_Delay = pINI->ReadInteger(section, "SonarPulse.Delay", pData->Sonar_Delay);
	PARSE_SND("SonarPulse.Sound", pData->Sonar_Sound);
}

bool SW_SonarPulse::CanFireAt(CellStruct *pCoords)
{
	CellClass *pCell = MapClass::Global()->GetCellAt(pCoords);
	return (pCell && pCell->get_LandType() == lt_Water);
}

bool SW_SonarPulse::Launch(SuperClass* pThis, CellStruct* pCoords)
{
	SuperWeaponTypeClass *pType = pThis->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pType));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pType];

	CoordStruct coords;
	MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);

	if(pData->Sonar_Anim)
	{
		new AnimClass(pData->Sonar_Anim, coords);
	}

	if(pData->Sonar_Sound != -1)
	{
		VocClass::PlayAt(pData->Sonar_Sound, &coords);
	}

	int countCells = CellSpread::NumCells(pData->Sonar_Range);
	for(int i = 0; i < countCells; ++i)
	{
		CellStruct tmpCell = CellSpread::GetCell(i);
		tmpCell += *pCoords;
		CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);
		for(ObjectClass *curObj = c->get_FirstObject(); curObj; curObj = curObj->get_NextObject())
		{
			if(!(curObj->get_AbstractFlags() & ABSFLAGS_ISTECHNO))
			{
				continue;
			}
			TechnoClass *curT = (TechnoClass *)curObj;
			if(curT->get_CloakState())
			{
				curT->Uncloak(1);
				curT->set_Sensed(1);
				curT->set_Cloakable(0);
				TechnoClassExt::Ext_p[curT]->CloakSkipTimer.Start(pData->Sonar_Delay);
			}
		}
	}

	Unsorted::CurrentSWType = -1;
	return 1;
}