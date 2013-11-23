#include "Firewall.h"
#include "../../Ext/Techno/Body.h"

#include "../Network.h"

int SW_Firewall::FirewallTypeIndex = -1;

bool SW_Firewall::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	AresNetEvent::Handlers::RaiseFirewallToggle(pThis->Owner);

	return 1;
}
