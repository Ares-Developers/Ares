#include "Body.h"

#include "../../Ext/House/Body.h"

#include "../../Misc/SWTypes.h"
#include "../../Misc/SWTypes/Firewall.h"

#include "../../Misc/SavegameDef.h"

//Static init
template<> const DWORD Extension<TActionClass>::Canary = 0x91919191;
TActionExt::ExtContainer TActionExt::ExtMap;

// Enables the Firestorm super weapon for a house.
/*!
	\returns Always true.

	\date 2012-09-24
*/
bool TActionExt::ExtData::ActivateFirestorm(
	TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	auto const pExt = HouseExt::ExtMap.Find(pHouse);

	if(!pExt->FirewallActive) {
		auto index = pHouse->FindSuperWeaponIndex(SW_Firewall::FirewallType);

		if(index >= 0) {
			pHouse->Fire_SW(index, CellStruct::Empty);
		}
	}
	return true;
}

// Disables the Firestorm super weapon for a house.
/*!
	\returns Always true.

	\date 2012-09-24
*/
bool TActionExt::ExtData::DeactivateFirestorm(
	TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location)
{
	auto const pExt = HouseExt::ExtMap.Find(pHouse);

	if(pExt->FirewallActive) {
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
bool TActionExt::Execute(
	TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location, bool* ret)
{
	auto pExt = ExtMap.Find(pAction);

	switch(pAction->ActionKind) {
	case TriggerAction::PlaySoundEffectRandom:
		// #1004906: function replaced to support more than 100 waypoints
		*ret = pAction->PlayAudioAtRandomWP(pHouse, pObject, pTrigger, location);
		break;

	case TriggerAction::ActivateFirestorm:
		*ret = pExt->ActivateFirestorm(pAction, pHouse, pObject, pTrigger, location);
		break;

	case TriggerAction::DeactivateFirestorm:
		*ret = pExt->DeactivateFirestorm(pAction, pHouse, pObject, pTrigger, location);
		break;

	default:
		UNREFERENCED_PARAMETER(pExt);
		return false;
	}

	return true;
}

// =============================
// load / save

template <typename T>
void TActionExt::ExtData::Serialize(T& Stm) {
	//Stm;
}

void TActionExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<TActionClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TActionExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<TActionClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

TActionExt::ExtContainer::ExtContainer() : Container("TActionClass") {
}

TActionExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(6DD176, TActionClass_CTOR, 5)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6E4761, TActionClass_SDDTOR, 6)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(6E3E30, TActionClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(6E3DB0, TActionClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TActionExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(6E3E29, TActionClass_Load_Suffix, 4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(6E3E4A, TActionClass_Save_Suffix, 3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}
#endif
