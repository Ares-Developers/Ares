#include "Body.h"
#include "../Building/Body.h"
#include "../Techno/Body.h"
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
		if((_strcmpi(Ares::readBuffer, "yes") == 0) || (_strcmpi(Ares::readBuffer, "true") == 0)) {
			this->Enabled = YES;
		} else if(_strcmpi(Ares::readBuffer, "forward") == 0) {
			this->Enabled = FORWARD;
		} else if(_strcmpi(Ares::readBuffer, "attack") == 0) {
			this->Enabled = ATTACK;
		} else if((_strcmpi(Ares::readBuffer, "no") == 0) || (_strcmpi(Ares::readBuffer, "false"))== 0) {
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
		this->DamageAdd.Read(&exINI, pID, "PrismForwarding.DamageAdd");
		this->ToAllies.Read(&exINI, pID, "PrismForwarding.ToAllies");
		this->MyHeight.Read(&exINI, pID, "PrismForwarding.MyHeight");
		this->BreakSupport.Read(&exINI, pID, "PrismForwarding.BreakSupport");
		this->Intensity.Read(&exINI, pID, "PrismForwarding.Intensity");
		
		int ChargeDelay = pINI->ReadInteger(pID, "PrismForwarding.ChargeDelay", this->ChargeDelay);
		if (ChargeDelay >= 1) {
			this->ChargeDelay.Set(ChargeDelay);
		} else {
			Debug::Log("[Developer Error] %s has an invalid PrismForwarding.ChargeDelay (%d), overriding to 1.\n", pThis->ID, ChargeDelay);
		}

		auto SuperWH = RulesClass::Instance->C4Warhead;
		if(!SuperWH) {
			SuperWH = WarheadTypeClass::Find("Super");
		}

		if(pINI->ReadString(pID, "PrismForwarding.SupportWeapon", "", Ares::readBuffer, Ares::readLength)) {
			if (WeaponTypeClass *cWeapon = WeaponTypeClass::FindOrAllocate(Ares::readBuffer)) {
				int idxWeapon = this->GetUnusedWeaponSlot(pThis, 0); //rookie weapons
				if (idxWeapon == -1) {
					Debug::FatalErrorAndExit(
						"BuildingType [%s] is a Prism Tower however there are no free\n"
						"weapon slots to assign the support weapon to.", pThis->ID);
				}
				this->SupportWeaponIndex = idxWeapon;
				if(!cWeapon->Warhead) {
					cWeapon->Warhead = SuperWH;
				}
				cWeapon->NeverUse = true; //the modder shouldn't be expected to have to set this
				CoordStruct supportFLH;
				pThis->set_Weapon(idxWeapon, cWeapon);
				//now get the FLH
				supportFLH = pThis->get_WeaponFLH(13); //AlternateFLH0
				if (supportFLH.X == 0 && supportFLH.Y == 0 && supportFLH.Z == 0) {
					//assuming that, for Prism Towers, this means the FLH was not set.
					supportFLH = pThis->get_WeaponFLH(0); //Primary
				}
				pThis->set_WeaponFLH(idxWeapon, supportFLH);
			}
		}

		if(pINI->ReadString(pID, "PrismForwarding.EliteSupportWeapon", "", Ares::readBuffer, Ares::readLength)) {
			if (WeaponTypeClass *cWeapon = WeaponTypeClass::FindOrAllocate(Ares::readBuffer)) {
				int idxWeapon = this->GetUnusedWeaponSlot(pThis, 1); //elite weapons
				if (idxWeapon == -1) {
					Debug::FatalErrorAndExit(
						"BuildingType [%s] is a Prism Tower however there are no free\n"
						"weapon slots to assign the elite support weapon to.", pThis->ID);
				}
				this->EliteSupportWeaponIndex = idxWeapon;
				if(!cWeapon->Warhead) {
					cWeapon->Warhead = SuperWH;
				}
				cWeapon->NeverUse = true; //the modder shouldn't be expected to have to set this
				CoordStruct supportFLH;
				pThis->set_EliteWeapon(idxWeapon, cWeapon);
				//now get the FLH
				supportFLH = pThis->get_WeaponFLH(14); //AlternateFLH1
				if (supportFLH.X == 0 && supportFLH.Y == 0 && supportFLH.Z == 0) {
					//assuming that, for Prism Towers, this means the FLH was not set.
					supportFLH = pThis->get_EliteWeaponFLH(0); //ElitePrimary
				}
				pThis->set_EliteWeaponFLH(idxWeapon, supportFLH);
			}
		}

	}
}

signed int BuildingTypeExt::cPrismForwarding::GetUnusedWeaponSlot(BuildingTypeClass *pThis, bool elite) {
	int idxWeapon = 1;
	while (++idxWeapon <= 12) {
		auto Weapon = elite
			? pThis->get_EliteWeapon(idxWeapon)
			: pThis->get_Weapon(idxWeapon)
		;
		if(!Weapon) {
				break;
		}
	}
	if (idxWeapon <= 12) { //13-18 is AlternateFLH0-4
		return idxWeapon;
	}
	return -1;
}

int BuildingTypeExt::cPrismForwarding::AcquireSlaves_MultiStage
	(BuildingClass *MasterTower, BuildingClass *TargetTower, int stage, int chain, int *NetworkSize, int *LongestChain) {
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
		return 0;
	}

	struct PrismTargetData {
		BuildingClass * Tower;
		int Distance;

		bool operator < (PrismTargetData const &rhs) {
			return this->Distance < rhs.Distance;
		}
	};

	CoordStruct MyPosition, curPosition;
	TargetTower->GetPosition_2(&MyPosition);

	//first, find eligible towers
	std::vector<PrismTargetData> EligibleTowers;
	//for(int i = 0; i < TargetTower->Owner->Buildings.Count; ++i) {
	for (int i = 0; i < BuildingClass::Array->Count; ++i) {
		//if (BuildingClass *SlaveTower = B->Owner->Buildings[i]) {
		if (BuildingClass *SlaveTower = BuildingClass::Array->GetItem(i)) {
			if (ValidateSupportTower(MasterTower, TargetTower, SlaveTower)) {
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
		++(*NetworkSize);
		//++TargetTower->SupportingPrisms; //Ares is now using this for longest backward chain of this tower, so don't set it here
		CoordStruct FLH, Base = {0, 0, 0};
		TargetTower->GetFLH(&FLH, 0, Base);
		nearestPrism->DelayBeforeFiring = nearestPrism->Type->DelayedFireDelay;
		nearestPrism->PrismStage = pcs_Slave;
		nearestPrism->PrismTargetCoords = FLH;

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

bool BuildingTypeExt::cPrismForwarding::ValidateSupportTower(
		BuildingClass *MasterTower, BuildingClass *TargetTower, BuildingClass *SlaveTower) {
	//MasterTower = the firing tower. This might be the same as TargetTower, it might not.
	//TargetTower = the tower that we are forwarding to
	//SlaveTower = the tower being considered to support TargetTower
	if(SlaveTower->IsAlive) {
		BuildingTypeClass *pSlaveType = SlaveTower->Type;
		BuildingTypeExt::ExtData *pSlaveTypeData = BuildingTypeExt::ExtMap.Find(pSlaveType);
		if (pSlaveTypeData->PrismForwarding.Enabled == YES || pSlaveTypeData->PrismForwarding.Enabled == FORWARD) {
			//building is a prism tower
			//get all the data we need
			TechnoExt::ExtData *pTechnoData = TechnoExt::ExtMap.Find(SlaveTower);
			BuildingExt::ExtData *pSlaveData = BuildingExt::ExtMap.Find(SlaveTower);
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
				if (pSlaveTypeData->PrismForwarding.Targets.FindItemIndex(&pTargetType) != -1) {
					//valid type to forward from
					HouseClass *pMasterHouse = MasterTower->Owner;
					HouseClass *pTargetHouse = TargetTower->Owner;
					HouseClass *pSlaveHouse = SlaveTower->Owner;
					if ((pSlaveHouse == pTargetHouse && pSlaveHouse == pMasterHouse)
						|| (pSlaveTypeData->PrismForwarding.ToAllies.Get()
							&& pSlaveHouse->IsAlliedWith(pTargetHouse)
							&& pSlaveHouse->IsAlliedWith(pMasterHouse))) {
						//ownership/alliance rules satisfied
						CellStruct tarCoords = TargetTower->GetCell()->MapCoords;
						CoordStruct MyPosition, curPosition;
						TargetTower->GetPosition_2(&MyPosition);
						SlaveTower->GetPosition_2(&curPosition);
						int Distance = MyPosition.DistanceFrom(curPosition);
						int SupportRange = 0;
						int idxSupport = -1;
						if (SlaveTower->Veterancy.IsElite()) {
							idxSupport = pSlaveTypeData->PrismForwarding.EliteSupportWeaponIndex;
						} else {
							idxSupport = pSlaveTypeData->PrismForwarding.SupportWeaponIndex;
						}
						if (idxSupport != -1) {
							if (WeaponTypeClass * supportWeapon = pSlaveType->get_Weapon(idxSupport)) {
								if (Distance < supportWeapon->MinimumRange) {
									return false; //below minimum range
								}
								SupportRange = supportWeapon->Range;
							}
						}
						if (SupportRange == 0) {
							//not specified on SupportWeapon so use Primary + 1 cell (Marshall chose to add the +1 cell default - see manual for reason)
							if (WeaponTypeClass * cPrimary = pSlaveType->get_Primary()) {
								SupportRange = cPrimary->Range + 256; //256 leptons == 1 cell
							}
						}
						if(SupportRange < 0	|| Distance <= SupportRange) {
							return true; //within range
						}
					}
				}
			}
		}
	}
	return false;
}

void BuildingTypeExt::cPrismForwarding::SetChargeDelay
	(BuildingClass * TargetTower, int LongestChain) {
	int ArrayLen = LongestChain + 1;
	DWORD *LongestCDelay = new DWORD[ArrayLen];
	memset(LongestCDelay, 0, ArrayLen * sizeof(DWORD));
	DWORD *LongestFDelay = new DWORD[ArrayLen];
	memset(LongestFDelay, 0, ArrayLen * sizeof(DWORD));
	
	int temp = 0;
	while (temp <= LongestChain) {
		LongestCDelay[temp] = 0;
		LongestFDelay[temp] = 0;
		++temp;
	}

	int endChain = LongestChain;
	while (endChain >= 0) {
		SetChargeDelay_Get(TargetTower, 0, endChain, LongestChain, LongestCDelay, LongestFDelay);
		--endChain;
	}

	SetChargeDelay_Set(TargetTower, 0, LongestCDelay, LongestFDelay, LongestChain);
	delete [] LongestFDelay;
	delete [] LongestCDelay;
}

void BuildingTypeExt::cPrismForwarding::SetChargeDelay_Get
	(BuildingClass * TargetTower, int chain, int endChain, int LongestChain, DWORD *LongestCDelay, DWORD *LongestFDelay) {
	BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(TargetTower);
	if (chain == endChain) {
		if (chain != LongestChain) {
			BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(TargetTower->Type);
			//update the delays for this chain
			unsigned int thisDelay = pTypeData->PrismForwarding.ChargeDelay.Get() + LongestCDelay[chain + 1];
			if ( thisDelay > LongestCDelay[chain]) {
				LongestCDelay[chain] = thisDelay;
			}
		}
		if ( TargetTower->DelayBeforeFiring > LongestFDelay[chain]) {
			LongestFDelay[chain] = TargetTower->DelayBeforeFiring;
		}
	} else {
		//ascend to the next chain
		int senderIdx = 0;
		while(senderIdx < pTargetData->PrismForwarding.Senders.Count) {
			BuildingClass *SenderTower = pTargetData->PrismForwarding.Senders[senderIdx];
			SetChargeDelay_Get(SenderTower, (chain + 1), endChain, LongestChain, LongestCDelay, LongestFDelay);
			++senderIdx;
		}
	}
}

//here we are only passing in LongestChain so we can set SupportingPrisms to the chain length. this has nothing to do with the charge delay which we have already calculated
void BuildingTypeExt::cPrismForwarding::SetChargeDelay_Set
	(BuildingClass * TargetTower, int chain, DWORD *LongestCDelay, DWORD *LongestFDelay, int LongestChain) {
	BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(TargetTower);
	pTargetData->PrismForwarding.PrismChargeDelay = (LongestFDelay[chain] - TargetTower->DelayBeforeFiring) + LongestCDelay[chain];
	TargetTower->SupportingPrisms = (LongestChain - chain);
	if (pTargetData->PrismForwarding.PrismChargeDelay == 0) {
		//no delay, so start animations now
		if (TargetTower->Type->BuildingAnim[BuildingAnimSlot::Special].Anim[0]) { //only if it actually has a special anim
			TargetTower->DestroyNthAnim(BuildingAnimSlot::Active);
			TargetTower->PlayNthAnim(BuildingAnimSlot::Special);
		}
	}
	int senderIdx = 0;
	while (senderIdx < pTargetData->PrismForwarding.Senders.Count) {
		BuildingClass *Sender = pTargetData->PrismForwarding.Senders[senderIdx];
		SetChargeDelay_Set(Sender, (chain + 1), LongestCDelay, LongestFDelay, LongestChain);
		++senderIdx;
	}
}


//Whenever a building is incapacitated, this method should be called to take it out of any prism network
//destruction, change sides, mind-control, sold, warped, emp, undeployed, low power, drained, lost operator
void BuildingTypeExt::cPrismForwarding::RemoveFromNetwork(BuildingClass *SlaveTower, bool bCease) {
	BuildingExt::ExtData *pSlaveData = BuildingExt::ExtMap.Find(SlaveTower);
	if(!pSlaveData) {
		return;
	}
	BuildingTypeClass *pSlaveType = SlaveTower->Type;
	BuildingTypeExt::ExtData *pSlaveTypeData = BuildingTypeExt::ExtMap.Find(pSlaveType);
	if(!pSlaveTypeData) {
		return;
	}
	if (pSlaveData->PrismForwarding.PrismChargeDelay || bCease) {
		//either hasn't started charging yet or animations have been reset so should go idle immediately
		SlaveTower->PrismStage = pcs_Idle;
		pSlaveData->PrismForwarding.PrismChargeDelay = 0;
		SlaveTower->DelayBeforeFiring = 0;
		pSlaveData->PrismForwarding.ModifierReserve = 0.0;
		pSlaveData->PrismForwarding.DamageReserve = 0;
		//animations should be controlled by whatever incapacitated the tower so no need to mess with anims here
	}
	if (BuildingClass *TargetTower = pSlaveData->PrismForwarding.SupportTarget) {
		//there is a target tower (so this is a slave rather than a master)
		BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(TargetTower);
		signed int idx = pTargetData->PrismForwarding.Senders.FindItemIndex(&SlaveTower);
		if(idx != -1) {
			pTargetData->PrismForwarding.Senders.RemoveItem(idx);
			--TargetTower->SupportingPrisms;  //Ares doesn't actually use this, but maintaining it anyway (as direct feeds only)
		}
		//slave tower is no longer reference by the target
		pSlaveData->PrismForwarding.SupportTarget = NULL; //slave tower no longer references the target
	}
	//finally, remove all the preceding slaves from the network
	for(int senderIdx = pSlaveData->PrismForwarding.Senders.Count; senderIdx; senderIdx--) {
		if (BuildingClass *NextTower = pSlaveData->PrismForwarding.Senders[senderIdx-1]) {
			RemoveFromNetwork(NextTower, false);
		}
	}
}

