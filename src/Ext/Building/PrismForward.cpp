#include "Body.h"

void BuildingExt::cPrismForwarding::RemoveAllSenders() {
	// disconnect all sender towers from their support target, which is me
	for(int senderIdx = this->Senders.Count; senderIdx; senderIdx--) {
		if(BuildingClass *NextTower = this->Senders[senderIdx - 1]) {
			BuildingTypeExt::cPrismForwarding::SetSupportTarget(NextTower, nullptr);
		}
	}

	// log if not all senders could be removed
	if(this->Senders.Count) {
		Debug::DevLog(Debug::Warning,
			"PrismForwarding::RemoveAllSenders: Tower (%p) still has %d senders after removal completed.\n",
			this->Owner->AttachedToObject, this->Senders.Count);
		for(int i = 0; i<this->Senders.Count; ++i) {
			Debug::DevLog(Debug::Warning, "Sender %03d: %p\n", i, this->Senders[i]);
		}

		this->Senders.Clear();
	}
}
