#include "Body.h"

void BuildingExt::cPrismForwarding::SetSupportTarget(BuildingClass *pTargetTower) {
	auto pSlaveTower = this->Owner->AttachedToObject;

	// meet the new tower, same as the old tower
	if(this->SupportTarget == pTargetTower) {
		return;
	}

	// if the target tower is already set, disconnect it by removing it from the old target tower's sender list
	if(auto pOldTarget = this->SupportTarget) {
		if(auto pOldTargetData = BuildingExt::ExtMap.Find(pOldTarget)) {
			int idxSlave = pOldTargetData->PrismForwarding.Senders.FindItemIndex(pSlaveTower);
			if(idxSlave != -1) {
				pOldTargetData->PrismForwarding.Senders.RemoveItem(idxSlave);
				// everywhere the comments say this is now the "longest backwards chain", but decreasing this here makes use of the original meaning. why is this needed here? AlexB 2012-04-08
				--pOldTarget->SupportingPrisms;  //Ares doesn't actually use this, but maintaining it anyway (as direct feeds only)
			} else {
				Debug::DevLog(Debug::Warning, "PrismForwarding::SetSupportTarget: Old target tower (%p) did not consider this tower (%p) as its sender.\n", pOldTarget, pSlaveTower);
			}
		}
		this->SupportTarget = nullptr;
	}

	// set the new tower as support target
	if(pTargetTower) {
		if(auto pTargetData = BuildingExt::ExtMap.Find(pTargetTower)) {
			this->SupportTarget = pTargetTower;

			if(pTargetData->PrismForwarding.Senders.FindItemIndex(pSlaveTower) == -1) {
				pTargetData->PrismForwarding.Senders.AddItem(pSlaveTower);
				// why isn't SupportingPrisms increased here? AlexB 2012-04-08
			} else {
				Debug::DevLog(Debug::Warning, "PrismForwarding::SetSupportTarget: Tower (%p) is already in new target tower's (%p) sender list.\n", pSlaveTower, pTargetTower);
			}
		}
	}
}

void BuildingExt::cPrismForwarding::RemoveAllSenders() {
	// disconnect all sender towers from their support target, which is me
	for(int senderIdx = this->Senders.Count; senderIdx; senderIdx--) {
		if(BuildingClass *NextTower = this->Senders[senderIdx - 1]) {
			auto pData = BuildingExt::ExtMap.Find(NextTower);
			pData->PrismForwarding.SetSupportTarget(nullptr);
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
