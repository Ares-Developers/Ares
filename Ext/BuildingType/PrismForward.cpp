#include "Body.h"
#include "../Building/Body.h"
#include <BulletClass.h>
#include <LaserDrawClass.h>

#include <vector>
#include <algorithm>

void BuildingTypeExt::cPrismForwarding::Initialize(BuildingTypeClass *pThis) {
	this->Enabled = NO;
	if (pThis == RulesClass::Instance->PrismType) {
		this->Enabled = YES;
	}
	this->Targets.AddItem(pThis);
}

void BuildingTypeExt::cPrismForwarding::LoadFromINIFile(BuildingTypeClass *pThis, CCINIClass* pINI) {
	const char * pID = pThis->ID;
	if(pINI->ReadString(pID, "PrismForwarding", "", Ares::readBuffer, Ares::readLength)) {
		if((strcmp(Ares::readBuffer, "yes") == 0) || (strcmp(Ares::readBuffer, "true") == 0)) {
			this->Enabled = YES;
		} else if(strcmp(Ares::readBuffer, "forward") == 0) {
			this->Enabled = FORWARD;
		} else if(strcmp(Ares::readBuffer, "attack") == 0) {
			this->Enabled = ATTACK;
		} else if((strcmp(Ares::readBuffer, "no") == 0) || (strcmp(Ares::readBuffer, "false"))== 0) {
			this->Enabled = NO;
		}
	}

	if (this->Enabled != NO) {
		if(pINI->ReadString(pID, "PrismForwarding.Targets", "", Ares::readBuffer, Ares::readLength)) {
			this->Targets.Clear();
			for(char *cur = strtok(Ares::readBuffer, ","); cur && *cur; cur = strtok(NULL, ",")) {
				BuildingTypeClass * target = BuildingTypeClass::Find(cur);
				if(target) {
					this->Targets.AddItem(target);
				}
			}
		}

		INI_EX exINI(pINI);

		this->MaxFeeds.Read(&exINI, pID, "PrismForwarding.MaxFeeds");
		this->MaxChainLength.Read(&exINI, pID, "PrismForwarding.MaxChainLength");
		this->MaxNetworkSize.Read(&exINI, pID, "PrismForwarding.MaxNetworkSize");
		this->SupportModifier.Read(&exINI, pID, "PrismForwarding.SupportModifier");
		Debug::Log("[PrismForwarding] SM is now %f\n", this->SupportModifier);
		this->DamageAdd.Read(&exINI, pID, "PrismForwarding.DamageAdd");
		this->SupportRange.Read(&exINI, pID, "PrismForwarding.SupportRange");

		if(this->SupportRange == 0) {
			if(WeaponTypeClass* Secondary = pThis->get_Secondary()) {
				this->SupportRange = Secondary->Range;
			} else if(WeaponTypeClass* Primary = pThis->get_Primary()) {
				this->SupportRange = Primary->Range;
			}
		} else if (this->SupportRange != -1) {
			this->SupportRange *= 256 //stored in leptons, not cells
		}
		Debug::Log("[PrismForwarding] SR is now %f\n", this->SupportRange);

		this->SupportDelay.Read(&exINI, pID, "PrismForwarding.SupportDelay");
		this->ToAllies.Read(&exINI, pID, "PrismForwarding.ToAllies");
		this->MyHeight.Read(&exINI, pID, "PrismForwarding.MyHeight");
		this->BreakSupport.Read(&exINI, pID, "PrismForwarding.BreakSupport");

		Debug::Log("[PrismForwarding] current ChargeDelay = %d\n", this->ChargeDelay);
		int ChargeDelay = pINI->ReadInteger(pID, "PrismForwarding.ChargeDelay", this->ChargeDelay);
		if (ChargeDelay >= 1) {
			this->ChargeDelay.Set(ChargeDelay);
		} else {
			Debug::Log("[Developer Error] %s has an invalid PrismForwarding.ChargeDelay (%d), overriding to 1.\n", pThis->ID, ChargeDelay);
		}

	}
	if (strcmp(pID, "ATESLA") == 0) {
		Debug::Log("[PrismForwarding] ATESLA: MNS=%d, SM=%f, CD=%u\n", this->MaxNetworkSize, this->SupportModifier, this->ChargeDelay);
	}
}

int BuildingTypeExt::cPrismForwarding::AcquireSlaves_MultiStage(BuildingClass *MasterTower, BuildingClass *TargetTower, int stage, int chain, int *NetworkSize, int *LongestChain) {
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
	Debug::Log("[PrismForwarding]: multistage called with stage %d, chain %d\n", stage, chain);
	int countSlaves = 0;
	if (stage == 0) {
		countSlaves += AcquireSlaves_SingleStage(MasterTower, TargetTower, stage, (chain + 1), NetworkSize, LongestChain);
	} else {
		BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(TargetTower);
		int senderIdx = 0;
		while(senderIdx < pTargetData->PrismForwarding.Senders.Count) {
			BuildingClass *SenderTower = pTargetData->PrismForwarding.Senders[senderIdx];
			countSlaves += AcquireSlaves_MultiStage(MasterTower, SenderTower, (stage - 1), (chain + 1), NetworkSize, LongestChain);
			++senderIdx;
		}
	}
	Debug::Log("[PrismForwarding]: multistage returning %d\n", countSlaves);
	return countSlaves;
}

int BuildingTypeExt::cPrismForwarding::AcquireSlaves_SingleStage
	(BuildingClass *MasterTower, BuildingClass *TargetTower, int stage, int chain, int *NetworkSize, int *LongestChain) {
	//set up immediate slaves for this particular tower

	BuildingTypeClass *pMasterType = MasterTower->Type;
	BuildingTypeExt::ExtData *pMasterTypeData = BuildingTypeExt::ExtMap.Find(pMasterType);
	BuildingTypeClass *pTargetType = TargetTower->Type;
	BuildingTypeExt::ExtData *pTargetTypeData = BuildingTypeExt::ExtMap.Find(pTargetType);

	signed int MaxFeeds = pTargetTypeData->PrismForwarding.MaxFeeds;
	signed int MaxNetworkSize = pMasterTypeData->PrismForwarding.MaxNetworkSize;
	signed int MaxChainLength = pMasterTypeData->PrismForwarding.MaxChainLength;

	if (MaxFeeds == 0
			|| (MaxChainLength != -1 && MaxChainLength < chain)
			|| (MaxNetworkSize != -1 && MaxNetworkSize <= *NetworkSize)) {
		Debug::Log("[PrismForwarding]: singlestage aborted. MaxFeeds=%d, CL=%d/%d, MNS=%d/%d\n", MaxFeeds, chain, MaxChainLength, *NetworkSize, MaxNetworkSize);
		return 0;
	}

	struct PrismTargetData {
		BuildingClass * Tower;
		int Distance;

		bool operator < (PrismTargetData const &rhs) {
			return this->Distance < rhs.Distance;
		}
	};

	CoordStruct MyPosition, curPosition;;
	TargetTower->GetPosition_2(&MyPosition);

	//first, find eligible towers
	std::vector<PrismTargetData> EligibleTowers;
	//for(int i = 0; i < TargetTower->Owner->Buildings.Count; ++i) {
	for (int i = 0; i < BuildingClass::Array->Count; ++i) {
		//if (BuildingClass *SlaveTower = B->Owner->Buildings[i]) {
		if (BuildingClass *SlaveTower = BuildingClass::Array->GetItem(i)) {
			if (ValidateSupportTower(MasterTower, TargetTower, SlaveTower)) {
				Debug::Log("PrismForwarding: SlaveTower confirmed eligible at stage %d, chain %d\n", stage, chain);
				SlaveTower->GetPosition_2(&curPosition);
				int Distance = MyPosition.DistanceFrom(curPosition);

				PrismTargetData pd = {SlaveTower, Distance};
				EligibleTowers.push_back(pd);
			}
		}
	}

	std::sort(EligibleTowers.begin(), EligibleTowers.end());
	//std::reverse(EligibleTowers.begin(), EligibleTowers.end());

	//now enslave the towers in order of proximity
	int iFeeds = 0;
	while (EligibleTowers.size() != 0 && (MaxFeeds == -1 || iFeeds < MaxFeeds) && (MaxNetworkSize == -1 || *NetworkSize < MaxNetworkSize)) {
		BuildingClass * nearestPrism = EligibleTowers[0].Tower;
		EligibleTowers.erase(EligibleTowers.begin());
		//we have a slave tower! do the bizzo
		++iFeeds;
		++NetworkSize;
		++TargetTower->SupportingPrisms; //Ares doesn't actually use this, but maintaining it anyway (as direct feeds only)
		CoordStruct FLH, Base = {0, 0, 0};
		TargetTower->GetFLH(&FLH, 0, Base);
		nearestPrism->DelayBeforeFiring = nearestPrism->Type->DelayedFireDelay;
		nearestPrism->PrismStage = pcs_Slave;
		nearestPrism->PrismTargetCoords = FLH;
		nearestPrism->DestroyNthAnim(BuildingAnimSlot::Active);
		nearestPrism->PlayNthAnim(BuildingAnimSlot::Special);

		BuildingExt::ExtData *pSlaveData = BuildingExt::ExtMap.Find(nearestPrism);
		BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(TargetTower);
		pSlaveData->PrismForwarding.SupportTarget = TargetTower;
		pTargetData->PrismForwarding.Senders.AddItem(nearestPrism);
		Debug::Log("PrismForwarding] Enslave loop end. ETS=%u F=%u MF=%d NS=%u MNS=%d\n", EligibleTowers.size(), iFeeds, MaxFeeds, *NetworkSize, MaxNetworkSize);
	}

	if (iFeeds != 0 && chain > *LongestChain) {
		++(*LongestChain);
	}

	return iFeeds;
}

bool BuildingTypeExt::cPrismForwarding::ValidateSupportTower(BuildingClass *MasterTower, BuildingClass *TargetTower, BuildingClass *SlaveTower) {
	//MasterTower = the firing tower. This might be the same as TargetTower, it might not.
	//TargetTower = the tower that we are forwarding to
	//SlaveTower = the tower being considered to support TargetTower
	if(SlaveTower->IsAlive) {
		if(SlaveTower->ReloadTimer.Ignorable()) {
			if(SlaveTower != TargetTower) {
				if (!SlaveTower->DelayBeforeFiring) {
					if(!SlaveTower->IsBeingDrained() && SlaveTower->GetCurrentMission() != mission_Attack) {
						BuildingExt::ExtData *pSlaveData = BuildingExt::ExtMap.Find(SlaveTower);
						BuildingTypeClass *pSlaveType = SlaveTower->Type;
						BuildingTypeExt::ExtData *pSlaveTypeData = BuildingTypeExt::ExtMap.Find(pSlaveType);
						if (pSlaveTypeData->PrismForwarding.Enabled == YES || pSlaveTypeData->PrismForwarding.Enabled == FORWARD) {
							//building is a prism tower
							BuildingTypeClass *pTargetType = TargetTower->Type;
							if (pSlaveTypeData->PrismForwarding.Targets.FindItemIndex(&pTargetType) != -1) {
								//valid type to forward from
								HouseClass *pMasterHouse = MasterTower->Owner;
								HouseClass *pTargetHouse = TargetTower->Owner;
								HouseClass *pSlaveHouse = SlaveTower->Owner;
								if ((pSlaveHouse == pTargetHouse && pSlaveHouse == pMasterHouse)
									|| (pSlaveTypeData->PrismForwarding.ToAllies
											&& pSlaveHouse->IsAlliedWith(pTargetHouse) && pSlaveHouse->IsAlliedWith(pMasterHouse))) {
									//ownership/alliance rules satisfied
									CellStruct tarCoords = TargetTower->GetCell()->MapCoords;
									CoordStruct MyPosition, curPosition;
									TargetTower->GetPosition_2(&MyPosition);
									SlaveTower->GetPosition_2(&curPosition);
									int Distance = MyPosition.DistanceFrom(curPosition);
									Debug::Log("[PrismForwarding] Distance=%u, SupportRange=%d\n", Distance, pSlaveTypeData->PrismForwarding.SupportRange);
									if(pSlaveTypeData->PrismForwarding.SupportRange == -1 || Distance <= pSlaveTypeData->PrismForwarding.SupportRange) {
										//within range
										return true;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return false;
}

int BuildingTypeExt::cPrismForwarding::SetPrismChargeDelay(BuildingClass *TargetTower) {
	BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(TargetTower);
	int thisDelay = 0;
	int senderIdx = 0;
	while(senderIdx < pTargetData->PrismForwarding.Senders.Count) {
		BuildingClass *SlaveTower = pTargetData->PrismForwarding.Senders[senderIdx];
		int slaveDelay = SetPrismChargeDelay(SlaveTower);
		if (slaveDelay > thisDelay) {
			thisDelay = slaveDelay;
		}
		++senderIdx;
	}
	BuildingTypeClass *pTargetType = TargetTower->Type;
	BuildingTypeExt::ExtData *pTargetTypeData = BuildingTypeExt::ExtMap.Find(pTargetType);
	if (senderIdx != 0) {
		//this is not the end of the chain so it must delay
		thisDelay += pTargetTypeData->PrismForwarding.ChargeDelay;
	}
	pTargetData->PrismForwarding.PrismChargeDelay = thisDelay;
	return thisDelay;
}

//Need to find out all the places that this should be called and call it!
//Death of tower, temporal, EMP, loss of power
void BuildingTypeExt::cPrismForwarding::RemoveSlave(BuildingClass *SlaveTower) {
	if (int PrismStage = SlaveTower->PrismStage) {
		BuildingExt::ExtData *pSlaveData = BuildingExt::ExtMap.Find(SlaveTower);
		OrphanSlave(SlaveTower);
		if (BuildingClass *TargetTower = pSlaveData->PrismForwarding.SupportTarget) {
			BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(TargetTower);
			signed int idx = pTargetData->PrismForwarding.Senders.FindItemIndex(&SlaveTower);
			if(idx != -1) {
				pTargetData->PrismForwarding.Senders.RemoveItem(idx);
			}
		}
		//assuming that this tower has been shut down so all charging activity ceases
		pSlaveData->PrismForwarding.PrismChargeDelay = 0;
		SlaveTower->DelayBeforeFiring = 0;
		pSlaveData->PrismForwarding.ModifierReserve = 0.0;
		pSlaveData->PrismForwarding.DamageReserve = 0;
		SlaveTower->DestroyNthAnim(BuildingAnimSlot::Special);
		//SlaveTower->PlayNthAnim(BuildingAnimSlot::Active); //do we need this?
		SlaveTower->PrismStage = pcs_Idle;
	}
}

void BuildingTypeExt::cPrismForwarding::OrphanSlave(BuildingClass *SlaveTower) {
	if (int PrismStage = SlaveTower->PrismStage) {
		BuildingExt::ExtData *pSlaveData = BuildingExt::ExtMap.Find(SlaveTower);
		BuildingTypeClass *pSlaveType = SlaveTower->Type;
		BuildingTypeExt::ExtData *pSlaveTypeData = BuildingTypeExt::ExtMap.Find(pSlaveType);
		if (pSlaveData->PrismForwarding.PrismChargeDelay) {
			//hasn't started charging yet, so can go idle immediately
			SlaveTower->PrismStage = pcs_Idle;
			pSlaveData->PrismForwarding.PrismChargeDelay = 0;
			SlaveTower->DelayBeforeFiring = 0;
			pSlaveData->PrismForwarding.ModifierReserve = 0.0;
			pSlaveData->PrismForwarding.DamageReserve = 0;
			SlaveTower->DestroyNthAnim(BuildingAnimSlot::Special);
			//SlaveTower->PlayNthAnim(BuildingAnimSlot::Active); //do we need this?
		} //else this is already charging so allow anim to continue
		if (BuildingClass *TargetTower = pSlaveData->PrismForwarding.SupportTarget) {
			BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(TargetTower);
			--TargetTower->SupportingPrisms;  //Ares doesn't actually use this, but maintaining it anyway (as direct feeds only)
			pSlaveData->PrismForwarding.SupportTarget = NULL; //thus making this slave tower an orphan
			int senderIdx = 0;
			while(senderIdx < pSlaveData->PrismForwarding.Senders.Count) {
				if (BuildingClass *NextTower = pTargetData->PrismForwarding.Senders[senderIdx]) {
					OrphanSlave(NextTower);
					++senderIdx;
				}
			}
		}
	}
}

