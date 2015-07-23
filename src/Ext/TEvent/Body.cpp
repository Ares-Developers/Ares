#include "Body.h"

#include "../../Misc/SavegameDef.h"

#include <HouseClass.h>
#include <TechnoTypeClass.h>

//Static init
template<> const DWORD Extension<TEventClass>::Canary = 0x61616161;
TEventExt::ExtContainer TEventExt::ExtMap;

// Gets the TechnoType pointed to by the event's TechnoName field.
/*!
	Resolves the TechnoName to a TechnoTypeClass and caches it. This function
	is an O(n) operation for the first call, every subsequent call is O(1).

	\returns The TechnoTypeClass TechnoName points to, nullptr if not set or invalid.

	\date 2012-05-09, 2013-02-09
*/
TechnoTypeClass* TEventExt::ExtData::GetTechnoType()
{
	if(this->TechnoType.empty()) {
		const char* eventTechno = this->OwnerObject()->TechnoName;
		TechnoTypeClass* pType = TechnoTypeClass::Find(eventTechno);

		if(!pType) {
			Debug::Log(Debug::Severity::Error, "Event references non-existing techno type \"%s\".", eventTechno);
			Debug::RegisterParserError();
		}

		this->TechnoType = pType;
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
	int count = this->OwnerObject()->arg;
	TechnoTypeClass* pType = this->GetTechnoType();

	if(!pType) {
		return false;
	}

	if(pType->Insignificant || pType->DontScore) {
		// check each techno and subtract one if the type matches.
		for(auto pTechno : *TechnoClass::Array) {
			if(pTechno->GetTechnoType() == pType) {
				count--;

				if(count <= 0) {
					return true;
				}
			}
		}
	} else {
		// decreases count by the number of owned techno types. iff count is zero or less,
		// this techno type exists at least 'count' times.
		for(auto pHouse : *HouseClass::Array) {
			count -= pHouse->CountOwnedNow(pType);

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
		for(auto pTechno : *TechnoClass::Array) {
			if(pTechno->GetTechnoType() == pType) {
				return false;
			}
		}		
	} else {
		// if any house owns this, this check fails.
		for(auto pHouse : *HouseClass::Array) {
			if(pHouse->CountOwnedNow(pType) > 0) {
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

// Resolves a param to a house.
HouseClass* TEventExt::ResolveHouseParam(int const param, HouseClass* const pOwnerHouse) {
	if(param == 8997) {
		return pOwnerHouse;
	}

	if(HouseClass::Index_IsMP(param)) {
		return HouseClass::FindByIndex(param);
	}

	return HouseClass::FindByCountryIndex(param);
}

// =============================
// load / save

template <typename T>
void TEventExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->TechnoType);
}

void TEventExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<TEventClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TEventExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<TEventClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

TEventExt::ExtContainer::ExtContainer() : Container("TEventClass") {
}

TEventExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(71E7F8, TEventClass_CTOR, 5)
{
	GET(TEventClass*, pItem, ESI);

	TEventExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(71FAA6, TEventClass_SDDTOR, 6)
{
	GET(TEventClass*, pItem, ESI);

	TEventExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(71F930, TEventClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(71F8C0, TEventClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TEventClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TEventExt::ExtMap.PrepareStream(pItem, pStm);

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
