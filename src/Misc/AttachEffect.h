#ifndef ATTACHEFFECT_H_
#define ATTACHEFFECT_H_

#include <vector>
#include <TechnoClass.h>

#include "../Utilities/Template.h"


class AttachEffectTypeClass {
public:

	AbstractTypeClass* Owner;
	Valueable<int> Duration;
	Valueable<bool> Cumulative;
	Valueable<bool> ForceDecloak;
	Valueable<bool> DiscardOnEntry;
	
	//#1573, #1623 animations on units
	Valueable<AnimTypeClass *> AnimType;
	Valueable<bool> AnimResetOnReapply;
	Valueable<bool> TemporalHidesAnim;
	
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

	void Attach(TechnoClass* Target, int Duration, TechnoClass* Invoker);
	//void Attach(TechnoClass* Target, int Duration, TechnoClass* Invoker, int DamageDelay);

	bool Load(AresStreamReader &Stm, bool RegisterForChange);

	bool Save(AresStreamWriter &Stm) const;
	
	AttachEffectTypeClass(AbstractTypeClass* Owner) : Owner(Owner),
		Cumulative(false),
		Duration(0),
		AnimType(nullptr),
		AnimResetOnReapply(false),
		TemporalHidesAnim(false),
		ForceDecloak(false),
		DiscardOnEntry(false),
		FirepowerMultiplier(1.0),
		ArmorMultiplier(1.0),
		SpeedMultiplier(1.0),
		Cloakable(false),
		/*
		Warhead(RulesClass::Global()->C4Warhead),
		Damage(0),
		DamageDelay(0),
		*/
		Delay(0)
	{
	}

	void Read(INI_EX &exINI);
};

ENABLE_ARES_PERSISTENCE(AttachEffectTypeClass);

class AttachEffectClass {
public:
	AttachEffectClass(AttachEffectTypeClass* Type, int Timer) : Type(Type),
		Animation(nullptr),
		ActualDuration(Timer),
		Invoker(nullptr)
	{
	}

	~AttachEffectClass() {
		this->Destroy();
	}

	AttachEffectTypeClass * Type;
	AnimClass * Animation;
	int ActualDuration;

	TechnoClass * Invoker;
	//int ActualDamageDelay;

	void Destroy();

	void InvalidatePointer(void *ptr);

	void CreateAnim(TechnoClass *Owner);
	void KillAnim();

	bool Load(AresStreamReader &Stm, bool RegisterForChange);

	bool Save(AresStreamWriter &Stm) const;

	static void Update(TechnoClass *Source);
};

ENABLE_ARES_PERSISTENCE(AttachEffectClass);

#endif
