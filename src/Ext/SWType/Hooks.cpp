#include "Body.h"
#include "../../Misc/SWTypes.h"
#include "../Building/Body.h"
#include "../House/Body.h"

#include <DiscreteSelectionClass.h>
#include <StringTable.h>
#include <VoxClass.h>

DEFINE_HOOK(6CEF84, SuperWeaponTypeClass_GetCursorOverObject, 7)
{
	GET(SuperWeaponTypeClass*, pThis, ECX);

	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);
	auto pType = pData->GetNewSWType();

	if((pThis->Action == Actions::SuperWeaponAllowed) || pType) {
		GET_STACK(CellStruct *, pMapCoords, 0x0C);

		auto Action = Actions::SuperWeaponAllowed;

		// prevent firing into shroud
		if(!pData->SW_FireToShroud) {
			CellClass* pCell = MapClass::Instance->GetCellAt(*pMapCoords);
			CoordStruct Crd = pCell->GetCoords();

			if(MapClass::Instance->IsLocationShrouded(Crd)) {
				Action = Actions::SuperWeaponDisallowed;
			}
		}

		// new SW types have to check whether the coordinates are valid.
		if(Action == Actions::SuperWeaponAllowed) {
			if(pType && !pType->CanFireAt(pData, HouseClass::Player, *pMapCoords, true)) {
				Action = Actions::SuperWeaponDisallowed;
			}
		}

		R->EAX(Action);

		if(Action == Actions::SuperWeaponAllowed) {
			SWTypeExt::CurrentSWType = pThis;
			Actions::Set(&pData->SW_Cursor, pData->SW_FireToShroud);
		} else {
			SWTypeExt::CurrentSWType = nullptr;
			Actions::Set(&pData->SW_NoCursor, pData->SW_FireToShroud);
		}
		return 0x6CEFD9;
	}
	return 0;
}


DEFINE_HOOK(653B3A, RadarClass_GetMouseAction_CustomSWAction, 5)
{
	int idxSWType = Unsorted::CurrentSWType;
	if(idxSWType > -1) {
		REF_STACK(const MouseEvent, EventFlags, 0x58);

		if(EventFlags & (MouseEvent::RightDown | MouseEvent::RightUp)) {
			return 0x653D6F;
		}

		SuperWeaponTypeClass *pThis = SuperWeaponTypeClass::Array->GetItem(idxSWType);
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);
		auto pType = pData->GetNewSWType();

		if((pThis->Action == Actions::SuperWeaponAllowed) || pType) {
			GET_STACK(CellStruct, MapCoords, STACK_OFFS(0x54, 0x3C));

			auto Action = Actions::SuperWeaponAllowed;

			// prevent firing into shroud
			if(!pData->SW_FireToShroud) {
				CellClass* pCell = MapClass::Instance->GetCellAt(MapCoords);
				CoordStruct Crd = pCell->GetCoords();

				if(MapClass::Instance->IsLocationShrouded(Crd)) {
					Action = Actions::SuperWeaponDisallowed;
				}
			}

			// new SW types have to check whether the coordinates are valid.
			if(Action == Actions::SuperWeaponAllowed) {
				if(pType && !pType->CanFireAt(pData, HouseClass::Player, MapCoords, true)) {
					Action = Actions::SuperWeaponDisallowed;
				}
			}

			R->ESI(Action);

			if(Action == Actions::SuperWeaponAllowed) {
				SWTypeExt::CurrentSWType = pThis;
				Actions::Set(&pData->SW_Cursor, pData->SW_FireToShroud);
			} else {
				SWTypeExt::CurrentSWType = nullptr;
				Actions::Set(&pData->SW_NoCursor, pData->SW_FireToShroud);
			}
			return 0x653CA3;
		}
	}
	return 0;
}

DEFINE_HOOK(6AAEDF, SidebarClass_ProcessCameoClick_SuperWeapons, 6) {
	GET(int, idxSW, ESI);
	SuperClass* pSuper = HouseClass::Player->Supers.GetItem(idxSW);

	if(SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type)) {
		// if this SW is only auto-firable, discard any clicks.
		// if AutoFire is off, the sw would not be firable at all,
		// thus we ignore the setting in that case.
		bool manual = !pData->SW_ManualFire && pData->SW_AutoFire;
		bool unstoppable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
			&& pData->SW_Unstoppable;

		// play impatient voice, if this isn't charged yet
		if(!pSuper->CanFire() && !manual) {
			VoxClass::PlayIndex(pData->EVA_Impatient);
			return 0x6AAFB1;
		}

		// prevent firing the SW if the player doesn't have sufficient
		// funds. play an EVA message in that case.
		if(!HouseClass::Player->CanTransactMoney(pData->Money_Amount)) {
			VoxClass::PlayIndex(pData->EVA_InsufficientFunds);
			pData->PrintMessage(pData->Message_InsufficientFunds, HouseClass::Player);
			return 0x6AAFB1;
		}
		
		// disallow manuals and active unstoppables
		if(manual || unstoppable) {
			return 0x6AAFB1;
		}

		return 0x6AAEF7;
	}

	return 0;
}

// play a customizable target selection EVA message
DEFINE_HOOK(6AAF9D, SidebarClass_ProcessCameoClick_SelectTarget, 5)
{
	GET(int, index, ESI);
	if(SuperClass* pSW = HouseClass::Player->Supers.GetItem(index)) {
		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW->Type)) {
			VoxClass::PlayIndex(pData->EVA_SelectTarget);
		}
	}

	return 0x6AB95A;
}

DEFINE_HOOK(6A932B, CameoClass_GetTip_MoneySW, 6) {
	GET(SuperWeaponTypeClass*, pSW, EAX);

	if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW)) {
		if(pData->Money_Amount < 0) {
			wchar_t* pTip = SidebarClass::TooltipBuffer;
			int length = SidebarClass::TooltipLength;

			// account for no-name SWs
			if(*reinterpret_cast<byte*>(0x884B8C) || !wcslen(pSW->UIName)) {
				const wchar_t* pFormat = StringTable::LoadStringA("TXT_MONEY_FORMAT_1");
				swprintf(pTip, length, pFormat, -pData->Money_Amount);
			} else {
				// then, this must be brand SWs
				const wchar_t* pFormat = StringTable::LoadStringA("TXT_MONEY_FORMAT_2");
				swprintf(pTip, length, pFormat, pSW->UIName, -pData->Money_Amount);
			}
			pTip[length - 1] = 0;

			// replace space by new line
			for(int i=wcslen(pTip); i>=0; --i) {
				if(pTip[i] == 0x20) {
					pTip[i] = 0xA;
					break;
				}
			}

			// put it there
			R->EAX(pTip);
			return 0x6A93E5;
		}
	}

	return 0;
}

// 6CEE96, 5
DEFINE_HOOK(6CEE96, SuperWeaponTypeClass_GetTypeIndex, 5)
{
	GET(const char *, TypeStr, EDI);
	auto customType = NewSWType::FindIndex(TypeStr);
	if(customType > SuperWeaponType::Invalid) {
		R->ESI(customType);
		return 0x6CEE9C;
	}
	return 0;
}

// 4AC20C, 7
// translates SW click to type
DEFINE_HOOK(4AC20C, DisplayClass_LMBUp, 7)
{
	auto Action = static_cast<enum class Action>(R->Stack32(0x9C));
	if(Action < Actions::SuperWeaponDisallowed) {
		// get the actual firing SW type instead of just the first type of the
		// requested action. this allows clones to work for legacy SWs (the new
		// ones use SW_*_CURSORs). we have to check that the action matches the
		// action of the found type as the no-cursor represents a different
		// action and we don't want to start a force shield even tough the UI
		// says no.
		auto pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(Unsorted::CurrentSWType);
		if(pSW && (pSW->Action != Action)) {
			pSW = nullptr;
		}

		R->EAX(pSW);
		return pSW ? 0x4AC21C : 0x4AC294;
	}
	else if(Action == Actions::SuperWeaponDisallowed) {
		R->EAX(0);
		return 0x4AC294;
	}

	R->EAX(SWTypeExt::CurrentSWType);
	return 0x4AC21C;
}

// decoupling sw anims from types
DEFINE_HOOK(4463F0, BuildingClass_Place_SuperWeaponAnimsA, 6)
{
	GET(BuildingClass*, pThis, EBP);
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	if(auto pSuper = pExt->GetFirstSuperWeapon()) {
		R->EAX(pSuper);
		return 0x44643E;
	}

	return 0x446580;
}

DEFINE_HOOK(44656D, BuildingClass_Place_SuperWeaponAnimsB, 6)
{
	return 0x446580;
}

DEFINE_HOOK(450F9E, BuildingClass_ProcessAnims_SuperWeaponsA, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	if(auto pSuper = pExt->GetFirstSuperWeapon()) {
		auto miss = pThis->GetCurrentMission();
		if(miss != Mission::Construction && miss != Mission::Selling
			&& pThis->Type->ChargedAnimTime <= 990.0)
		{
			R->EDI(pThis->Type);
			R->EAX(pSuper);
			return 0x451030;
		}
	}

	return 0x451145;
}

DEFINE_HOOK(451132, BuildingClass_ProcessAnims_SuperWeaponsB, 6)
{
	return 0x451145;
}

// EVA_Detected
DEFINE_HOOK(4468F4, BuildingClass_Place_AnnounceSW, 6) {
	GET(BuildingClass*, pThis, EBP);
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	if(auto pSuper = pExt->GetFirstSuperWeapon()) {
		auto pData = SWTypeExt::ExtMap.Find(pSuper->Type);

		pData->PrintMessage(pData->Message_Detected, pThis->Owner);

		if(pData->EVA_Detected == -1 && pData->IsOriginalType() && !pData->IsTypeRedirected()) {
			R->EAX(pSuper->Type->Type);
			return 0x446943;
		}

		VoxClass::PlayIndex(pData->EVA_Detected);
	}

	return 0x44699A;
}

// EVA_Ready
// 6CBDD7, 6
DEFINE_HOOK(6CBDD7, SuperClass_AnnounceReady, 6)
{
	GET(SuperWeaponTypeClass*, pThis, EAX);
	auto pData = SWTypeExt::ExtMap.Find(pThis);

	pData->PrintMessage(pData->Message_Ready, HouseClass::Player);

	if(pData->EVA_Ready != -1 || !pData->IsOriginalType() || pData->IsTypeRedirected()) {
		VoxClass::PlayIndex(pData->EVA_Ready);
		return 0x6CBE68;
	}
	return 0;
}

// 6CC0EA, 9
DEFINE_HOOK(6CC0EA, SuperClass_AnnounceQuantity, 9)
{
	GET(SuperClass*, pThis, ESI);
	auto pData = SWTypeExt::ExtMap.Find(pThis->Type);

	pData->PrintMessage(pData->Message_Ready, HouseClass::Player);

	if(pData->EVA_Ready != -1 || !pData->IsOriginalType() || pData->IsTypeRedirected()) {
		VoxClass::PlayIndex(pData->EVA_Ready);
		return 0x6CC17E;
	}
	return 0;
}

// AI SW targeting submarines
DEFINE_HOOK(50CFAA, HouseClass_PickOffensiveSWTarget, 0)
{
	// reset weight
	R->ESI(0);

	// mark as ineligible
	R->Stack8(0x13, 0);

	return 0x50CFC9;
}

// ARGH!
DEFINE_HOOK(6CC390, SuperClass_Launch, 6)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	Debug::Log("[LAUNCH] %s\n", pSuper->Type->ID);

	auto const handled = SWTypeExt::Activate(pSuper, *pCell, isPlayer);

	return handled ? 0x6CDE40 : 0;
}

DEFINE_HOOK(457630, BuildingClass_SWAvailable, 9) {
	GET(BuildingClass*, pThis, ECX);
	auto pExt = BuildingExt::ExtMap.Find(pThis);
	R->EAX(pExt->GetSuperWeaponIndex(0));
	return 0x457688;
}

DEFINE_HOOK(457690, BuildingClass_SW2Available, 9) {
	GET(BuildingClass*, pThis, ECX);
	auto pExt = BuildingExt::ExtMap.Find(pThis);
	R->EAX(pExt->GetSuperWeaponIndex(1));
	return 0x4576E8;
}

DEFINE_HOOK(43BE50, BuildingClass_DTOR_HasAnySW, 6) {
	GET(BuildingClass*, pThis, ESI);
	auto pExt = BuildingExt::ExtMap.Find(pThis);
	return pExt->HasSuperWeapon() ? 0x43BEEAu : 0x43BEF5u;
}

DEFINE_HOOK(449716, BuildingClass_Mi_Guard_HasFirstSW, 6) {
	GET(BuildingClass*, pThis, ESI);
	return pThis->FirstActiveSWIdx() != -1 ? 0x4497AFu : 0x449762u;
}

DEFINE_HOOK(4FAE72, HouseClass_SWFire_PreDependent, 6)
{
	GET(HouseClass*, pThis, EBX);

	// find the predependent SW. decouple this from the chronosphere.
	// don't use a fixed SW type but the very one acutually fired last.
	SuperClass* pSource = nullptr;
	if(auto pExt = HouseExt::ExtMap.Find(pThis)) {
		pSource = pThis->Supers.GetItemOrDefault(pExt->SWLastIndex);
	}

	R->ESI(pSource);

	return 0x4FAE7B;
}

DEFINE_HOOK(6CC2B0, SuperClass_NameReadiness, 5) {
	GET(SuperClass*, pThis, ECX);
	auto pData = SWTypeExt::ExtMap.Find(pThis->Type);

	// complete rewrite of this method.

	auto text = &pData->Text_Preparing;
	if(pThis->IsOnHold) {
		// on hold
		text = &pData->Text_Hold;
	} else {
		if(pThis->Type->UseChargeDrain) {
			switch(pThis->ChargeDrainState) {
			case ChargeDrainState::Charging:
				// still charging
				text = &pData->Text_Charging;
				break;
			case ChargeDrainState::Ready:
				// ready
				text = &pData->Text_Ready;
				break;
			case ChargeDrainState::Draining:
				// currently active
				text = &pData->Text_Active;
				break;
			}

		} else {
			// ready
			if(pThis->IsCharged) {
				text = &pData->Text_Ready;
			}
		}
	}

	R->EAX(text->empty() ? nullptr : text->Text);
	return 0x6CC352;
}

// #896002: darken SW cameo if player can't afford it
DEFINE_HOOK(6A99B7, TabCameoListClass_Draw_SuperDarken, 5)
{
	GET(int, idxSW, EDI);

	auto pSW = HouseClass::Player->Supers.GetItem(idxSW);
	auto pExt = SWTypeExt::ExtMap.Find(pSW->Type);

	bool darken = false;
	if(pSW->IsCharged && !pSW->Owner->CanTransactMoney(pExt->Money_Amount)) {
		darken = true;
	}

	R->BL(darken);
	return 0;
}

DEFINE_HOOK(4F9004, HouseClass_Update_TrySWFire, 7) {
	GET(HouseClass*, pThis, ESI);
	bool isHuman = R->AL() != 0;

	if(isHuman) {
		// update the SWs for human players to support auto firing.
		pThis->AI_TryFireSW();
	} else if(!pThis->Type->MultiplayPassive) {
		return 0x4F9015;
	}

	return 0x4F9038;
}

// #1369308: if still charged it hasn't fired.
// more efficient place would be 4FAEC9, but this is global
DEFINE_HOOK(4FAF2A, HouseClass_SWDefendAgainst_Aborted, 8)
{
	GET(SuperClass*, pSW, EAX);
	return (pSW && !pSW->IsCharged) ? 0x4FAF32 : 0x4FB0CF;
}

DEFINE_HOOK(6CBF5B, SuperClass_GetCameoChargeState_ChargeDrainRatio, 9) {
	GET_STACK(int, rechargeTime1, 0x10);
	GET_STACK(int, rechargeTime2, 0x14);
	GET_STACK(int, timeLeft, 0xC);
	
	GET(SuperWeaponTypeClass*, pType, EBX);
	if(SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pType)) {

		// use per-SW charge-to-drain ratio.
		double percentage = 0.0;
		double ratio = pData->GetChargeToDrainRatio();
		if(std::abs(rechargeTime2 * ratio) > 0.001) {
			percentage = 1.0 - (rechargeTime1 * ratio - timeLeft) / (rechargeTime2 * ratio);
		}

		// up to 55 steps
		int charge = Game::F2I(percentage * 54.0);
		R->EAX(charge);
		return 0x6CC053;
	}

	return 0;
}

DEFINE_HOOK(6CC053, SuperClass_GetCameoChargeState_FixFullyCharged, 5) {
	GET(int, charge, EAX);

	// some smartass capped this at 53, causing the last
	// wedge of darken.shp never to disappear.
	R->EAX(std::min(charge, 54));
	return 0x6CC066;
}

// a ChargeDrain SW expired - fire it to trigger status update
DEFINE_HOOK(6CBD86, SuperClass_Progress_Charged, 7)
{
	GET(SuperClass* const, pThis, ESI);
	SWTypeExt::Deactivate(pThis, CellStruct::Empty, true);
	return 0;
}

// SW was lost (source went away)
DEFINE_HOOK(6CB7B0, SuperClass_Lose, 6)
{
	GET(SuperClass* const, pThis, ECX);
	auto ret = false;

	if(pThis->Granted) {
		pThis->IsCharged = false;
		pThis->Granted = false;

		SuperClass::ShowTimers->Remove(pThis);
		SWTypeExt::Deactivate(pThis, CellStruct::Empty, false);

		ret = true;
	}

	R->EAX(ret);
	return 0x6CB810;
}

// activate or deactivate the SW
DEFINE_HOOK(6CB920, SuperClass_ClickFire, 5)
{
	GET(SuperClass* const, pThis, ECX);
	GET_STACK(bool const, isPlayer, 0x4);
	GET_STACK(CellStruct const* const, pCell, 0x8);

	retfunc<bool> ret(R, 0x6CBC9C);

	auto const pType = pThis->Type;
	auto const pExt = SWTypeExt::ExtMap.Find(pType);

	auto const pOwner = pThis->Owner;

	if(pType->UseChargeDrain) {

		// AI get non-draining SWs
		if(!pOwner->ControlledByHuman()) {
			auto const pOwnerExt = HouseExt::ExtMap.Find(pOwner);
			if(!pOwnerExt->FirewallActive) {
				pThis->Launch(*pCell, isPlayer);
			} else {
				SWTypeExt::Deactivate(pThis, *pCell, isPlayer);
			}

		} else {
			if(pThis->ChargeDrainState == ChargeDrainState::Draining) {
				// deactivate for human players
				pThis->ChargeDrainState = ChargeDrainState::Ready;
				auto const left = pThis->RechargeTimer.GetTimeLeft();

				auto const duration = Game::F2I(pThis->GetRechargeTime()
					- (left / pExt->GetChargeToDrainRatio()));
				pThis->RechargeTimer.Start(duration);

				SWTypeExt::Deactivate(pThis, *pCell, isPlayer);

			} else if(pThis->ChargeDrainState == ChargeDrainState::Ready) {
				// activate for human players
				pThis->ChargeDrainState = ChargeDrainState::Draining;
				auto const left = pThis->RechargeTimer.GetTimeLeft();

				auto const duration = Game::F2I(
					(pThis->GetRechargeTime() - left)
					* pExt->GetChargeToDrainRatio());
				pThis->RechargeTimer.Start(duration);

				pThis->Launch(*pCell, isPlayer);
			}
		}

		return ret(false);
	}

	if((pThis->RechargeTimer.StartTime == -1
		|| !pThis->Granted
		|| !pThis->IsCharged)
		&& !pType->PostClick)
	{
		return ret(false);
	}

	// auto-abort if no money
	if(!pOwner->CanTransactMoney(pExt->Money_Amount)) {
		if(pOwner->IsPlayer()) {
			VoxClass::PlayIndex(pExt->EVA_InsufficientFunds);
			pExt->PrintMessage(pExt->Message_InsufficientFunds, pOwner);
		}
		return ret(false);
	}

	// can this super weapon fire now?
	if(auto const pNewType = pExt->GetNewSWType()) {
		if(pNewType->AbortFire(pThis, isPlayer)) {
			return ret(false);
		}
	}

	pThis->Launch(*pCell, isPlayer);

	// the others will be reset after the PostClick SW fired
	if(!pType->PostClick && !pType->PreClick) {
		pThis->IsCharged = false;
	}

	if(pThis->OneTime) {
		// remove this SW
		pThis->OneTime = false;
		auto const lost = pThis->Lose();
		return ret(lost);
	} else if(pType->ManualControl) {
		// set recharge timer, then pause
		auto time = pThis->GetRechargeTime();
		pThis->CameoChargeState = -1;
		pThis->RechargeTimer.Start(time);
		pThis->RechargeTimer.Pause();
	} else {
		if(!pType->PreClick && !pType->PostClick) {
			pThis->StopPreclickAnim(isPlayer);
		}
	}

	return ret(false);
}

// rewriting OnHold to support ChargeDrain
DEFINE_HOOK(6CB4D0, SuperClass_SetOnHold, 6)
{
	GET(SuperClass* const, pThis, ECX);
	GET_STACK(bool const, onHold, 0x4);

	auto ret = false;

	if(pThis->Granted
		&& !pThis->OneTime
		&& pThis->CanHold
		&& onHold != pThis->IsOnHold)
	{
		if(onHold || pThis->Type->ManualControl) {
			pThis->RechargeTimer.Pause();
		} else {
			pThis->RechargeTimer.Resume();
		}

		pThis->IsOnHold = onHold;

		if(pThis->Type->UseChargeDrain) {
			if(onHold) {
				SWTypeExt::Deactivate(pThis, CellStruct::Empty, false);
				pThis->ChargeDrainState = ChargeDrainState::None;
			} else {
				pThis->ChargeDrainState = ChargeDrainState::Charging;
				pThis->RechargeTimer.Start(pThis->Type->RechargeTime);
			}
		}

		ret = true;
	}

	R->EAX(ret);
	return 0x6CB555;
}

DEFINE_HOOK(6CBD6B, SuperClass_Update_DrainMoney, 8) {
	// draining weapon active. take or give money. stop, 
	// if player has insufficient funds.
	GET(SuperClass*, pSuper, ESI);
	GET(int, timeLeft, EAX);

	if(timeLeft > 0 && pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining) {
		if(SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type)) {
			int money = pData->Money_DrainAmount;
			if(money != 0 && pData->Money_DrainDelay > 0) {
				if(!(timeLeft % pData->Money_DrainDelay)) {
					auto pOwner = pSuper->Owner;

					// only abort if SW drains money and there is none
					if(!pOwner->CanTransactMoney(money)) {
						if(pOwner->ControlledByHuman()) {
							VoxClass::PlayIndex(pData->EVA_InsufficientFunds);
							pData->PrintMessage(pData->Message_InsufficientFunds, HouseClass::Player);
						}
						return 0x6CBD73;
					}

					// apply drain money
					pOwner->TransactMoney(money);
				}
			}
		}
	}

	return (timeLeft ? 0x6CBE7C : 0x6CBD73);
}

// clear the chrono placement animation if not ChronoWarp
DEFINE_HOOK(6CBCDE, SuperClass_Update_Animation, 5) {
	if(auto const pType = SuperWeaponTypeClass::Array->GetItemOrDefault(Unsorted::CurrentSWType)) {
		if(pType->Type == SuperWeaponType::ChronoWarp) {
			return 0x6CBCFE;
		}
	}
	return 0x6CBCE3;
}

// used only to find the nuke for ICBM crates. only supports nukes fully.
DEFINE_HOOK(6CEEB0, SuperWeaponTypeClass_FindFirstOfAction, 8) {
	GET(Action, action, ECX);

	SuperWeaponTypeClass* pFound = nullptr;

	// this implementation is as stupid as short sighted, but it should work
	// for the moment. as there are no actions any more, this has to be
	// reworked if powerups are expanded. for now, it only has to find a nuke.
	for(auto pType : *SuperWeaponTypeClass::Array) {
		if(pType->Action == action) {
			pFound = pType;
			break;
		} else {
			auto pExt = SWTypeExt::ExtMap.Find(pType);
			if(auto pNewSWType = pExt->GetNewSWType()) {
				if(pNewSWType->HandlesType(SuperWeaponType::Nuke)) {
					pFound = pType;
					break;
				}
			}
		}
	}

	// put a hint into the debug log to explain why we will crash now.
	if(!pFound) {
		Debug::FatalErrorAndExit("Failed finding an Action=Nuke or Type=MultiMissile super weapon to be granted by ICBM crate.");
	}

	R->EAX(pFound);
	return 0x6CEEE5;
}
