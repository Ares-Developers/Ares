#include "Firewall.h"
#include "../../Ext/House/Body.h"
#include "../../Ext/Techno/Body.h"

#include "../Network.h"

SuperWeaponType SW_Firewall::FirewallType = SuperWeaponType::Invalid;

bool SW_Firewall::Activate(
	SuperClass* const pThis, const CellStruct& cell, bool const isPlayer)
{
	auto const pOwner = pThis->Owner;
	auto const pExt = HouseExt::ExtMap.Find(pOwner);
	pExt->SetFirestormState(true);

	if(isPlayer) {
		pOwner->RecheckTechTree = true;
	}

	return true;
}

void SW_Firewall::Deactivate(
	SuperClass* const pThis, CellStruct const cell, bool const isPlayer)
{
	auto const pOwner = pThis->Owner;
	auto const pExt = HouseExt::ExtMap.Find(pOwner);
	pExt->SetFirestormState(false);

	if(isPlayer) {
		pOwner->RecheckTechTree = true;
	}
}
