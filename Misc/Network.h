#ifndef NETWORKING_H_
#define NETWORKING_H_

#include <NetworkEvents.h>

class AresNetEvent {
public:
	enum {
		aev_First = 0x60,
		aev_TrenchRedirectClick = 0x60,
		aev_Last = 0x60
	};

	class Handlers {
	public:
		static void RaiseTrenchRedirectClick(BuildingClass *Source, CellStruct *Target);
		static void RespondToTrenchRedirectClick(NetworkEvent *Event);
	};
};

#endif
