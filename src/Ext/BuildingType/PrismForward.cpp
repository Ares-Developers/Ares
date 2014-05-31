#include "Body.h"
#include "../Building/Body.h"
#include "../Techno/Body.h"
#include "../../Utilities/TemplateDef.h"
#include <BulletClass.h>
#include <HouseClass.h>
#include <LaserDrawClass.h>
#include <WarheadTypeClass.h>

#include <vector>
#include <algorithm>

void BuildingTypeExt::cPrismForwarding::Initialize(BuildingTypeClass *pThis) {
	this->Enabled = NO;
	if (pThis == RulesClass::Instance->PrismType) {
		this->Enabled = YES;
	}
	this->Targets.push_back(pThis);
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
		INI_EX exINI(pINI);

		this->Targets.Read(exINI, pID, "PrismForwarding.Targets");
		this->MaxFeeds.Read(exINI, pID, "PrismForwarding.MaxFeeds");
		this->MaxChainLength.Read(exINI, pID, "PrismForwarding.MaxChainLength");
		this->MaxNetworkSize.Read(exINI, pID, "PrismForwarding.MaxNetworkSize");
		this->SupportModifier.Read(exINI, pID, "PrismForwarding.SupportModifier");
		this->DamageAdd.Read(exINI, pID, "PrismForwarding.DamageAdd");
		this->ToAllies.Read(exINI, pID, "PrismForwarding.ToAllies");
		this->MyHeight.Read(exINI, pID, "PrismForwarding.MyHeight");
		this->BreakSupport.Read(exINI, pID, "PrismForwarding.BreakSupport");
		this->Intensity.Read(exINI, pID, "PrismForwarding.Intensity");
		
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
	for(int idxWeapon = 2; idxWeapon < 13; ++idxWeapon) { //13-18 is AlternateFLH0-4
		auto Weapon = elite ? pThis->get_EliteWeapon(idxWeapon) : pThis->get_Weapon(idxWeapon);
		if(!Weapon) {
			return idxWeapon;
		}
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
		for(int senderIdx = 0; senderIdx < pTargetData->PrismForwarding.Senders.Count; ++senderIdx) {
			BuildingClass *SenderTower = pTargetData->PrismForwarding.Senders[senderIdx];
			countSlaves += AcquireSlaves_MultiStage(MasterTower, SenderTower, (stage - 1), (chain + 1), NetworkSize, LongestChain);
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

	signed int MaxFeeds = pTargetTypeData->PrismForwarding.GetMaxFeeds();
	signed int MaxNetworkSize = pMasterTypeData->PrismForwarding.GetMaxNetworkSize();
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
				int Distance = (int)MyPosition.DistanceFrom(curPosition);

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
		nearestPrism->PrismStage = PrismChargeState::Slave;
		nearestPrism->PrismTargetCoords = FLH;

		auto pData = BuildingExt::ExtMap.Find(nearestPrism);
		pData->PrismForwarding.SetSupportTarget(TargetTower);
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
				if (pSlaveTypeData->PrismForwarding.Targets.Contains(pTargetType)) {
					//valid type to forward from
					HouseClass *pMasterHouse = MasterTower->Owner;
					HouseClass *pTargetHouse = TargetTower->Owner;
					HouseClass *pSlaveHouse = SlaveTower->Owner;
					if ((pSlaveHouse == pTargetHouse && pSlaveHouse == pMasterHouse)
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
