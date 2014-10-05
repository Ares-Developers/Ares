#include "Body.h"

#include "../../Misc/SWTypes.h"
#include "../../Misc/SWTypes/Firewall.h"

#include <HouseClass.h>


//Static init
template<> const DWORD Extension<TActionClass>::Canary = 0x91919191;
Container<TActionExt> TActionExt::ExtMap;

template<> TActionClass* Container<TActionExt>::SavingObject = nullptr;
template<> IStream *Container<TActionExt>::SavingStream = nullptr;

// Enables the Firestorm super weapon for a house.
/*!
	\returns Always True.

	\date 2012-09-24
*/
bool TActionExt::ExtData::ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos)
{
	if(!pHouse->FirestormActive) {
		auto index = pHouse->FindSuperWeaponIndex(SW_Firewall::FirewallType);

		if(index >= 0) {
			pHouse->Fire_SW(index, CellStruct::Empty);
		}
	}
	return true;
}

// Disables the Firestorm super weapon for a house.
/*!
	\returns Always True.

	\date 2012-09-24
*/
bool TActionExt::ExtData::DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos)
{
	if(pHouse->FirestormActive) {
		auto index = pHouse->FindSuperWeaponIndex(SW_Firewall::FirewallType);

		if(index >= 0) {
			pHouse->Fire_SW(index, CellStruct::Empty);
		}
	}
	return true;
}

// Handles the execution of actions.
/*!
	Override any execution of actions here. Set ret to the result of the action.

	\returns True if this action was executed by Ares, false otherwise.

	\date 2012-09-24
*/
bool TActionExt::Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos, bool* ret) {
	auto pExt = ExtMap.Find(pAction);

	switch(pAction->ActionKind) {
	case TriggerAction::PlaySoundEffectRandom:
		// #1004906: function replaced to support more than 100 waypoints
		*ret = pAction->PlayAudioAtRandomWP(pHouse, pObject, pTrigger, pos);
		break;

	case TriggerAction::ActivateFirestorm:
		*ret = pExt->ActivateFirestorm(pAction, pHouse, pObject, pTrigger, pos);
		break;

	case TriggerAction::DeactivateFirestorm:
		*ret = pExt->DeactivateFirestorm(pAction, pHouse, pObject, pTrigger, pos);
		break;

	default:
		UNREFERENCED_PARAMETER(pExt);
		return false;
	}

	return true;
}

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
A_FINE_HOOK(6DD176, TActionClass_CTOR, 5)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

A_FINE_HOOK(6E4761, TActionClass_SDDTOR, 6)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

A_FINE_HOOK_AGAIN(6E3E30, TActionClass_SaveLoad_Prefix, 8)
A_FINE_HOOK(6E3DB0, TActionClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<TActionExt>::PrepareStream(pItem, pStm);

	return 0;
}

A_FINE_HOOK(6E3E29, TActionClass_Load_Suffix, 4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

A_FINE_HOOK(6E3E4A, TActionClass_Save_Suffix, 3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}
#endif
