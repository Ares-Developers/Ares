#include "Firewall.h"
#include "../../Ext/Techno/Body.h"

#include "../Network.h"

SuperWeaponType SW_Firewall::FirewallType = SuperWeaponType::Invalid;

bool SW_Firewall::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	AresNetEvent::Handlers::RaiseFirewallToggle(pThis->Owner);

	return true;
}
