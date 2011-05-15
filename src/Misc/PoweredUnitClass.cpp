#include <BuildingClass.h>
#include <HouseClass.h>
#include <GeneralStructures.h>
#include "../Ext/Building/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "Debug.h"
#include "EMPulse.h"
#include "PoweredUnitClass.h"

bool PoweredUnitClass::IsPoweredBy(HouseClass* Owner) const
{
	for(int i = 0; i < Owner->Buildings.Count; ++i)	{
		BuildingClass* Building  = Owner->Buildings.GetItem(i);
		TechnoExt::ExtData* BExt = TechnoExt::ExtMap.Find(Building);
		
		for(int j = 0; j < this->Ext->PoweredBy.Count; ++j) {
			if( !( Building->Type != this->Ext->PoweredBy.GetItem(j)
				|| Building->BeingWarpedOut
				|| Building->IsUnderEMP()
				|| !BExt->IsOperated()
				|| !Building->IsPowerOnline() )
			) return true;
		}
	}
	
	return false;
}

void PoweredUnitClass::PowerUp()
{
	TechnoExt::ExtData* e = TechnoExt::ExtMap.Find(this->Techno);
	if( !this->Techno->IsUnderEMP() && e->IsOperated() ) {
		EMPulse::DisableEMPEffect2(this->Techno);
	}
}

void PoweredUnitClass::PowerDown()
{
	if( EMPulse::IsDeactivationAdvisable(this->Techno) && !EMPulse::EnableEMPEffect2(this->Techno) ) {
		// for EMP.Threshold=inair
		if( this->Ext->EMP_Threshold == -1 && this->Techno->IsInAir() )	{
			this->Techno->Destroyed(NULL);
			this->Techno->Crash(NULL);
			
			if (this->Techno->Owner == HouseClass::Player) {
				VocClass::PlayAt(this->Techno->GetTechnoType()->VoiceCrashing, &this->Techno->Location, NULL);
			}
		}
	}
}

void PoweredUnitClass::Update()
{
	if( (Unsorted::CurrentFrame - this->LastScan) < this->ScanInterval ) return;
	
	HouseClass* Owner = this->Techno->Owner;
	bool HasPower     = this->IsPoweredBy(Owner);
	
	this->Powered = HasPower;
	
	if(HasPower && this->Techno->Deactivated) {
		this->PowerUp();
	} else if(!HasPower && !this->Techno->Deactivated) {
		this->PowerDown();
	}
	
	LastScan = Unsorted::CurrentFrame;
}
