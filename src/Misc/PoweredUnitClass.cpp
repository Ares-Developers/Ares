#include <BuildingClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>
#include <GeneralStructures.h>
#include "../Ext/Building/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "Debug.h"
#include "EMPulse.h"
#include "PoweredUnitClass.h"

bool PoweredUnitClass::IsPoweredBy(HouseClass* const pOwner) const
{
	auto const pType = this->Techno->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	auto const& PoweredBy = pTypeExt->PoweredBy;

	for(auto const& pBuilding : pOwner->Buildings) {
		auto const inArray = PoweredBy.Contains(pBuilding->Type);

		if(inArray && !pBuilding->BeingWarpedOut && !pBuilding->IsUnderEMP()) {
			auto const pExt = TechnoExt::ExtMap.Find(pBuilding);
			if(pExt->IsOperated() && pBuilding->IsPowerOnline()) {
				return true;
			}
		}
	}
	
	return false;
}

void PoweredUnitClass::PowerUp()
{
	auto const pTechno = this->Techno;
	auto const pExt = TechnoExt::ExtMap.Find(pTechno);

	if(!pTechno->IsUnderEMP() && pExt->IsOperated()) {
		EMPulse::DisableEMPEffect2(pTechno);
	}
}

bool PoweredUnitClass::PowerDown()
{
	auto const pTechno = this->Techno;

	if(EMPulse::IsDeactivationAdvisable(pTechno)) {
		// destroy if EMP.Threshold would crash this unit when in air
		auto const pType = pTechno->GetTechnoType();
		auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

		if(EMPulse::EnableEMPEffect2(pTechno)
			|| (pExt->EMP_Threshold && pTechno->IsInAir()))
		{
			return false;
		}
	}

	return true;
}

bool PoweredUnitClass::Update()
{
	if((Unsorted::CurrentFrame - this->LastScan) < PoweredUnitClass::ScanInterval) {
		return true;
	}

	auto const pTechno = this->Techno;

	if(!pTechno->IsAlive || !pTechno->Health || pTechno->InLimbo) {
		return true;
	}

	this->LastScan = Unsorted::CurrentFrame;

	auto const pOwner = pTechno->Owner;
	auto const hasPower = this->IsPoweredBy(pOwner);
	
	this->Powered = hasPower;
	
	if(hasPower && pTechno->Deactivated) {
		this->PowerUp();
	} else if(!hasPower && !pTechno->Deactivated) {
		// don't shutdown units inside buildings (warfac, barracks, shipyard) because that locks up the factory and the robot tank did it
		auto const whatAmI = pTechno->WhatAmI();
		if((whatAmI != InfantryClass::AbsID && whatAmI != UnitClass::AbsID) || (!pTechno->GetCell()->GetBuilding())) {
			return this->PowerDown();
		}
	}

	return true;
}

bool PoweredUnitClass::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return Stm
		.Process(this->Techno)
		.Process(this->LastScan)
		.Process(this->Powered)
		.Success();
}

bool PoweredUnitClass::Save(AresStreamWriter &Stm) const {
	return Stm
		.Process(this->Techno)
		.Process(this->LastScan)
		.Process(this->Powered)
		.Success();
}
