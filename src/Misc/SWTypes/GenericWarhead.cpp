#include "GenericWarhead.h"
#include "../../Ext/Techno/Body.h"
#include "../../Ext/WarheadType/Body.h"

void SW_GenericWarhead::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	pData->SW_AITargetingType = SuperWeaponAITargetingMode::Offensive;
}

bool SW_GenericWarhead::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	SuperWeaponTypeClass *pType = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

	auto pWarhead = GetWarhead(pData);
	auto damage = GetDamage(pData);

	if(!pData || !pWarhead) {
		Debug::Log("Couldn't launch GenericWarhead SW ([%s])\n", pType->ID);
		return 0;
	}

	CellClass *Cell = MapClass::Instance->GetCellAt(Coords);
	CoordStruct coords = Cell->GetCoordsWithBridge();

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	// crush, kill, destroy
	// NULL -> TechnoClass* SourceObject
	pWHExt->applyRipples(coords);
	pWHExt->applyIronCurtain(coords, pThis->Owner, damage);

	BuildingClass *Firer = nullptr;
	HouseClass *FirerHouse = pThis->Owner;
	for(int i = 0; i < FirerHouse->Buildings.Count; ++i) {
		BuildingClass *B = FirerHouse->Buildings[i];
		if(B->HasSuperWeapon(pThis->Type->ArrayIndex)) {
			Firer = B;
			break;
		}
	}

	pWHExt->applyEMP(coords, Firer);
	pWHExt->applyAttachedEffect(&coords, Firer);

	if(!pWHExt->applyPermaMC(coords, pThis->Owner, Cell->GetContent())) {
		MapClass::DamageArea(coords, damage, Firer, pWarhead, true, pThis->Owner);
		if(AnimTypeClass * DamageAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, Cell->LandType, coords)) {
			GameCreate<AnimClass>(DamageAnimType, coords);
		}
		MapClass::FlashbangWarheadAt(damage, pWarhead, coords, false, 0);
	}

	return 1;
}
