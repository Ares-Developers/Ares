#include "Body.h"
#include "../Building/Body.h"
#include <BulletClass.h>
#include <LaserDrawClass.h>

void BuildingTypeExt::cPrismForwarding::Initialize(BuildingTypeClass *pThis) {
	this->Enabled = NO;
	if (pThis == RulesClass::Instance->PrismType) {
		this->Enabled = YES;
	}
	this->Targets.AddItem(pThis);
	this->MaxFeeds = RulesClass::Instance->PrismSupportMax;
	this->MaxChainLength = 1;
	this->MaxNetworkSize = RulesClass::Instance->PrismSupportMax;
	this->SupportModifier = RulesClass::Instance->PrismSupportModifier;
	this->DamageAdd = 0;

	if(WeaponTypeClass* Secondary = pThis->get_Weapon(1)) {
		this->ForwardingRange = Secondary->Range;
	} else if(WeaponTypeClass* Primary = pThis->get_Weapon(0)) {
		this->ForwardingRange = Primary->Range;
	} else {
		this->ForwardingRange = 0;
	}

	this->ForwardingRange = 0;
	this->SupportDelay = RulesClass::Instance->PrismSupportDelay;
	this->ToAllies = false;
	this->MyHeight = RulesClass::Instance->PrismSupportHeight;
	this->BreakSupport = false;
	this->ChargeDelay = 1;
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

		this->MaxFeeds = pINI->ReadInteger(pID, "PrismForwarding.MaxFeeds", this->MaxFeeds);
		this->MaxChainLength = pINI->ReadInteger(pID, "PrismForwarding.MaxChainLength", this->MaxChainLength);
		this->MaxNetworkSize = pINI->ReadInteger(pID, "PrismForwarding.MaxNetworkSize", this->MaxNetworkSize);
		this->SupportModifier = pINI->ReadDouble(pID, "PrismForwarding.SupportModifier", this->SupportModifier);
		this->DamageAdd = pINI->ReadInteger(pID, "PrismForwarding.DamageAdd", this->DamageAdd);
		this->ForwardingRange = pINI->ReadInteger(pID, "PrismForwarding.ForwardingRange", this->ForwardingRange);
		this->SupportDelay = pINI->ReadInteger(pID, "PrismForwarding.SupportDelay", this->SupportDelay);
		this->ToAllies = pINI->ReadBool(pID, "PrismForwarding.ToAllies", this->ToAllies);
		this->MyHeight = pINI->ReadInteger(pID, "PrismForwarding.MyHeight", this->MyHeight);
		this->BreakSupport = pINI->ReadBool(pID, "PrismForwarding.BreakSupport", this->BreakSupport);

		Debug::Log("current ChargeDelay = %d\n", this->ChargeDelay);
		int ChargeDelay = pINI->ReadInteger(pID, "PrismForwarding.ChargeDelay", this->ChargeDelay);
		if (ChargeDelay >= 1) {
			this->ChargeDelay = ChargeDelay;
		} else {
			Debug::Log("%s has an invalid PrismForwarding.ChargeDelay (%d), overriding to 1.\n", pThis->ID, ChargeDelay);
		}

	}
	if (strcmp(pID, "ATESLA") == 0) {
		Debug::Log("ATESLA: MNS=%d, SM=%d, CD=%d\n", this->MaxNetworkSize, this->SupportModifier, this->ChargeDelay);
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
	Debug::Log("PrismForwarding: multistage called with stage %d, chain %d\n", stage, chain);
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
	Debug::Log("PrismForwarding: multistage returning %d\n", countSlaves);
	return countSlaves;
}

int BuildingTypeExt::cPrismForwarding::AcquireSlaves_SingleStage
	(BuildingClass *MasterTower, BuildingClass *TargetTower, int stage, int chain, int *NetworkSize, int *LongestChain) {
	//set up immediate slaves for this particular tower

	BuildingTypeClass *pMasterType = MasterTower->Type;
	BuildingTypeExt::ExtData *pMasterTypeData = BuildingTypeExt::ExtMap.Find(pMasterType);
	BuildingTypeClass *pTargetType = TargetTower->Type;
	BuildingTypeExt::ExtData *pTargetTypeData = BuildingTypeExt::ExtMap.Find(pTargetType);

	if ((pTargetTypeData->PrismForwarding.MaxFeeds == 0)
			|| (pMasterTypeData->PrismForwarding.MaxChainLength != -1 && pMasterTypeData->PrismForwarding.MaxChainLength < chain)
			|| (pMasterTypeData->PrismForwarding.MaxNetworkSize != -1 && pMasterTypeData->PrismForwarding.MaxNetworkSize <= *NetworkSize)) {
		Debug::Log("PrismForwarding: singlestage aborted\n");
		return 0;
	}

	//first, find eligible towers
	DynamicVectorClass<BuildingClass*> EligibleTowers;
	//for(int i = 0; i < TargetTower->Owner->Buildings.Count; ++i) {
	for (int i = 0; i < BuildingClass::Array->Count; ++i) {
		//if (BuildingClass *SlaveTower = B->Owner->Buildings[i]) {
		if (BuildingClass *SlaveTower = (BuildingClass*)BuildingClass::Array->GetItem(i)) {
			Debug::Log("PrismForwarding: checking if SlaveTower is eligible at stage %d, chain %d\n", stage, chain);
			if (ValidateSupportTower(MasterTower, TargetTower, SlaveTower)) {
				Debug::Log("PrismForwarding: SlaveTower confirmed eligible\n");
				EligibleTowers.AddItem(SlaveTower);
			}
		}
	}

	//now enslave the towers in order of proximity
	int iFeeds = 0;
	int MaxFeeds = pTargetTypeData->PrismForwarding.MaxFeeds;
	int MaxNetworkSize = pMasterTypeData->PrismForwarding.MaxNetworkSize;
	Debug::Log("PrismForwarding: singlestage checkpoint 01\n");
	while (EligibleTowers.Count != 0 && (MaxFeeds == -1 || iFeeds < MaxFeeds) && (MaxNetworkSize == -1 || *NetworkSize < MaxNetworkSize)) {
		int nearestDistance = 0x7FFFFFFF;
		BuildingClass * nearestPrism = NULL;
		for (int i = 0; i < EligibleTowers.Count; ++i) {
			BuildingClass *SlaveTower = EligibleTowers.GetItem(i);
			CoordStruct MyPosition, curPosition;
			TargetTower->GetPosition_2(&MyPosition);
			SlaveTower->GetPosition_2(&curPosition);
			int Distance = MyPosition.DistanceFrom(curPosition);
			if(!nearestPrism || Distance < nearestDistance) {
				nearestPrism = SlaveTower;
				nearestDistance = Distance;
			}
		}
		Debug::Log("PrismForwarding: singlestage checkpoint 02\n");
		//we have a slave tower! do the bizzo
		signed int idx = EligibleTowers.FindItemIndex(&nearestPrism);
		if(idx != -1) {
			EligibleTowers.RemoveItem(idx);
		}
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
							Debug::Log("PrismForwarding: validate checkpoint 01\n");
							if (pSlaveTypeData->PrismForwarding.Targets.FindItemIndex(&pTargetType) != -1) {
								Debug::Log("PrismForwarding: validate checkpoint 02\n");
								//valid type to forward from
								HouseClass *pMasterHouse = MasterTower->Owner;
								HouseClass *pTargetHouse = TargetTower->Owner;
								HouseClass *pSlaveHouse = SlaveTower->Owner;
								if ((pSlaveHouse == pTargetHouse && pSlaveHouse == pMasterHouse)
									|| (pSlaveTypeData->PrismForwarding.ToAllies
											&& pSlaveHouse->IsAlliedWith(pTargetHouse) && pSlaveHouse->IsAlliedWith(pMasterHouse))) {
									//ownership/alliance rules satisfied
									Debug::Log("PrismForwarding: validate checkpoint 03\n");
									CellStruct tarCoords = TargetTower->GetCell()->MapCoords;
									CoordStruct MyPosition, curPosition;
									TargetTower->GetPosition_2(&MyPosition);
									SlaveTower->GetPosition_2(&curPosition);
									int Distance = MyPosition.DistanceFrom(curPosition);
									if(Distance <= pSlaveTypeData->PrismForwarding.ForwardingRange) {
										//within range
										Debug::Log("PrismForwarding: validate checkpoint 04\n");
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

