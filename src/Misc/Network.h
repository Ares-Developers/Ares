#pragma once

#include <Networking.h>

class AresNetEvent {
public:
	enum class Events : unsigned char {
		TrenchRedirectClick = 0x60,
		FirewallToggle = 0x61,

		First = TrenchRedirectClick,
		Last = FirewallToggle
	};

	class Handlers {
	public:
		static void RaiseTrenchRedirectClick(BuildingClass *Source, CellStruct *Target);
		static void RespondToTrenchRedirectClick(NetworkEvent *Event);

		static void RaiseFirewallToggle(HouseClass *Source);
		static void RespondToFirewallToggle(NetworkEvent *Event);
	};
};
