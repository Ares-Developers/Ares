#ifndef ATTACHEFFECT_H_
#define ATTACHEFFECT_H_

#include <vector>
#include <TechnoClass.h>

#include "../Utilities/Template.h"


class AttachEffectTypeClass {
public:

	const char* ID;
	Valueable<int> Duration;
	Valueable<bool> Cumulative;
	
	//#1573, #1623 animations on units
	Valueable<AnimTypeClass *> AnimType;

	//#255, crate stat modifiers on weapons
	Valueable<double> FirepowerMultiplier;
	Valueable<double> ArmorMultiplier;
	Valueable<double> SpeedMultiplier;
	Valueable<bool> Cloakable;


	virtual void Attach(TechnoClass* Target, int Duration);

	AttachEffectTypeClass(){
		this->Cumulative = false;
		this->Duration = 0;
		this->AnimType = NULL;
		this->FirepowerMultiplier = 1;
		this->ArmorMultiplier = 1;
		this->SpeedMultiplier = 1;
		this->Cloakable = false;


	};

	void Read(INI_EX *exINI, const char * section) {
		
		this->ID = section;
		this->Duration.Read(exINI, section, "AttachEffect.Duration");
		this->Cumulative.Read(exINI, section, "AttachEffect.Cumulative");
		this->AnimType.Parse(exINI, section, "AttachEffect.Animation");
		this->FirepowerMultiplier.Read(exINI, section, "AttachEffect.FirepowerMultiplier");
		this->ArmorMultiplier.Read(exINI, section, "AttachEffect.ArmorMultiplier");
		this->SpeedMultiplier.Read(exINI, section, "AttachEffect.SpeedMultiplier");
		this->Cloakable.Read(exINI, section, "AttachEffect.Cloakable");


	}
};

class AttachEffectClass {
public:
	AttachEffectClass(AttachEffectTypeClass* AEType, int Timer){
	this->Type = AEType;
	this->ActualDuration = Timer;
	this->Animation = NULL;
	}

	AttachEffectTypeClass * Type;
	AnimClass * Animation;
	int ActualDuration;

	void Destroy();
};


#endif