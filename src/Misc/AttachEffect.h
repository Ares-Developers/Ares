#ifndef ATTACHEFFECT_H_
#define ATTACHEFFECT_H_

#include <vector>
#include <TechnoClass.h>

#include "../Utilities/Template.h"


class AttachEffectTypeClass {
public:

	char ID[24]; // as westwood once said, 24 chars ought to be enough for any ID
	Valueable<int> Duration;
	Valueable<bool> Cumulative;
	
	//#1573, #1623 animations on units
	Valueable<AnimTypeClass *> AnimType;
	Valueable<bool> AnimResetOnReapply;

	//#255, crate stat modifiers on weapons
	Valueable<double> FirepowerMultiplier;
	Valueable<double> ArmorMultiplier;
	Valueable<double> SpeedMultiplier;
	Valueable<bool> Cloakable;

	//#408, residual damage
	/*
	Valueable<WarheadTypeClass *> Warhead;
	Valueable<int> Damage;
	Valueable<int> DamageDelay;
	*/

	//#1623-only tags
	Valueable<int> Delay;

	virtual void Attach(TechnoClass* Target, int Duration, TechnoClass* Invoker);
	//virtual void Attach(TechnoClass* Target, int Duration, TechnoClass* Invoker, int DamageDelay);
	
	AttachEffectTypeClass(): Cumulative(false),
		Duration(0),
		AnimType(NULL),
		AnimResetOnReapply(false),
		FirepowerMultiplier(1),
		ArmorMultiplier(1),
		SpeedMultiplier(1),
		Cloakable(false),
		/*
		Warhead(RulesClass::Global()->C4Warhead),
		Damage(0),
		DamageDelay(0),
		*/
		Delay(0)
		{
			this->ID[0] = 0;
		};

	void Read(INI_EX *exINI, const char * section);
};

class AttachEffectClass {
public:
	AttachEffectClass(AttachEffectTypeClass* AEType, int Timer): Type(AEType), Animation(NULL), AnimAlreadyKilled(true), ActualDuration(Timer) {
	}

	AttachEffectTypeClass * Type;
	AnimClass * Animation;
	bool AnimAlreadyKilled;
	int ActualDuration;

	TechnoClass * Invoker;
	//int ActualDamageDelay;

	void Destroy();

	void InvalidateAnimPointer(AnimClass *ptr);

	void KillAnim();

	static bool Update(TechnoClass *Source);
};

#endif
