#include "Firewall.h"
#include "../../Ext/Techno/Body.h"

#include "../Network.h"

int SW_Firewall::FirewallTypeIndex = -1;

bool SW_Firewall::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	AresNetEvent::Handlers::RaiseFirewallToggle(pThis->Owner);

	return true;
}
