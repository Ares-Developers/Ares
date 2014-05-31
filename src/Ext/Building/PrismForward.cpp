#include "Body.h"

void BuildingExt::cPrismForwarding::SetChargeDelay_Get(int chain, int endChain, int LongestChain, DWORD *LongestCDelay, DWORD *LongestFDelay) {
	auto TargetTower = this->Owner->AttachedToObject;

	if(chain == endChain) {
		if(chain != LongestChain) {
			auto pTypeData = BuildingTypeExt::ExtMap.Find(TargetTower->Type);
			//update the delays for this chain
			unsigned int thisDelay = pTypeData->PrismForwarding.ChargeDelay + LongestCDelay[chain + 1];
			if(thisDelay > LongestCDelay[chain]) {
				LongestCDelay[chain] = thisDelay;
			}
		}
		if(TargetTower->DelayBeforeFiring > LongestFDelay[chain]) {
			LongestFDelay[chain] = TargetTower->DelayBeforeFiring;
		}
	} else {
		//ascend to the next chain
		for(auto SenderTower : this->Senders) {
			auto pData = BuildingExt::ExtMap.Find(SenderTower);
			pData->PrismForwarding.SetChargeDelay_Get(chain + 1, endChain, LongestChain, LongestCDelay, LongestFDelay);
		}
	}
}

//here we are only passing in LongestChain so we can set SupportingPrisms to the chain length. this has nothing to do with the charge delay which we have already calculated
void BuildingExt::cPrismForwarding::SetChargeDelay_Set(int chain, DWORD* LongestCDelay, DWORD* LongestFDelay, int LongestChain) {
	auto pTargetTower = this->Owner->AttachedToObject;

	this->PrismChargeDelay = (LongestFDelay[chain] - pTargetTower->DelayBeforeFiring) + LongestCDelay[chain];
	pTargetTower->SupportingPrisms = (LongestChain - chain);
	if(this->PrismChargeDelay == 0) {
		//no delay, so start animations now
		if(pTargetTower->Type->BuildingAnim[BuildingAnimSlot::Special].Anim[0]) { //only if it actually has a special anim
			pTargetTower->DestroyNthAnim(BuildingAnimSlot::Active);
			pTargetTower->PlayNthAnim(BuildingAnimSlot::Special);
		}
	}
	for(auto Sender : this->Senders) {
		auto pData = BuildingExt::ExtMap.Find(Sender);
		pData->PrismForwarding.SetChargeDelay_Set(chain + 1, LongestCDelay, LongestFDelay, LongestChain);
	}
}

//Whenever a building is incapacitated, this method should be called to take it out of any prism network
//destruction, change sides, mind-control, sold, warped, emp, undeployed, low power, drained, lost operator
void BuildingExt::cPrismForwarding::RemoveFromNetwork(bool bCease) {
	auto pSlave = this->Owner->AttachedToObject;

	auto pSlaveTypeData = BuildingTypeExt::ExtMap.Find(pSlave->Type);
	if(!pSlaveTypeData) {
		return;
	}
	if(this->PrismChargeDelay || bCease) {
		//either hasn't started charging yet or animations have been reset so should go idle immediately
		pSlave->PrismStage = PrismChargeState::Idle;
		this->PrismChargeDelay = 0;
		pSlave->DelayBeforeFiring = 0;
		this->ModifierReserve = 0.0;
		this->DamageReserve = 0;
		//animations should be controlled by whatever incapacitated the tower so no need to mess with anims here
	}
	this->SetSupportTarget(nullptr);
	//finally, remove all the preceding slaves from the network
	for(int senderIdx = this->Senders.Count; senderIdx; senderIdx--) {
		if(BuildingClass *NextTower = this->Senders[senderIdx - 1]) {
			auto pData = BuildingExt::ExtMap.Find(NextTower);
			pData->PrismForwarding.RemoveFromNetwork(false);
		}
	}
}

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
