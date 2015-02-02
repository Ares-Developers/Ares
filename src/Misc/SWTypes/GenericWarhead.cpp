#include "GenericWarhead.h"
#include "../../Ext/Building/Body.h"
#include "../../Ext/Techno/Body.h"
#include "../../Ext/WarheadType/Body.h"

void SW_GenericWarhead::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	pData->SW_AITargetingType = SuperWeaponAITargetingMode::Offensive;
}

bool SW_GenericWarhead::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	auto const pType = pThis->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pType);

	auto pWarhead = GetWarhead(pData);
	auto damage = GetDamage(pData);

	if(!pData || !pWarhead) {
		Debug::Log("Couldn't launch GenericWarhead SW ([%s])\n", pType->ID);
		return false;
	}

	auto const pCell = MapClass::Instance->GetCellAt(Coords);
	auto const coords = pCell->GetCoordsWithBridge();

	BuildingClass* pFirer = nullptr;
	for(auto const& pBld : pThis->Owner->Buildings) {
		if(this->IsLaunchSiteEligible(pData, Coords, pBld, false)) {
			pFirer = pBld;
			break;
		}
	}

	// crush, kill, destroy
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	pWHExt->applyRipples(coords);
	pWHExt->applyIronCurtain(coords, pThis->Owner, damage);
	pWHExt->applyEMP(coords, pFirer);
	pWHExt->applyAttachedEffect(coords, pFirer);

	if(!pWHExt->applyPermaMC(pThis->Owner, pCell->GetContent())) {
		MapClass::DamageArea(coords, damage, pFirer, pWarhead, true, pThis->Owner);
		if(auto const pAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, pCell->LandType, coords)) {
			GameCreate<AnimClass>(pAnimType, coords);
		}
		MapClass::FlashbangWarheadAt(damage, pWarhead, coords, false, SpotlightFlags::None);
	}

	return true;
}
