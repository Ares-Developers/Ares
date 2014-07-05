#include "Body.h"
#include "../House/Body.h"
#include "../Techno/Body.h"
#include <MouseClass.h>
#include <SuperClass.h>

#include <algorithm>
#include <functional>

// cache all super weapon statuses
struct SWStatus {
	bool Available;
	bool PowerSourced;
	bool Charging;
};

// This function controls the availability of super weapons. If a you want to
// add to or change the way the game thinks a building provides a super weapon,
// change the lambda UpdateStatus. Available means this super weapon exists at
// all. Setting it to false removes the super weapon. PowerSourced controls
// whether the super weapon charges or can be used.
std::vector<SWStatus> GetSuperWeaponStatuses(HouseClass* pHouse) {
	std::vector<SWStatus> Statuses(pHouse->Supers.Count, {false, false, false});

	// look at every sane building this player owns, if it is not defeated already.
	if(!pHouse->Defeated) {
		for(auto pBld : pHouse->Buildings) {
			if(pBld->IsAlive && !pBld->InLimbo) {
				TechnoExt::ExtData *pExt = TechnoExt::ExtMap.Find(pBld);

				// the super weapon status update lambda.
				auto UpdateStatus = [&](int idxSW) {
					if(idxSW > -1) {
						auto& status = Statuses[idxSW];
						status.Available = true;
						if(!status.PowerSourced || !status.Charging) {
							bool validBuilding = pBld->HasPower
								&& pExt->IsOperated()
								&& !pBld->IsUnderEMP();

							if(!status.PowerSourced) {
								status.PowerSourced = validBuilding;
							}

							if(!status.Charging) {
								status.Charging = validBuilding
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
				for(auto pUpgrade : pBld->Upgrades) {
					if(pUpgrade) {
						UpdateStatus(pUpgrade->SuperWeapon);
						UpdateStatus(pUpgrade->SuperWeapon2);
					}
				}

				// look for the main building.
				UpdateStatus(pBld->FirstActiveSWIdx());
				UpdateStatus(pBld->SecondActiveSWIdx());
			}
		}

		// kill off super weapons that are disallowed and
		// factor in the player's power status
		auto hasPower = pHouse->HasFullPower();

		for(auto pSuper : pHouse->Supers) {
			auto index = pSuper->Type->GetArrayIndex();
			auto& status = Statuses[index];

			// turn off super weapons that are disallowed.
			if(!Unsorted::SWAllowed) {
				if(pSuper->Type->DisableableFromShell) {
					status.Available = false;
				}
			}

			// if the house is generally on low power,
			// powered super weapons aren't powered
			if(pSuper->IsPowered()) {
				status.PowerSourced &= hasPower;
			}
		}
	}

	return Statuses;
}

DEFINE_HOOK(50AF10, HouseClass_UpdateSuperWeaponsOwned, 5)
{
	GET(HouseClass *, pThis, ECX);

	auto Statuses = GetSuperWeaponStatuses(pThis);

	// now update every super weapon that is valid.
	// if this weapon has not been granted there's no need to update
	for(auto pSuper : pThis->Supers) {
		if(pSuper->Granted) {
			auto pType = pSuper->Type;
			auto index = pType->GetArrayIndex();
			auto& status = Statuses[index];

			// is this a super weapon to be updated?
			// sw is bound to a building and no single-shot => create goody otherwise
			bool isCreateGoody = (!pSuper->CanHold || pSuper->OneTime);
			if(!isCreateGoody || pThis->Defeated) {

				// shut down or power up super weapon and decide whether
				// a sidebar tab update is needed.
				bool update = false;
				if(!status.Available || pThis->Defeated) {
					update = (pSuper->Lose() && HouseClass::Player);
				} else if(status.Charging && !pSuper->IsPowered()) {
					update = pSuper->IsOnHold && pSuper->SetOnHold(false);
				} else if(!status.Charging && !pSuper->IsPowered()) {
					update = !pSuper->IsOnHold && pSuper->SetOnHold(true);
				} else if(!status.PowerSourced) {
					update = (pSuper->IsPowered() && pSuper->SetOnHold(true));
				} else {
					update = (status.PowerSourced && pSuper->SetOnHold(false));
				}

				// update only if needed.
				if(update) {
					// only the human player can see the sidebar.
					if(pThis->IsPlayer()) {
						if(Unsorted::CurrentSWType == index) {
							Unsorted::CurrentSWType = -1;
						}
						int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, index, 0);
						MouseClass::Instance->RepaintSidebar(idxTab);
					}
					pThis->RecheckTechTree = true;
				}
			}
		}
	}

	return 0x50B1CA;
}

DEFINE_HOOK(50B1D0, HouseClass_UpdateSuperWeaponsUnavailable, 6)
{
	GET(HouseClass*, pThis, ECX);

	if(!pThis->Defeated) {
		auto Statuses = GetSuperWeaponStatuses(pThis);

		// update all super weapons not repeatedly available
		for(auto pSuper : pThis->Supers) {
			if(!pSuper->Granted || pSuper->OneTime) {
				auto index = pSuper->Type->GetArrayIndex();
				auto& status = Statuses[index];

				if(status.Available) {
					pSuper->Grant(false, pThis->IsPlayer(), !status.PowerSourced);

					if(pThis->IsPlayer()) {
						// hide the cameo (only if this is an auto-firing SW)
						auto pData = SWTypeExt::ExtMap.Find(pSuper->Type);
						if(pData->SW_ShowCameo || !pData->SW_AutoFire) {
							MouseClass::Instance->AddCameo(0x1F /* Special */, index);
							int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, index, 0);
							MouseClass::Instance->RepaintSidebar(idxTab);
						}
					}
				}
			}
		}
	}

	return 0x50B36E;
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
	if(!pSuper->Granted || pSuper->OneTime || !pSuper->CanHold) {
		R->EAX(0);
	} else if(OnHold == pSuper->IsOnHold) {
		R->EAX(0);
	} else {
		if(OnHold || pSuper->Type->ManualControl) {
			pSuper->RechargeTimer.Pause();
		} else {
			pSuper->RechargeTimer.Resume();
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
