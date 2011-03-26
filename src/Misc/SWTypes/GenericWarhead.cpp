#include "GenericWarhead.h"
#include "../../Ext/Techno/Body.h"

void SW_GenericWarhead::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	pData->SW_AITargetingType = SuperWeaponAITargetingMode::Offensive;
}

bool SW_GenericWarhead::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	SuperWeaponTypeClass *pType = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

	if(!pData || !pData->SW_Warhead) {
		Debug::Log("Couldn't launch GenericWarhead SW ([%s])\n", pType->ID);
		return 0;
	}

	CoordStruct coords;
	CellClass *Cell = MapClass::Instance->GetCellAt(pCoords);
	Cell->GetCoordsWithBridge(&coords);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pData->SW_Warhead);

	// crush, kill, destroy
	// NULL -> TechnoClass* SourceObject
	pWHExt->applyRipples(&coords);
	pWHExt->applyIronCurtain(&coords, pThis->Owner, pData->SW_Damage);

	BuildingClass *Firer = NULL;
	HouseClass *FirerHouse = pThis->Owner;
	for(int i = 0; i < FirerHouse->Buildings.Count; ++i) {
		BuildingClass *B = FirerHouse->Buildings[i];
		if(B->HasSuperWeapon(pThis->Type->ArrayIndex)) {
			Firer = B;
			break;
		}
	}

	pWHExt->applyEMP(&coords, Firer);

	if(!pWHExt->applyPermaMC(&coords, pThis->Owner, Cell->GetContent())) {
		MapClass::DamageArea(&coords, pData->SW_Damage, Firer, pData->SW_Warhead, 1, pThis->Owner);
		if(AnimTypeClass * DamageAnimType = MapClass::SelectDamageAnimation(pData->SW_Damage, pData->SW_Warhead, Cell->LandType, &coords)) {
			AnimClass *DamageAnim;
			GAME_ALLOC(AnimClass, DamageAnim, DamageAnimType, &coords);
		}
		MapClass::FlashbangWarheadAt(pData->SW_Damage, pData->SW_Warhead, coords, false, 0);
	}

	return 1;
}
