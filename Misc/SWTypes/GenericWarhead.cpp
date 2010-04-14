#include "GenericWarhead.h"
#include "../../Ext/Techno/Body.h"

void SW_GenericWarhead::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->GWarhead_WH.Parse(&exINI, section, "GenericWarhead.Warhead");
	pData->GWarhead_Damage.Parse(&exINI, section, "GenericWarhead.Damage");
}

/*bool SW_GenericWarhead::CanFireAt(CellStruct *pCoords) // D says I don't have to have this if there are no limits
{
	CellClass *pCell = MapClass::Global()->GetCellAt(pCoords);
	return (pCell && pCell->LandType == lt_Water);
}*/

bool SW_GenericWarhead::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	SuperWeaponTypeClass *pType = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

	if(!pData || !pData->GWarhead_WH) {
		Debug::Log("Couldn't launch GenericWarhead SW - a variable is unavailable! \
		SW ExtData: %p - Warhead: %p - Damage: %i", pData, pData->GWarhead_WH, pData->GWarhead_Damage);
		return 0;
	}

	CoordStruct coords;
	MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);

	// crush, kill, destroy
	// NULL -> TechnoClass* SourceObject
	WarheadTypeExt::ExtData::applyRipples(pData->GWarhead_WH, coords);
	WarheadTypeExt::ExtData::applyIronCurtain(pData->GWarhead_WH, coords, pThis->Owner);
	WarheadTypeExt::ExtData::applyEMP(pData->GWarhead_WH, coords);
	if(!WarheadTypeExt::ExtData::applyPermaMC(pData->GWarhead_WH, coords, pThis->Owner, MapClass::Global()->GetCellAt(pCoords)->GetContent())) {
		MapClass::DamageArea(coords, pData->GWarhead_Damage, NULL, pData->GWarhead_WH, 1, pThis->Owner);
	}


	Unsorted::CurrentSWType = -1;
	return 1;
}
