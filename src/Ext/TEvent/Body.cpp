#include "Body.h"

//Static init
template<> const DWORD Extension<TEventClass>::Canary = 0x61616161;
Container<TEventExt> TEventExt::ExtMap;

template<> TEventExt::TT *Container<TEventExt>::SavingObject = nullptr;
template<> IStream *Container<TEventExt>::SavingStream = nullptr;

void TEventExt::ExtData::Initialize(TEventClass *pThis)
{

}

// Gets the TechnoType pointed to by the event's TechnoName field.
/*!
	Resolves the TechnoName to a TechnoTypeClass and caches it. This function
	is an O(n) operation for the first call, every subsequent call is O(1).

	\returns The TechnoTypeClass TechnoName points to, nullptr if not set or invalid.

	\date 2012-05-09, 2013-02-09
*/
TechnoTypeClass* TEventExt::ExtData::GetTechnoType()
{
	if(!this->TechnoType.isset()) {
		const char* eventTechno = this->AttachedToObject->TechnoName;
		TechnoTypeClass* pType = TechnoTypeClass::Find(eventTechno);

		if(!pType) {
			Debug::DevLog(Debug::Error, "Event references non-existing techno type \"%s\".", eventTechno);
		}

		this->TechnoType.Set(pType);
	}

	return this->TechnoType;
}

// Gets whether the referenced TechnoType exist at least 'count' times.
/*!
	\remark Returns false if the type cannot be resolved.

	\returns True if TechnoType exists at least count times, false otherwise.

	\date 2012-05-09, last updated 2013-03-03
*/
bool TEventExt::ExtData::TechTypeExists()
{
	int count = this->AttachedToObject->arg;
	TechnoTypeClass* pType = this->GetTechnoType();

	if(!pType) {
		return false;
	}

	if(pType->Insignificant || pType->DontScore) {
		// check each techno and subtract one if the type matches.
		for(auto i=TechnoClass::Array->begin(); i<TechnoClass::Array->end(); ++i) {
			if((*i)->GetTechnoType() == pType) {
				count--;

				if(count <= 0) {
					return true;
				}
			}
		}
	} else {
		// decreases count by the number of owned techno types. iff count is zero or less,
		// this techno type exists at least 'count' times.
		for(auto i=HouseClass::Array->begin(); i<HouseClass::Array->end(); ++i) {
			count -= (*i)->CountOwnedNow(pType);

			if(count <= 0) {
				return true;
			}
		}
	}

	return count <= 0;
}

// Gets whether the referenced TechnoType does not exist.
/*!
	\remark Returns false if the type cannot be resolved.

	\returns True if TechnoType does not exists, false otherwise.

	\date 2012-05-09, last updated 2013-03-03
*/
bool TEventExt::ExtData::TechTypeDoesNotExist()
{
	TechnoTypeClass* pType = this->GetTechnoType();

	if(!pType) {
		return false;
	}

	if(pType->Insignificant || pType->DontScore) {
		// we have to loop through all technos here, because game doesn't
		// keep track of this type.
		for(auto i=TechnoClass::Array->begin(); i<TechnoClass::Array->end(); ++i) {
			if((*i)->GetTechnoType() == pType) {
				return false;
			}
		}		
	} else {
		// if any house owns this, this check fails.
		for(auto i=HouseClass::Array->begin(); i<HouseClass::Array->end(); ++i) {
			if((*i)->CountOwnedNow(pType) > 0) {
				return false;
			}
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
		break;

	case TriggerEvent::TechTypeDoesntExist:
		*ret = pExt->TechTypeDoesNotExist();
		break;

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

DEFINE_HOOK_AGAIN(71F930, TEventClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(71F8C0, TEventClass_SaveLoad_Prefix, 5)
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
