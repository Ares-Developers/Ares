#include "Body.h"
#include "../House/Body.h"
#include "../Techno/Body.h"
#include <MouseClass.h>

#include <algorithm>
#include <functional>

// This function controls the availability of super weapons. If a you want to
// add to or change the way the game thinks a building provides a super weapon,
// change the lambda UpdateStatus. Available means this super weapon exists at
// all. Setting it to false removes the super weapon. PowerSourced controls
// whether the super weapon charges or can be used.
DEFINE_HOOK(50AF10, HouseClass_CheckSWs, 5)
{
	GET(HouseClass *, pThis, ECX);

	// cache all super weapon statuses
	struct SWStatus {
		bool Available;
		bool PowerSourced;
		bool Charging;
	};
	SWStatus *Statuses = new SWStatus[pThis->Supers.Count];
	for(int i = 0; i < pThis->Supers.Count; ++i) {
		Statuses[i].Available = false;
		Statuses[i].PowerSourced = false;
		Statuses[i].Charging = false;
	}

	// look at every sane building this player owns, if it is not defeated already.
	if(!pThis->Defeated) {
		for(int idxBld = 0; idxBld < pThis->Buildings.Count; ++idxBld) {
			BuildingClass * pBld = pThis->Buildings.GetItem(idxBld);
			if(pBld->IsAlive && !pBld->InLimbo) {
				TechnoExt::ExtData *pExt = TechnoExt::ExtMap.Find(pBld);

				// the super weapon status update lambda.
				auto UpdateStatus = [=](int idxSW) {
					if(idxSW > -1) {
						Statuses[idxSW].Available = true;
						if(!Statuses[idxSW].PowerSourced || !Statuses[idxSW].Charging) {
							bool validBuilding = pBld->HasPower
								&& pExt->IsOperated()
								&& !pBld->IsUnderEMP();
							
							if(!Statuses[idxSW].PowerSourced) {
								Statuses[idxSW].PowerSourced = validBuilding;
							}

							if(!Statuses[idxSW].Charging) {
								Statuses[idxSW].Charging = validBuilding
									&& !pBld->IsBeingWarpedOut()
									&& (pBld->CurrentMission != mission_Selling)
									&& (pBld->QueuedMission != mission_Selling)
									&& (pBld->CurrentMission != mission_Construction)
									&& (pBld->QueuedMission != mission_Construction);
							}
						}
					}
				};

				// check for upgrades. upgrades can give super weapons, too.
				for(int i = 0; i < 3; ++i) {
					if(BuildingTypeClass *Upgrade = pBld->Upgrades[i]) {
						UpdateStatus(Upgrade->SuperWeapon);
						UpdateStatus(Upgrade->SuperWeapon2);
					}
				}

				// look for the main building.
				UpdateStatus(pBld->FirstActiveSWIdx());
				UpdateStatus(pBld->SecondActiveSWIdx());
			}
		}
	}

	// now update every super weapon that is valid.
	for(int idxSW = 0; idxSW < pThis->Supers.Count; ++idxSW) {
		SuperClass *pSW = pThis->Supers[idxSW];
		SuperWeaponTypeClass * pSWType = pSW->Type;

		// if this weapon has not been granted there's no need to update
		if(pSW->Granted) {

			// is this a super weapon to be updated?
			// sw is bound to a building and no single-shot => create goody otherwise
			bool isCreateGoody = (!pSW->unknown_bool_60 || pSW->Quantity);
			bool needsUpdate = !isCreateGoody || pThis->Defeated;
			if(needsUpdate) {

				// super weapons of defeated players are removed.
				if(!pThis->Defeated) {

					// turn off super weapons that are disallowed.
					if(!Unsorted::SWAllowed) {
						if(pSWType->DisableableFromShell) {
							Statuses[idxSW].Available = false;
						}
					}

					// there is at least one available and powered building,
					// but the house is generally on low power.
					if(pThis->PowerOutput < pThis->PowerDrain) {
						Statuses[idxSW].PowerSourced = false;
					}
				} else {
					// remove. owner is defeated.
					Statuses[idxSW].Available = false;
				}

				// shut down or power up super weapon and decide whether
				// a sidebar tab update is needed.
				bool update = false;
				if(!Statuses[idxSW].Available || pThis->Defeated) {
					update = (pSW->Lose() && HouseClass::Player);
				} else if(Statuses[idxSW].Charging && !pSW->IsPowered()) {
					update = pSW->IsOnHold && pSW->SetOnHold(false);
				} else if(!Statuses[idxSW].Charging && !pSW->IsPowered()) {
					update = !pSW->IsOnHold && pSW->SetOnHold(true);
				} else if(!Statuses[idxSW].PowerSourced) {
					update = (pSW->IsPowered() && pSW->SetOnHold(true));
				} else {
					update = (Statuses[idxSW].PowerSourced && pSW->SetOnHold(false));
				}

				// update only if needed.
				if(update) {
					// only the human player can see the sidebar.
					if(pThis == HouseClass::Player) {
						if(Unsorted::CurrentSWType == idxSW) {
							Unsorted::CurrentSWType = -1;
						}
						int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, pSWType->GetArrayIndex(), 0);
						MouseClass::Instance->RepaintSidebar(idxTab);
					}
					pThis->ShouldRecheckTechTree = true;
				}
			}
		}
	}

	// clean up.
	delete [] Statuses;
	Statuses = NULL;

	return 0x50B1CA;
}

// a ChargeDrain SW expired - fire it to trigger status update
DEFINE_HOOK(6CBD86, SuperClass_Progress_Charged, 7)
{
	GET(SuperClass *, pSuper, ESI);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(pSuper->Owner);
	pHouseData->SetFirestormState(0);

	return 0;
}

// a ChargeDrain SW was clicked while it was active - fire to trigger status update
DEFINE_HOOK(6CB979, SuperClass_ClickFire, 6)
{
	GET(SuperClass *, pSuper, ESI);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(pSuper->Owner);
	pHouseData->SetFirestormState(0);

	return 0;
}

// SW was lost (source went away)
DEFINE_HOOK(6CB7BA, SuperClass_Lose, 6)
{
	GET(SuperClass *, pSuper, ECX);
	if(pSuper->Type->UseChargeDrain) {
		HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(pSuper->Owner);
		pHouseData->SetFirestormState(0);
	}

	return 0;
}

// rewriting OnHold to support ChargeDrain
DEFINE_HOOK(6CB4D0, SuperClass_SetOnHold, 6)
{
	GET(SuperClass *, pSuper, ECX);
	GET_STACK(bool, OnHold, 0x4);
	OnHold = !!OnHold;
	if(!pSuper->Granted || pSuper->Quantity || !pSuper->unknown_bool_60) {
		R->EAX(0);
	} else if(OnHold == pSuper->IsOnHold) {
		R->EAX(0);
	} else {
		if(OnHold || pSuper->Type->ManualControl) {
			int TimeStart = pSuper->RechargeTimer.StartTime;
			if(TimeStart != -1) {
				int TimeLeft = pSuper->RechargeTimer.TimeLeft;
				int TimeElapsed = Unsorted::CurrentFrame - TimeStart;
				if(TimeElapsed >= TimeLeft) {
					TimeLeft = 0;
				} else {
					TimeLeft -= TimeElapsed;
				}
				pSuper->RechargeTimer.TimeLeft = TimeLeft;
				pSuper->RechargeTimer.StartTime = -1;
			}
		} else {
			if(pSuper->RechargeTimer.StartTime == -1) {
				pSuper->RechargeTimer.StartTime = Unsorted::CurrentFrame;
			}
		}
		pSuper->IsOnHold = OnHold;
		if(pSuper->Type->UseChargeDrain) {
			HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(pSuper->Owner);
			if(OnHold) {
				pHouseData->SetFirestormState(0);
				pSuper->ChargeDrainState = ChargeDrainState::None;
			} else {
				pSuper->ChargeDrainState = ChargeDrainState::Charging;
				pSuper->RechargeTimer.Start(pSuper->Type->RechargeTime);
			}
		}
		R->EAX(1);
	}
	return 0x6CB555;
}
