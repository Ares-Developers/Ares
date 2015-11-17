#pragma once

#include "../Utilities/Constructs.h"
#include "../Utilities/Template.h"

class AbstractTypeClass;
class AnimClass;
class AnimTypeClass;
class TechnoClass;

class AttachEffectTypeClass {
public:

	AbstractTypeClass* Owner{ nullptr };
	Valueable<int> Duration{ 0 };
	Valueable<bool> Cumulative{ false };
	Valueable<bool> ForceDecloak{ false };
	Valueable<bool> DiscardOnEntry{ false };

	//#1573, #1623 animations on units
	Valueable<AnimTypeClass*> AnimType{ nullptr };
	Valueable<bool> AnimResetOnReapply{ false };
	Valueable<bool> TemporalHidesAnim{ false };

	//#255, crate stat modifiers on weapons
	Valueable<double> FirepowerMultiplier{ 1.0 };
	Valueable<double> ArmorMultiplier{ 1.0 };
	Valueable<double> SpeedMultiplier{ 1.0 };
	Valueable<bool> Cloakable{ false };

	//#408, residual damage
	/*
	Valueable<WarheadTypeClass*> Warhead{ nullptr };
	Valueable<int> Damage{ 0 };
	Valueable<int> DamageDelay{ 0 };
	*/

	//#1623-only tags
	Valueable<int> Delay{ 0 };

	void Attach(TechnoClass* pTarget, int duration, TechnoClass* pInvoker);
	//void Attach(TechnoClass* pTarget, int duration, TechnoClass* pInvoker, int damageDelay);

	bool Load(AresStreamReader &Stm, bool RegisterForChange);

	bool Save(AresStreamWriter &Stm) const;

	AttachEffectTypeClass(AbstractTypeClass* pOwner) : Owner(pOwner)
	{ }

	void Read(INI_EX &exINI);
};

class AttachEffectClass {
	struct UninitAnim {
		void operator() (AnimClass* const pAnim) const;
	};

public:
	AttachEffectClass() noexcept = default;

	AttachEffectClass(AttachEffectTypeClass* pType, int timer) noexcept :
		Type(pType),
		ActualDuration(timer)
	{ }

	AttachEffectTypeClass* Type{ nullptr };
	Handle<AnimClass*, UninitAnim> Animation{ nullptr };
	int ActualDuration{ 0 };

	TechnoClass* Invoker{ nullptr };
	//int ActualDamageDelay{ 0 };

	void InvalidatePointer(void* ptr);

	void CreateAnim(TechnoClass* pOwner);
	void KillAnim();

	bool Load(AresStreamReader &Stm, bool RegisterForChange);

	bool Save(AresStreamWriter &Stm) const;

	static void Update(TechnoClass* pSource);
};
