#include "Body.h"

DEFINE_HOOK(71E949, EventClass_HasOccured, 7)
{
	GET(TEventClass*, pEvent, EBP);

	// check for events handled in Ares.
	bool ret = false;
	if(TEventExt::HasOccured(pEvent, &ret)) {
		// returns true or false
		return ret ? 0x71F1B1 : 0x71F163;
	}
	
	// not handled in Ares.
	return 0;
}
