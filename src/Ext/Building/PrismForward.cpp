#include "Body.h"

#include "../Techno/Body.h"
#include "../../Misc/SavegameDef.h"

#include <HouseClass.h>

#include <vector>
#include <algorithm>

bool BuildingExt::cPrismForwarding::Load(AresStreamReader &Stm, bool RegisterForChange) {
	// support pointer to this type
	Stm.RegisterChange(this);

	return Stm
		.Process(this->Senders, RegisterForChange)
		.Process(this->SupportTarget, RegisterForChange)
		.Process(this->PrismChargeDelay)
		.Process(this->ModifierReserve)
		.Process(this->DamageReserve)
		.Success();
}

bool BuildingExt::cPrismForwarding::Save(AresStreamWriter &Stm) const {
	// remember this object address
	Stm.RegisterChange(this);

	return Stm
		.Process(this->Senders)
		.Process(this->SupportTarget)
		.Process(this->PrismChargeDelay)
		.Process(this->ModifierReserve)
		.Process(this->DamageReserve)
		.Success();
}

int BuildingExt::cPrismForwarding::AcquireSlaves_MultiStage(BuildingExt::cPrismForwarding* const TargetTower, int const stage, int const chain, int& NetworkSize, int& LongestChain) {
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
	auto countSlaves = 0;
	if(stage == 0) {
		countSlaves += this->AcquireSlaves_SingleStage(TargetTower, stage, chain + 1, NetworkSize, LongestChain);
	} else {
		// do not think of using iterators or a ranged-for here. Senders grows and might reallocate.
		for(auto senderIdx = 0; senderIdx < TargetTower->Senders.Count; ++senderIdx) {
			auto const& SenderTower = TargetTower->Senders[senderIdx];
			countSlaves += this->AcquireSlaves_MultiStage(SenderTower, stage - 1, chain + 1, NetworkSize, LongestChain);
		}
	}
	return countSlaves;
}

int BuildingExt::cPrismForwarding::AcquireSlaves_SingleStage(BuildingExt::cPrismForwarding* const TargetTower, int const stage, int const chain, int& NetworkSize, int& LongestChain) {
	//set up immediate slaves for this particular tower

	auto const pMasterType = this->GetOwner()->Type;
	auto const pMasterTypeData = BuildingTypeExt::ExtMap.Find(pMasterType);
	auto const pTargetType = TargetTower->GetOwner()->Type;
	auto const pTargetTypeData = BuildingTypeExt::ExtMap.Find(pTargetType);

	auto const MaxFeeds = pTargetTypeData->PrismForwarding.GetMaxFeeds();
	auto const MaxNetworkSize = pMasterTypeData->PrismForwarding.GetMaxNetworkSize();
	auto const MaxChainLength = pMasterTypeData->PrismForwarding.MaxChainLength;

	if(MaxFeeds == 0
		|| (MaxChainLength != -1 && MaxChainLength < chain)
		|| (MaxNetworkSize != -1 && MaxNetworkSize <= NetworkSize)) {
		return 0;
	}

	struct PrismTargetData {
		cPrismForwarding* Tower;
		int Distance;

		bool operator < (const PrismTargetData &rhs) const {
			return this->Distance < rhs.Distance;
		}
	};

	CoordStruct MyPosition, curPosition;
	TargetTower->GetOwner()->GetPosition_2(&MyPosition);

	//first, find eligible towers
	std::vector<PrismTargetData> EligibleTowers;
	for(auto const& SlaveTower : *BuildingClass::Array) {
		auto const pSlaveData = BuildingExt::ExtMap.Find(SlaveTower);
		if(this->ValidateSupportTower(TargetTower, &pSlaveData->PrismForwarding)) {
			SlaveTower->GetPosition_2(&curPosition);
			int Distance = static_cast<int>(MyPosition.DistanceFrom(curPosition));
			PrismTargetData pd = {&pSlaveData->PrismForwarding, Distance};
			EligibleTowers.push_back(pd);
		}
	}

	std::stable_sort(EligibleTowers.begin(), EligibleTowers.end());

	//now enslave the towers in order of proximity
	auto iFeeds = 0;
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
		auto const nearestPrism = eligible.Tower->GetOwner();
		++iFeeds;
		++NetworkSize;
		//++TargetTower->SupportingPrisms; //Ares is now using this for longest backward chain of this tower, so don't set it here
		auto const FLH = TargetTower->GetOwner()->GetFLH(0, CoordStruct::Empty);
		nearestPrism->DelayBeforeFiring = nearestPrism->Type->DelayedFireDelay;
		nearestPrism->PrismStage = PrismChargeState::Slave;
		nearestPrism->PrismTargetCoords = FLH;

		eligible.Tower->SetSupportTarget(TargetTower);
	}

	if(iFeeds != 0 && chain > LongestChain) {
		++LongestChain;
	}

	return iFeeds;
}

bool BuildingExt::cPrismForwarding::ValidateSupportTower(BuildingExt::cPrismForwarding* const pTargetTower, BuildingExt::cPrismForwarding* const pSlaveTower) {
	//MasterTower = the firing tower. This might be the same as TargetTower, it might not.
	//TargetTower = the tower that we are forwarding to
	//SlaveTower = the tower being considered to support TargetTower
	auto const TargetTower = pTargetTower->GetOwner();
	auto const SlaveTower = pSlaveTower->GetOwner();
	if(SlaveTower->IsAlive) {
		auto const pSlaveType = SlaveTower->Type;
		auto const pSlaveTypeData = BuildingTypeExt::ExtMap.Find(pSlaveType);
		if(pSlaveTypeData->PrismForwarding.CanForward())
		{
			//building is a prism tower
			//get all the data we need
			auto const pTechnoData = TechnoExt::ExtMap.Find(SlaveTower);
			//BuildingExt::ExtData *pSlaveData = BuildingExt::ExtMap.Find(SlaveTower);
			auto const SlaveMission = SlaveTower->GetCurrentMission();
			//now check all the rules
			if(SlaveTower->ReloadTimer.Expired()
				&& SlaveTower != TargetTower
				&& !SlaveTower->DelayBeforeFiring
				&& !SlaveTower->IsBeingDrained()
				&& !SlaveTower->IsBeingWarpedOut()
				&& SlaveMission != Mission::Attack
				&& SlaveMission != Mission::Construction
				&& SlaveMission != Mission::Selling
				&& pTechnoData->IsPowered() //robot control logic
				&& pTechnoData->IsOperated() //operator logic
				&& SlaveTower->IsPowerOnline() //base-powered or overpowerer-powered
				&& !SlaveTower->IsUnderEMP()) //EMP logic - I think this should already be checked by IsPowerOnline() but included just to be sure
			{
				auto const pTargetType = TargetTower->Type;
				if(pSlaveTypeData->PrismForwarding.Targets.Contains(pTargetType)) {
					//valid type to forward from
					const auto pMasterHouse = this->GetOwner()->Owner;
					const auto pTargetHouse = TargetTower->Owner;
					const auto pSlaveHouse = SlaveTower->Owner;
					if((pSlaveHouse == pTargetHouse && pSlaveHouse == pMasterHouse)
						|| (pSlaveTypeData->PrismForwarding.ToAllies
						&& pSlaveHouse->IsAlliedWith(pTargetHouse)
						&& pSlaveHouse->IsAlliedWith(pMasterHouse)))
					{
						//ownership/alliance rules satisfied
						CoordStruct MyPosition, curPosition;
						TargetTower->GetPosition_2(&MyPosition);
						SlaveTower->GetPosition_2(&curPosition);
						auto const Distance = static_cast<int>(MyPosition.DistanceFrom(curPosition));
						int SupportRange = 0;
						int idxSupport = -1;
						if(SlaveTower->Veterancy.IsElite()) {
							idxSupport = pSlaveTypeData->PrismForwarding.EliteSupportWeaponIndex;
						} else {
							idxSupport = pSlaveTypeData->PrismForwarding.SupportWeaponIndex;
						}
						if(idxSupport != -1) {
							if(auto const supportWeapon = pSlaveType->Weapon[idxSupport].WeaponType) {
								if(Distance < supportWeapon->MinimumRange) {
									return false; //below minimum range
								}
								SupportRange = supportWeapon->Range;
							}
						}
						if(SupportRange == 0) {
							//not specified on SupportWeapon so use Primary + 1 cell (Marshall chose to add the +1 cell default - see manual for reason)
							if(auto const cPrimary = pSlaveType->Weapon[0].WeaponType) {
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

void BuildingExt::cPrismForwarding::SetChargeDelay(int const LongestChain) {
	auto const ArrayLen = LongestChain + 1;
	std::vector<DWORD> LongestCDelay(ArrayLen, 0);
	std::vector<DWORD> LongestFDelay(ArrayLen, 0);

	for(auto endChain = LongestChain; endChain >= 0; --endChain) {
		this->SetChargeDelay_Get(0, endChain, LongestChain, LongestCDelay.data(), LongestFDelay.data());
	}

	this->SetChargeDelay_Set(0, LongestCDelay.data(), LongestFDelay.data(), LongestChain);
}

void BuildingExt::cPrismForwarding::SetChargeDelay_Get(int const chain, int const endChain, int const LongestChain, DWORD* const LongestCDelay, DWORD* const LongestFDelay) {
	auto const TargetTower = this->GetOwner();

	if(chain == endChain) {
		if(chain != LongestChain) {
			auto const pTypeData = BuildingTypeExt::ExtMap.Find(TargetTower->Type);
			//update the delays for this chain
			auto const thisDelay = pTypeData->PrismForwarding.ChargeDelay + LongestCDelay[chain + 1];
			if(thisDelay > LongestCDelay[chain]) {
				LongestCDelay[chain] = thisDelay;
			}
		}
		if(TargetTower->DelayBeforeFiring > LongestFDelay[chain]) {
			LongestFDelay[chain] = TargetTower->DelayBeforeFiring;
		}
	} else {
		//ascend to the next chain
		for(auto const& SenderTower : this->Senders) {
			SenderTower->SetChargeDelay_Get(chain + 1, endChain, LongestChain, LongestCDelay, LongestFDelay);
		}
	}
}

//here we are only passing in LongestChain so we can set SupportingPrisms to the chain length. this has nothing to do with the charge delay which we have already calculated
void BuildingExt::cPrismForwarding::SetChargeDelay_Set(int const chain, DWORD const* const LongestCDelay, DWORD const* const LongestFDelay, int const LongestChain) {
	auto const pTargetTower = this->GetOwner();

	this->PrismChargeDelay = (LongestFDelay[chain] - pTargetTower->DelayBeforeFiring) + LongestCDelay[chain];
	pTargetTower->SupportingPrisms = (LongestChain - chain);
	if(this->PrismChargeDelay == 0) {
		//no delay, so start animations now
		if(pTargetTower->Type->GetBuildingAnim(BuildingAnimSlot::Special).Anim[0]) { //only if it actually has a special anim
			pTargetTower->DestroyNthAnim(BuildingAnimSlot::Active);
			pTargetTower->PlayNthAnim(BuildingAnimSlot::Special);
		}
	}
	for(auto const& Sender : this->Senders) {
		Sender->SetChargeDelay_Set(chain + 1, LongestCDelay, LongestFDelay, LongestChain);
	}
}

//Whenever a building is incapacitated, this method should be called to take it out of any prism network
//destruction, change sides, mind-control, sold, warped, emp, undeployed, low power, drained, lost operator
void BuildingExt::cPrismForwarding::RemoveFromNetwork(bool const bCease) {

	if(this->PrismChargeDelay || bCease) {
		//either hasn't started charging yet or animations have been reset so should go idle immediately
		this->PrismChargeDelay = 0;
		this->ModifierReserve = 0.0;
		this->DamageReserve = 0;

		auto pSlave = this->GetOwner();
		pSlave->PrismStage = PrismChargeState::Idle;
		pSlave->DelayBeforeFiring = 0;
		//animations should be controlled by whatever incapacitated the tower so no need to mess with anims here
	}

	this->SetSupportTarget(nullptr);

	//finally, remove all the preceding slaves from the network
	for(auto senderIdx = this->Senders.Count; senderIdx; --senderIdx) {
		if(auto const& NextTower = this->Senders[senderIdx - 1]) {
			NextTower->RemoveFromNetwork(false);
		}
	}
}

void BuildingExt::cPrismForwarding::SetSupportTarget(BuildingExt::cPrismForwarding* const pTargetTower) {
	// meet the new tower, same as the old tower
	if(this->SupportTarget == pTargetTower) {
		return;
	}

	// if the target tower is already set, disconnect it by removing it from the old target tower's sender list
	if(auto const pOldTarget = this->SupportTarget) {
		if(!pOldTarget->Senders.Remove(this)) {
			Debug::Log(Debug::Severity::Warning, "PrismForwarding::SetSupportTarget: Old target tower (%p) did not consider this tower (%p) as its sender.\n",
				pOldTarget->GetOwner(), this->GetOwner());
		}
	}

	this->SupportTarget = pTargetTower;

	// set the new tower as support target
	if(pTargetTower) {
		if(pTargetTower->Senders.FindItemIndex(this) == -1) {
			pTargetTower->Senders.AddItem(this);
		} else {
			Debug::Log(Debug::Severity::Warning, "PrismForwarding::SetSupportTarget: Tower (%p) is already in new target tower's (%p) sender list.\n",
				this->GetOwner(), pTargetTower->GetOwner());
		}
	}
}

void BuildingExt::cPrismForwarding::RemoveAllSenders() {
	// disconnect all sender towers from their support target, which is me
	for(auto senderIdx = this->Senders.Count; senderIdx; senderIdx--) {
		if(auto const& NextTower = this->Senders[senderIdx - 1]) {
			NextTower->SetSupportTarget(nullptr);
		}
	}

	// log if not all senders could be removed
	if(this->Senders.Count) {
		Debug::Log(Debug::Severity::Warning,
			"PrismForwarding::RemoveAllSenders: Tower (%p) still has %d senders after removal completed.\n",
			this->GetOwner(), this->Senders.Count);
		for(auto i = 0; i < this->Senders.Count; ++i) {
			Debug::Log(Debug::Severity::Warning, "Sender %03d: %p\n", i, this->Senders[i]->GetOwner());
		}

		this->Senders.Clear();
	}
}
