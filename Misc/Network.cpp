#include "Network.h"

#include "Debug.h"
#include "../Ares.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <NetworkEvents.h>

DEFINE_HOOK(4C6CCD, Networking_RespondToEvent, 0)
{
	GET(DWORD, EventKind, EAX);
	GET(NetworkEvent *, Event, ESI);

	if(EventKind >= AresNetEvent::aev_First) {
		// Received Ares event, do something about it
		switch(EventKind) {
			case AresNetEvent::aev_TrenchRedirectClick:
				Debug::Log("Setting trench target...\n");
				AresNetEvent::Handlers::RespondToTrenchRedirectClick(Event);
				break;
		}
	}

	--EventKind;
	R->EAX(EventKind);
	return (EventKind > 0x2D)
	 ? 0x4C8109
	 : 0x4C6CD7
	;
}

/*
 how to raise your own events
	NetworkEvent * Event = new NetworkEvent();
	Event->Kind = AresNetworkEvent::aev_blah;
	Event->HouseIndex = U->Owner->ArrayIndex;
	memcpy(Event->ExtraData, "Boom de yada", 0xkcd);
	Networking::AddEvent(Event);
	delete Event;
*/

void AresNetEvent::Handlers::RaiseTrenchRedirectClick(BuildingClass *Source, CellStruct *Target) {
	NetworkEvent * Event = new NetworkEvent();
	Event->Kind = AresNetEvent::aev_TrenchRedirectClick;
	Event->HouseIndex = byte(Source->Owner->ArrayIndex);
	byte *ExtraData = Event->ExtraData;

	NetID SourceObject, TargetCoords;

	TargetCoords.Pack(Target);
	memcpy(ExtraData, &TargetCoords, sizeof(TargetCoords));
	ExtraData += sizeof(TargetCoords);

	SourceObject.Pack(Source);
	memcpy(ExtraData, &SourceObject, sizeof(SourceObject));
	ExtraData += sizeof(SourceObject);

	Networking::AddEvent(Event);
	delete Event;
}

void AresNetEvent::Handlers::RespondToTrenchRedirectClick(NetworkEvent *Event) {
	NetID *ID = reinterpret_cast<NetID *>(Event->ExtraData);
	if(BuildingClass * pSourceBuilding = ID->UnpackBuilding()) {
		++ID;
		if(CellClass * pTargetCell = ID->UnpackCell()) {
			Debug::Log("Setting target coords of [%s] to cell at %d, %d\n", pSourceBuilding->Type->ID, pTargetCell->MapCoords);
		}
	}

}
