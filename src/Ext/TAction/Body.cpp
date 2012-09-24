#include "Body.h"

#include "../../Misc/SWTypes.h"

//#include <HouseClass.h>


//Static init
template<> const DWORD Extension<TActionClass>::Canary = 0x61616161;
Container<TActionExt> TActionExt::ExtMap;

template<> TActionExt::TT *Container<TActionExt>::SavingObject = NULL;
template<> IStream *Container<TActionExt>::SavingStream = NULL;

void TActionExt::ExtData::Initialize(TActionClass *pThis)
{

}

// Enables the Firestorm super weapon for a house.
/*!
	\returns Always True.

	\date 2012-09-24
*/
bool TActionExt::ExtData::ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos)
{
	if(!pHouse->FirestormActive && pHouse->Supers.Count) {
		int firestorm = NewSWType::FindIndex("Firestorm");
		for(int i=0; i<pHouse->Supers.Count; ++i) {
			if(pHouse->Supers.GetItem(i)->Type->Type == firestorm) {
				CellStruct empty;
				pHouse->Fire_SW(i, &empty);
				break;
			}
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
	if(pHouse->FirestormActive && pHouse->Supers.Count) {
		int firestorm = NewSWType::FindIndex("Firestorm");
		for(int i=0; i<pHouse->Supers.Count; ++i) {
			if(pHouse->Supers.GetItem(i)->Type->Type == firestorm) {
				CellStruct empty;
				pHouse->Fire_SW(i, &empty);
				break;
			}
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
	case TriggerAction::ActivateFirestorm:
		*ret = pExt->ActivateFirestorm(pAction, pHouse, pObject, pTrigger, pos);
		break;

	case TriggerAction::DeactivateFirestorm:
		*ret = pExt->DeactivateFirestorm(pAction, pHouse, pObject, pTrigger, pos);
		break;

	default:
		return false;
	}

	return true;
}
