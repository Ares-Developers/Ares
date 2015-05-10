#include "Body.h"
#include "../BuildingType/Body.h"
#include "../HouseType/Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"
#include "../../Misc/Network.h"

#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <InfantryClass.h>
#include <CellClass.h>
#include <HouseClass.h>
#include <VoxClass.h>
#include <MessageListClass.h>

#include <cmath>

/* #754 - evict Hospital/Armory contents */

// one more hook at 448277

// one more hook at 447113

DEFINE_HOOK(44D8A1, BuildingClass_UnloadPassengers_Unload, 6)
{
	GET(BuildingClass *, B, EBP);

	BuildingExt::KickOutHospitalArmory(B);
	return 0;
}

// for yet unestablished reasons a unit might not be present.
// maybe something triggered the KickOutHospitalArmory
DEFINE_HOOK(44BB1B, BuildingClass_Mi_Repair_Promote, 6)
{
	//GET(BuildingClass*, pThis, EBP);
	GET(TechnoClass*, pTrainee, EAX);

	return pTrainee ? 0 : 0x44BB3C;
}

/* 	#218 - specific occupiers -- see Hooks.Trenches.cpp */

// EMP'd power plants don't produce power
DEFINE_HOOK(44E855, BuildingClass_PowerProduced_EMP, 6) {
	GET(BuildingClass*, pBld, ESI);
	return ((pBld->EMPLockRemaining > 0) ? 0x44E873 : 0);
}

// VeteranBuildings
DEFINE_HOOK(43BA48, BuildingClass_CTOR_VeteranBuildings, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(BuildingTypeClass*, pType, EAX);

	if(auto pOwner = pThis->Owner) {
		if(auto pExt = HouseTypeExt::ExtMap.Find(pOwner->Type)) {
			if(pExt->VeteranBuildings.Contains(pType)) {
				pThis->Veterancy.SetVeteran(true);
			}
		}
	}

	return 0;
}

// restore pip count for tiberium storage (building and house)
DEFINE_HOOK(44D755, BuildingClass_GetPipFillLevel_Tiberium, 6)
{
	GET(BuildingClass*, pThis, ECX);
	GET(BuildingTypeClass*, pType, ESI);

	double amount = 0.0;
	if(pType->Storage > 0) {
		amount = pThis->Tiberium.GetTotalAmount() / pType->Storage;
	} else {
		amount = pThis->Owner->GetStoragePercentage();
	}

	int ret = Game::F2I(pType->GetPipMax() * amount);
	R->EAX(ret);
	return 0x44D750;
}

// the game specifically hides tiberium building pips. allow them, but
// take care they don't show up for the original game
DEFINE_HOOK(709B4E, TechnoClass_DrawPipscale_SkipSkipTiberium, 6)
{
	GET(TechnoClass*, pThis, EBP);

	bool showTiberium = true;
	if(auto pType = specific_cast<BuildingTypeClass*>(pThis->GetTechnoType())) {
		if((pType->Refinery || pType->ResourceDestination) && pType->Storage > 0) {
			// show only if this refinery uses storage. otherwise, the original
			// refineries would show an unused tiberium pip scale
			auto pExt = TechnoTypeExt::ExtMap.Find(pType);
			showTiberium = pExt->Refinery_UseStorage;
		}
	}

	return showTiberium ? 0x709B6E : 0x70A980;
}

// also consider NeedsEngineer when activating animations
// if the status changes, animations might start to play that aren't
// supposed to play because the building requires an Engineer which
// didn't capture the building yet.
DEFINE_HOOK(4467D6, BuildingClass_Place_NeedsEngineer, 6)
{
	GET(BuildingClass*, pThis, EBP);
	R->AL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x4467DC;
}

DEFINE_HOOK(454BF7, BuildingClass_UpdatePowered_NeedsEngineer, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->CL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x454BFD;
}

DEFINE_HOOK(451A54, BuildingClass_PlayAnim_NeedsEngineer, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->CL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x451A5A;
}

// infantry exiting hospital get their focus reset, but not for armory
DEFINE_HOOK(444D26, BuildingClass_KickOutUnit_ArmoryExitBug, 6)
{
	GET(BuildingTypeClass*, pType, EDX);
	R->AL(pType->Hospital || pType->Armory);
	return 0x444D2C;
}

// do not crash if the EMP cannon primary has no Report sound
DEFINE_HOOK(44D4CA, BuildingClass_Mi_Missile_NoReport, 9)
{
	GET(TechnoTypeClass*, pType, EAX);
	GET(WeaponTypeClass*, pWeapon, EBP);

	bool play = !pType->IsGattling && pWeapon->Report.Count;
	return play ? 0x44D4D4 : 0x44D51F;
}

DEFINE_HOOK(44840B, BuildingClass_ChangeOwnership_Tech, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(HouseClass*, pNewOwner, EBX);

	if(pThis->Owner != pNewOwner) {
		const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

		auto PrintMessage = [](const CSFText& text) {
			if(!text.empty()) {
				auto color = HouseClass::Player->ColorSchemeIndex;
				MessageListClass::Instance->PrintMessage(text, RulesClass::Instance->MessageDelay, color);
			}
		};

		if(pThis->Owner->ControlledByPlayer()) {
			VoxClass::PlayIndex(pExt->LostEvaEvent);
			PrintMessage(pExt->MessageLost);
		}
		if(pNewOwner->ControlledByPlayer()) {
			VoxClass::PlayIndex(pThis->Type->CaptureEvaEvent);
			PrintMessage(pExt->MessageCapture);
		}
	}

	return 0x44848F;
}

// support oil derrick logic on building upgrades
DEFINE_HOOK(4409F4, BuildingClass_Put_ProduceCash, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(BuildingClass*, pToUpgrade, EDI);

	auto pExt = BuildingExt::ExtMap.Find(pToUpgrade);

	if(auto delay = pThis->Type->ProduceCashDelay) {
		pExt->CashUpgradeTimers[pToUpgrade->UpgradeLevel - 1].Start(delay);
	}

	return 0;
}

DEFINE_HOOK(43FD2C, BuildingClass_Update_ProduceCash, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	auto Process = [](BuildingClass* pBld, BuildingTypeClass* pType, TimerStruct& timer) {
		if(timer.GetTimeLeft() == 1) {
			timer.Start(pType->ProduceCashDelay);

			if(!pBld->Owner->Type->MultiplayPassive && pBld->IsPowerOnline()) {
				pBld->Owner->TransactMoney(pType->ProduceCashAmount);
			}
		}
	};

	Process(pThis, pThis->Type, pThis->CashProductionTimer);

	for(size_t i = 0; i < 3; ++i) {
		if(const auto& pUpgrade = pThis->Upgrades[i]) {
			Process(pThis, pUpgrade, pExt->CashUpgradeTimers[i]);
		}
	}

	return 0x43FDD6;
}

DEFINE_HOOK(4482BD, BuildingClass_ChangeOwnership_ProduceCash, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(HouseClass*, pNewOwner, EBX);
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	auto Process = [](HouseClass* pOwner, BuildingTypeClass* pType, TimerStruct& timer) {
		if(pType->ProduceCashStartup) {
			pOwner->TransactMoney(pType->ProduceCashStartup);
			timer.Start(pType->ProduceCashDelay);
		}
	};

	Process(pNewOwner, pThis->Type, pThis->CashProductionTimer);

	for(size_t i = 0; i < 3; ++i) {
		if(const auto& pUpgrade = pThis->Upgrades[i]) {
			Process(pNewOwner, pUpgrade, pExt->CashUpgradeTimers[i]);
		}
	}

	return 0x4482FC;
}

// make temporal weapons play nice with power toggle.
// previously, power state was set to true unconditionally.
DEFINE_HOOK(452287, BuildingClass_GoOnline_TogglePower, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pExt = BuildingExt::ExtMap.Find(pThis);
	pExt->TogglePower_HasPower = true;
	return 0;
}

DEFINE_HOOK(452393, BuildingClass_GoOffline_TogglePower, 7)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pExt = BuildingExt::ExtMap.Find(pThis);
	pExt->TogglePower_HasPower = false;
	return 0;
}

DEFINE_HOOK(452210, BuildingClass_Enable_TogglePower, 7)
{
	GET(BuildingClass* const, pThis, ECX);
	auto const pExt = BuildingExt::ExtMap.Find(pThis);
	pThis->HasPower = pExt->TogglePower_HasPower;
	return 0x452217;
}

// replaces the UnitReload handling and makes each docker independent of all
// others. this means planes don't have to wait one more ReloadDelay because
// the first docker triggered repair mission while the other dockers arrive
// too late and need to be put to sleep first.
DEFINE_HOOK(44C844, BuildingClass_MissionRepair_Reload, 6)
{
	GET(BuildingClass* const, pThis, EBP);
	auto const pExt = BuildingExt::ExtMap.Find(pThis);

	// ensure there are enough slots
	pExt->DockReloadTimers.Reserve(pThis->RadioLinks.Capacity);

	// update all dockers, check if there's
	// at least one needing more attention
	bool keep_reloading = false;
	for(auto i = 0; i < pThis->RadioLinks.Capacity; ++i) {
		if(auto const pLink = pThis->GetNthLink(i)) {

			auto const SendCommand = [=](RadioCommand command) {
				auto const response = pThis->SendCommand(command, pLink);
				return response == RadioCommand::AnswerPositive;
			};

			// check if reloaded and repaired already
			auto const pLinkType = pLink->GetTechnoType();
			auto done = SendCommand(RadioCommand::QueryReadiness)
				&& pLink->Health == pLinkType->Strength;

			if(!done) {
				// check if docked
				auto const miss = pLink->GetCurrentMission();
				if(miss == Mission::Enter 
					|| !SendCommand(RadioCommand::QueryMoving))
				{
					continue;
				}

				keep_reloading = true;

				// make the unit sleep first
				if(miss != Mission::Sleep) {
					pLink->QueueMission(Mission::Sleep, false);
					continue;
				}

				// check whether the timer completed
				auto const last_timer = pExt->DockReloadTimers[i];
				if(last_timer > Unsorted::CurrentFrame) {
					continue;
				}

				// set the next frame
				auto const pLinkExt = TechnoTypeExt::ExtMap.Find(pLinkType);
				auto const defaultRate = RulesClass::Instance->ReloadRate;
				auto const rate = pLinkExt->ReloadRate.Get(defaultRate);
				auto const frames = static_cast<int>(rate * 900);
				pExt->DockReloadTimers[i] = Unsorted::CurrentFrame + frames;

				// only reload if the timer was not outdated
				if(last_timer != Unsorted::CurrentFrame) {
					continue;
				}

				// reload and repair, return true if both failed
				done = !SendCommand(RadioCommand::RequestReload)
					&& !SendCommand(RadioCommand::RequestRepair);
			}

			if(done) {
				pLink->vt_entry_484(0, 1);
				pLink->ForceMission(Mission::Guard);
				pLink->ProceedToNextPlanningWaypoint();

				pExt->DockReloadTimers[i] = -1;
			}
		}
	}

	if(keep_reloading) {
		// update each frame
		R->EAX(1);
	} else {
		pThis->QueueMission(Mission::Guard, false);
		R->EAX(3);
	}

	return 0x44C968;
}
