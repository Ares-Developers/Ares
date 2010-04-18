#include "GenericWarhead.h"
#include "../../Ext/Techno/Body.h"

void SW_GenericWarhead::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->GWarhead_WH.Parse(&exINI, section, "GenericWarhead.Warhead");
	pData->GWarhead_Damage.Read(&exINI, section, "GenericWarhead.Damage");
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
		Debug::Log("Couldn't launch GenericWarhead SW ([%s])\n", pType->ID);
		return 0;
	}

	CoordStruct coords;
	CellClass *Cell = MapClass::Instance->GetCellAt(pCoords);
	Cell->GetCoordsWithBridge(&coords);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pData->GWarhead_WH);

	// crush, kill, destroy
	// NULL -> TechnoClass* SourceObject
	pWHExt->applyRipples(&coords);
	pWHExt->applyIronCurtain(&coords, pThis->Owner);
	pWHExt->applyEMP(&coords);

	BuildingClass *Firer = NULL;
	HouseClass *FirerHouse = pThis->Owner;
	for(int i = 0; i < FirerHouse->Buildings.Count; ++i) {
		BuildingClass *B = FirerHouse->Buildings[i];
		if(B->HasSuperWeapon(pThis->Type->ArrayIndex)) {
			Firer = B;
			break;
		}
	}

	if(!pWHExt->applyPermaMC(&coords, pThis->Owner, Cell->GetContent())) {
		MapClass::DamageArea(&coords, pData->GWarhead_Damage, Firer, pData->GWarhead_WH, 1, pThis->Owner);
		if(AnimTypeClass * DamageAnimType = MapClass::SelectDamageAnimation(pData->GWarhead_Damage, pData->GWarhead_WH, Cell->LandType, &coords)) {
			AnimClass *DamageAnim;
			GAME_ALLOC(AnimClass, DamageAnim, DamageAnimType, &coords);
		}
		MapClass::FlashbangWarheadAt(pData->GWarhead_Damage, pData->GWarhead_WH, coords, false, 0);
	}

	Unsorted::CurrentSWType = -1;
	return 1;
}
