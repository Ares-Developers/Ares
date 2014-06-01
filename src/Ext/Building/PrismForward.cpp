#include "Body.h"

#include "../Techno/Body.h"

#include <HouseClass.h>

#include <vector>
#include <algorithm>

int BuildingExt::cPrismForwarding::AcquireSlaves_MultiStage(BuildingClass* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain) {
	//get all slaves for a specific stage in the prism chain
	//this is done for all sibling chains in parallel, so we prefer multiple short chains over one really long chain
	//towers should be added in the following way:
	// 1---2---4---6
	// |        \
	// |         7
	// |
	// 3---5--8
	// as opposed to
	// 1---2---3---4
	// |          /
	// |         5
	// |
	// 6---7--8
	// ...which would not be as good.
	int countSlaves = 0;
	if(stage == 0) {
		countSlaves += this->AcquireSlaves_SingleStage(TargetTower, stage, chain + 1, NetworkSize, LongestChain);
	} else {
		auto pTargetData = BuildingExt::ExtMap.Find(TargetTower);
		for(int senderIdx = 0; senderIdx < pTargetData->PrismForwarding.Senders.Count; ++senderIdx) {
			auto SenderTower = pTargetData->PrismForwarding.Senders[senderIdx];
			countSlaves += this->AcquireSlaves_MultiStage(SenderTower, stage - 1, chain + 1, NetworkSize, LongestChain);
		}
	}
	return countSlaves;
}

int BuildingExt::cPrismForwarding::AcquireSlaves_SingleStage(BuildingClass* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain) {
	auto MasterTower = this->Owner->AttachedToObject;

	//set up immediate slaves for this particular tower

	BuildingTypeClass *pMasterType = MasterTower->Type;
	BuildingTypeExt::ExtData *pMasterTypeData = BuildingTypeExt::ExtMap.Find(pMasterType);
	BuildingTypeClass *pTargetType = TargetTower->Type;
	BuildingTypeExt::ExtData *pTargetTypeData = BuildingTypeExt::ExtMap.Find(pTargetType);

	signed int MaxFeeds = pTargetTypeData->PrismForwarding.GetMaxFeeds();
	signed int MaxNetworkSize = pMasterTypeData->PrismForwarding.GetMaxNetworkSize();
	signed int MaxChainLength = pMasterTypeData->PrismForwarding.MaxChainLength;

	if(MaxFeeds == 0
		|| (MaxChainLength != -1 && MaxChainLength < chain)
		|| (MaxNetworkSize != -1 && MaxNetworkSize <= NetworkSize)) {
		return 0;
	}

	struct PrismTargetData {
		BuildingClass * Tower;
		int Distance;

		bool operator < (const PrismTargetData &rhs) const {
			return this->Distance < rhs.Distance;
		}
	};

	CoordStruct MyPosition, curPosition;
	TargetTower->GetPosition_2(&MyPosition);

	//first, find eligible towers
	std::vector<PrismTargetData> EligibleTowers;
	for(auto SlaveTower : *BuildingClass::Array) {
		if(this->ValidateSupportTower(TargetTower, SlaveTower)) {
			SlaveTower->GetPosition_2(&curPosition);
			int Distance = static_cast<int>(MyPosition.DistanceFrom(curPosition));
			PrismTargetData pd = {SlaveTower, Distance};
			EligibleTowers.push_back(pd);
		}
	}

	std::stable_sort(EligibleTowers.begin(), EligibleTowers.end());

	//now enslave the towers in order of proximity
	int iFeeds = 0;
	for(const auto& eligible : EligibleTowers) {
		// feed limit enabled and reached
		if(MaxFeeds != -1 && iFeeds >= MaxFeeds) {
			break;
		}

		// network size limit enabled and reached
		if(MaxNetworkSize != -1 && NetworkSize >= MaxNetworkSize) {
			break;
		}

		//we have a slave tower! do the bizzo
		auto nearestPrism = eligible.Tower;
		++iFeeds;
		++NetworkSize;
		//++TargetTower->SupportingPrisms; //Ares is now using this for longest backward chain of this tower, so don't set it here
		CoordStruct FLH, Base = {0, 0, 0};
		TargetTower->GetFLH(&FLH, 0, Base);
		nearestPrism->DelayBeforeFiring = nearestPrism->Type->DelayedFireDelay;
		nearestPrism->PrismStage = PrismChargeState::Slave;
		nearestPrism->PrismTargetCoords = FLH;

		auto pData = BuildingExt::ExtMap.Find(nearestPrism);
		pData->PrismForwarding.SetSupportTarget(TargetTower);
	}

	if(iFeeds != 0 && chain > LongestChain) {
		++LongestChain;
	}

	return iFeeds;
}

bool BuildingExt::cPrismForwarding::ValidateSupportTower(BuildingClass* TargetTower, BuildingClass* SlaveTower) {
	auto MasterTower = this->Owner->AttachedToObject;

	//MasterTower = the firing tower. This might be the same as TargetTower, it might not.
	//TargetTower = the tower that we are forwarding to
	//SlaveTower = the tower being considered to support TargetTower
	if(SlaveTower->IsAlive) {
		BuildingTypeClass *pSlaveType = SlaveTower->Type;
		BuildingTypeExt::ExtData *pSlaveTypeData = BuildingTypeExt::ExtMap.Find(pSlaveType);
		if(pSlaveTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::YES
			|| pSlaveTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::FORWARD)
		{
			//building is a prism tower
			//get all the data we need
			TechnoExt::ExtData *pTechnoData = TechnoExt::ExtMap.Find(SlaveTower);
			//BuildingExt::ExtData *pSlaveData = BuildingExt::ExtMap.Find(SlaveTower);
			int SlaveMission = SlaveTower->GetCurrentMission();
			//now check all the rules
			if(SlaveTower->ReloadTimer.Ignorable()
				&& SlaveTower != TargetTower
				&& !SlaveTower->DelayBeforeFiring
				&& !SlaveTower->IsBeingDrained()
				&& !SlaveTower->IsBeingWarpedOut()
				&& SlaveMission != mission_Attack
				&& SlaveMission != mission_Construction
				&& SlaveMission != mission_Selling
				&& pTechnoData->IsPowered() //robot control logic
				&& pTechnoData->IsOperated() //operator logic
				&& SlaveTower->IsPowerOnline() //base-powered or overpowerer-powered
				&& !SlaveTower->IsUnderEMP() //EMP logic - I think this should already be checked by IsPowerOnline() but included just to be sure
			) {
				BuildingTypeClass *pTargetType = TargetTower->Type;
				if(pSlaveTypeData->PrismForwarding.Targets.Contains(pTargetType)) {
					//valid type to forward from
					HouseClass *pMasterHouse = MasterTower->Owner;
					HouseClass *pTargetHouse = TargetTower->Owner;
					HouseClass *pSlaveHouse = SlaveTower->Owner;
					if((pSlaveHouse == pTargetHouse && pSlaveHouse == pMasterHouse)
						|| (pSlaveTypeData->PrismForwarding.ToAllies
						&& pSlaveHouse->IsAlliedWith(pTargetHouse)
						&& pSlaveHouse->IsAlliedWith(pMasterHouse))) {
						//ownership/alliance rules satisfied
						CellStruct tarCoords = TargetTower->GetCell()->MapCoords;
						CoordStruct MyPosition, curPosition;
						TargetTower->GetPosition_2(&MyPosition);
						SlaveTower->GetPosition_2(&curPosition);
						int Distance = (int)MyPosition.DistanceFrom(curPosition);
						int SupportRange = 0;
						int idxSupport = -1;
						if(SlaveTower->Veterancy.IsElite()) {
							idxSupport = pSlaveTypeData->PrismForwarding.EliteSupportWeaponIndex;
						} else {
							idxSupport = pSlaveTypeData->PrismForwarding.SupportWeaponIndex;
						}
						if(idxSupport != -1) {
							if(WeaponTypeClass * supportWeapon = pSlaveType->get_Weapon(idxSupport)) {
								if(Distance < supportWeapon->MinimumRange) {
									return false; //below minimum range
								}
								SupportRange = supportWeapon->Range;
							}
						}
						if(SupportRange == 0) {
							//not specified on SupportWeapon so use Primary + 1 cell (Marshall chose to add the +1 cell default - see manual for reason)
							if(WeaponTypeClass * cPrimary = pSlaveType->get_Primary()) {
								SupportRange = cPrimary->Range + 256; //256 leptons == 1 cell
							}
						}
						if(SupportRange < 0 || Distance <= SupportRange) {
							return true; //within range
						}
					}
				}
			}
		}
	}
	return false;
}

void BuildingExt::cPrismForwarding::SetChargeDelay(int LongestChain) {
	int ArrayLen = LongestChain + 1;
	std::vector<DWORD> LongestCDelay(ArrayLen, 0);
	std::vector<DWORD> LongestFDelay(ArrayLen, 0);

	for(int endChain = LongestChain; endChain >= 0; --endChain) {
		this->SetChargeDelay_Get(0, endChain, LongestChain, LongestCDelay.data(), LongestFDelay.data());
	}

	this->SetChargeDelay_Set(0, LongestCDelay.data(), LongestFDelay.data(), LongestChain);
}

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

void BuildingExt::cPrismForwarding::AnnounceInvalidPointer(void* ptr, bool removed) {
	// verify that ptr points to an existing object that is a building without
	// accessing any of its fields or members.
	if(auto pExt = ExtMap.Find(static_cast<BuildingClass*>(ptr))) {
		auto bld = pExt->AttachedToObject;
		if(bld == this->SupportTarget) {
			Debug::Log("Should remove my support target\n");
		}
		auto senderIdx = this->Senders.FindItemIndex(bld);
		if(senderIdx != -1) {
			Debug::Log("Should remove my sender #%d\n", senderIdx);
		}
		pExt->PrismForwarding.RemoveFromNetwork(true);
		if(bld == this->SupportTarget) {
			_snprintf_s(Ares::readBuffer, Ares::readLength - 1, "Prism Forwarder (ExtData %p) failed to remove support target\n", this->Owner);
			Debug::FatalError(true);
			Debug::Exit();
		}
		senderIdx = this->Senders.FindItemIndex(bld);
		if(senderIdx != -1) {
			_snprintf_s(Ares::readBuffer, Ares::readLength - 1, "Prism Forwarder (ExtData %p) failed to remove sender #%d\n", this->Owner, senderIdx);
			Debug::FatalError(true);
			Debug::Exit();
		}
	}
}
