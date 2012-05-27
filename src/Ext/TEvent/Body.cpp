#include "Body.h"

//Static init
Container<TEventExt> TEventExt::ExtMap;

template<> TEventExt::TT *Container<TEventExt>::SavingObject = NULL;
template<> IStream *Container<TEventExt>::SavingStream = NULL;

void TEventExt::ExtData::Initialize(TEventClass *pThis)
{

}

// Gets the TechnoType pointed to by the event's TechnoName field.
/*!
	Resolves the TechnoName to a TechnoTypeClass and caches it. This function
	is an O(n) operation for the first call, every subsequent call is O(1).

	\returns The TechnoTypeClass TechnoName points to, NULL if not set or invalid.

	\date 2012-05-09
*/
TechnoTypeClass* TEventExt::ExtData::GetTechnoType()
{
	if(!this->TechnoType.isset()) {
		TechnoTypeClass* pType = NULL;

		const char* eventTechno = this->AttachedToObject->TechnoName;
		for(auto i=TechnoTypeClass::Array->start(); i<TechnoTypeClass::Array->end(); ++i) {
			if(!_stricmp((*i)->ID, eventTechno)) {
				pType = *i;
				break;
			}
		}

		this->TechnoType.Set(pType);
	}

	return this->TechnoType;
}

// Gets whether the referenced TechnoType exist at least 'count' times on the map.
/*!
	\returns True if TechnoType exists at least count times, false otherwise.

	\date 2012-05-09
*/
bool TEventExt::ExtData::TechTypeExists()
{
	int count = this->AttachedToObject->arg;
	TechnoTypeClass* pType = this->GetTechnoType();

	// decreases count by the number of owned techno types. iff count is zero or less,
	// this techno type exists at least 'count' times.
	for(auto i=HouseClass::Array->start(); i<HouseClass::Array->end(); ++i) {
		count -= (*i)->CountOwnedAndPresent(pType);

		if(count <= 0) {
			return true;
		}
	}

	return false;
}

// Gets whether the referenced TechnoType does not exist on the map.
/*!
	\returns True if TechnoType does not exists, false otherwise.

	\date 2012-05-09
*/
bool TEventExt::ExtData::TechTypeDoesNotExist()
{
	TechnoTypeClass* pType = this->GetTechnoType();

	// if any house owns this, this check fails.
	for(auto i=HouseClass::Array->start(); i<HouseClass::Array->end(); ++i) {
		if((*i)->CountOwnedAndPresent(pType) > 0) {
			return false;
		}
	}

	return true;
}

// Handles the occurence of events.
/*!
	Override any checks for whether an event occured or not. Set ret to true
	if the event occured, to false otherwise.

	\returns True if this event was handled by Ares, false otherwise.

	\date 2012-05-09
*/
bool TEventExt::HasOccured(TEventClass* pEvent, bool* ret) {
	auto pExt = ExtMap.Find(pEvent);

	switch(pEvent->EventKind) {
	case TriggerEvent::TechTypeExists:
		*ret = pExt->TechTypeExists();

	case TriggerEvent::TechTypeDoesntExist:
		*ret = pExt->TechTypeDoesNotExist();

	default:
		return false;
	}

	return true;
}

// =============================
// container hooks

DEFINE_HOOK(71E7F8, TEventClass_CTOR, 5)
{
	GET(TEventClass*, pItem, ESI);

	TEventExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}


DEFINE_HOOK(71FA80, TEventClass_SDDTOR, 6)
{
	GET(TEventClass*, pItem, ECX);

	TEventExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(71F8C0, TEventClass_SaveLoad_Prefix, 5)
DEFINE_HOOK_AGAIN(71F930, TEventClass_SaveLoad_Prefix, 8)
{
	GET_STACK(TEventExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<TEventExt>::SavingObject = pItem;
	Container<TEventExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(71F92B, TEventClass_Load_Suffix, 5)
{
	TEventExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(71F94A, TEventClass_Save_Suffix, 5)
{
	TEventExt::ExtMap.SaveStatic();
	return 0;
}
