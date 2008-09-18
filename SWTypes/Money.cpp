#include "Money.h"
#include "../TechnoExt.h"

void SW_Money::LoadFromINI(
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

	PARSE_BUF();

	pData->Money_Amount = pINI->ReadInteger(section, "Money.Amount", pData->Money_Amount);
	PARSE_SND("Money.Sound", pData->Money_Chaching);
}

bool SW_Money::Launch(SuperClass* pThis, CellStruct* pCoords)
{
	SuperWeaponTypeClass *pType = pThis->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pType));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pType];

	if(pData->Money_Chaching != -1)
	{
		VocClass::PlayGlobal(pData->Money_Chaching, 1.0f, 0x2000);
	}

	Ares::Log("House %d gets %d credits\n", pThis->get_Owner()->get_ArrayIndex(), pData->Money_Amount);
	pThis->get_Owner()->GiveMoney(pData->Money_Amount);

	Unsorted::CurrentSWType = -1;
	return 1;
}