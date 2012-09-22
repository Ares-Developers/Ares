#ifndef ARES_NETWORKING_H_
#define ARES_NETWORKING_H_

#include <Networking.h>

class AresNetEvent {
public:
	enum {
		aev_First = 0x60,
		aev_TrenchRedirectClick = 0x60,
		aev_FirewallToggle = 0x61,
		aev_Last = 0x61
	};

	class Handlers {
	public:
		static void RaiseTrenchRedirectClick(BuildingClass *Source, CellStruct *Target);
		static void RespondToTrenchRedirectClick(NetworkEvent *Event);

		static void RaiseFirewallToggle(HouseClass *Source);
		static void RespondToFirewallToggle(NetworkEvent *Event);
	};
};

#endif
