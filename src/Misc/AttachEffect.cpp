#include "AttachEffect.h"
#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/WarheadType/Body.h"
#include "../Utilities/TemplateDef.h"

#include <vector>

/*
Attached Effects system
covering feature requests
#1573, #1623 and #255

Graion Dilach, 2011-09-24+

In a nutshell:
	relatively flexible way to interact with the unit's properties (as many as possible)
	to create as many as possible interesting effects
	what are done are the crate-simulating effects and animations
	keep in mind, this does not contain any bugfixes within it, so everything can be applied as much as YR applies it
	*cough* cloaked jumpjets *cough*

	Some commented out code were aimed for Residual Damage, #408
	but it doesn't work as it is coded

*/

bool AttachEffectTypeClass::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return Stm
		.Process(this->Owner)
		.Process(this->Duration)
		.Process(this->Cumulative)
		.Process(this->ForceDecloak)
		.Process(this->DiscardOnEntry)
		.Process(this->AnimType)
		.Process(this->AnimResetOnReapply)
		.Process(this->TemporalHidesAnim)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
		.Process(this->SpeedMultiplier)
		.Process(this->Cloakable)
		.Process(this->Delay)
		.Success()
		&& Stm.RegisterChange(this); // announce this type
}

bool AttachEffectTypeClass::Save(AresStreamWriter &Stm) const {
	return Stm
		.Process(this->Owner)
		.Process(this->Duration)
		.Process(this->Cumulative)
		.Process(this->ForceDecloak)
		.Process(this->DiscardOnEntry)
		.Process(this->AnimType)
		.Process(this->AnimResetOnReapply)
		.Process(this->TemporalHidesAnim)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
		.Process(this->SpeedMultiplier)
		.Process(this->Cloakable)
		.Process(this->Delay)
		.Success()
		&& Stm.RegisterChange(this);
}

void AttachEffectTypeClass::Read(INI_EX &exINI) {
	auto const pSection = this->Owner->ID;
	this->Duration.Read(exINI, pSection, "AttachEffect.Duration");
	this->Cumulative.Read(exINI, pSection, "AttachEffect.Cumulative");
	this->AnimType.Read(exINI, pSection, "AttachEffect.Animation");
	this->AnimResetOnReapply.Read(exINI, pSection, "AttachEffect.AnimResetOnReapply");
	this->TemporalHidesAnim.Read(exINI, pSection, "AttachEffect.TemporalHidesAnim");
	this->ForceDecloak.Read(exINI, pSection, "AttachEffect.ForceDecloak");
	this->DiscardOnEntry.Read(exINI, pSection, "AttachEffect.DiscardOnEntry");
	this->FirepowerMultiplier.Read(exINI, pSection, "AttachEffect.FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, pSection, "AttachEffect.ArmorMultiplier");
	this->SpeedMultiplier.Read(exINI, pSection, "AttachEffect.SpeedMultiplier");
	this->Cloakable.Read(exINI, pSection, "AttachEffect.Cloakable");

	/*
	this->Damage.Read(exINI, pSection, "AttachEffect.Damage");
	this->DamageDelay.Read(exINI, pSection, "AttachEffect.DamageDelay");
	this->Warhead.Parse(exINI, pSection, "AttachEffect.Warhead");
	*/

	this->Delay.Read(exINI, pSection, "AttachEffect.Delay");
}

/*!
	This function attaches an effect to a target.

	\param pTarget The Techno which gets affected.
	\param duration The location the projectile detonated.
	\param pInvoker The Techno that casts the effect.

	\author Graion Dilach
	\date 2011-09-24+
*/

//void AttachEffectTypeClass::Attach(TechnoClass* pTarget, int duration, TechnoClass* pInvoker, int DamageDelay) {
void AttachEffectTypeClass::Attach(
	TechnoClass* const pTarget, int const duration,
	TechnoClass* const pInvoker)
{
	if(!pTarget || pTarget->IsIronCurtained()) {
		return;
	}

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	auto& Effects = pTargetExt->AttachedEffects;

	if(!this->Cumulative) {
		auto const it = std::find_if(Effects.begin(), Effects.end(),
			[=](auto const& item) { return item.Type == this; });

		if(it != Effects.end()) {
			auto& Item = *it;

			Item.ActualDuration = Item.Type->Duration;

			if(this->AnimType && this->AnimResetOnReapply) {
				Item.CreateAnim(pTarget);
			}

			if(this->ForceDecloak) {
				auto const state = pTarget->CloakState;
				if(state == CloakState::Cloaked
					|| state == CloakState::Cloaking)
				{
					pTarget->Uncloak(true);
				}
			}

			return;
		}
	}

	// there goes the actual attaching
	Effects.emplace_back(this, duration);
	auto& Attaching = Effects.back();

	Attaching.Invoker = pInvoker;

	// update the unit with the attached effect
	pTargetExt->RecalculateStats();

	// check cloak
	if(this->ForceDecloak && pTarget->CloakState != CloakState::Uncloaked) {
		pTarget->Uncloak(true);
	}

	// animation
	Attaching.CreateAnim(pTarget);
}

bool AttachEffectClass::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return Stm
		.Process(this->Type)
		.Process(this->Animation)
		.Process(this->ActualDuration)
		.Process(this->Invoker)
		.Success();
}

bool AttachEffectClass::Save(AresStreamWriter &Stm) const {
	return Stm
		.Process(this->Type)
		.Process(this->Animation)
		.Process(this->ActualDuration)
		.Process(this->Invoker)
		.Success();
}

void AttachEffectClass::UninitAnim::operator() (AnimClass* const pAnim) const {
	pAnim->SetOwnerObject(nullptr);
	pAnim->UnInit();
}

void AttachEffectClass::InvalidatePointer(void* ptr) {
	if(this->Invoker == ptr) {
		this->Invoker = nullptr;
	}

	if(this->Animation == ptr) {
		this->KillAnim();
	}
}

void AttachEffectClass::CreateAnim(TechnoClass* pOwner) {
	auto const pType = this->Type;

	auto const state = pOwner->CloakState;
	if((state == CloakState::Cloaked || state == CloakState::Cloaking) ||
		(pOwner->TemporalTargetingMe && pType->TemporalHidesAnim)) {
		return;
	}

	if(AnimTypeClass* const pAnimType = pType->AnimType) {
		this->Animation.reset(GameCreate<AnimClass>(pAnimType, pOwner->Location));

		if(AnimClass* const pAnim = this->Animation) {
			pAnim->SetOwnerObject(pOwner);
			pAnim->RemainingIterations = 0xFFu;

			if(this->Invoker && this->Invoker->Owner) {
				pAnim->Owner = this->Invoker->Owner;
			}
		}
	}
}

//animation remover, boolean is needed otherwise destructor goes to infinite loop during UnInit
void AttachEffectClass::KillAnim() {
	this->Animation.clear();
}

/*!
	This function updates the units' AttachEffects.

	(retval boolean, to see if the unit gets killed (scrapped 408, it always crashed))

	\param pSource The currently updated Techno.

	\author Graion Dilach
	\date 2011-09-24+
*/

void AttachEffectClass::Update(TechnoClass* pSource) {
	auto const Logging = false;

	if(!pSource || pSource->InLimbo || pSource->IsImmobilized || pSource->Transporter) {
		return;
	}

	auto const pType = pSource->GetTechnoType();
	auto const pData = TechnoExt::ExtMap.Find(pSource);
	auto const pTypeData = TechnoTypeExt::ExtMap.Find(pType);

	if(!pData->AttachedEffects.empty()) {

		if(pSource->CloakState == CloakState::Cloaked
			|| pSource->CloakState == CloakState::Cloaking)
		{
			if(!pData->AttachEffects_RecreateAnims) {
				for(auto& Item : pData->AttachedEffects) {
					Item.KillAnim();
				}
				pData->AttachEffects_RecreateAnims = true;
			}
		} else {
			if(pData->AttachEffects_RecreateAnims) {
				for(auto& Item : pData->AttachedEffects) {
					Item.CreateAnim(pSource);
				}
				pData->AttachEffects_RecreateAnims = false;
			}
		}

		Debug::Log(Logging,
			"[AttachEffect]AttachEffect update of %s...\n", pType->ID);

		for(auto& Effect : pData->AttachedEffects) {
			auto const pEffectType = Effect.Type;
			auto duration = Effect.ActualDuration;

			if(duration > 0) {
				--duration;
			}

			//#408, residual damage
			/* unfinished, crash if unit gets killed and was targeted
			if(pEffectType->Damage) {
				if(Effect.ActualDamageDelay) {
					--Effect.ActualDamageDelay;
				} else {
					Effect.ActualDamageDelay = pEffectType->DamageDelay;

					auto const pAttacker = Effect.Invoker
						? Effect.Invoker : pSource;
					auto const pHouse = pKiller->Owner;

					int damage = pEffectType->Damage;
					auto const res = pSource->ReceiveDamage(
						&damage, 0, pEffectType->Warhead, pAttacker, false,
						false, pHouse);

					if(res == DamageState::NowDead) {
						// check if the unit is still alive, if residual damage
						// killed it, no reason to continue
						return false;	//this is a void atm
					}
				}
			}*/

			auto const isOwnType = (pEffectType->Owner == pType);
			if(isOwnType && pSource->Deactivated) {
				duration = 0;
			}

			// expired effect will be removed
			if(!duration) {
				if(isOwnType) { //#1623, hardcodes Cumulative to false
					pData->AttachedTechnoEffect_isset = false;
					pData->AttachedTechnoEffect_Delay = pEffectType->Delay;
				}

				Debug::Log(Logging,
					"[AttachEffect]Item at %d expired, removing...\n",
					&Effect - pData->AttachedEffects.data());
			}

			Effect.ActualDuration = duration;
		}

		// remove the expired effects and update the unit's properties
		auto const it = std::remove_if(
			pData->AttachedEffects.begin(), pData->AttachedEffects.end(),
			[](AttachEffectClass const& Item)
		{
			return !Item.ActualDuration;
		});

		if(it != pData->AttachedEffects.end()) {
			auto const count = pData->AttachedEffects.end() - it;
			pData->AttachedEffects.erase(it, pData->AttachedEffects.end());
			pData->RecalculateStats();

			Debug::Log(Logging,
				"[AttachEffect]Removing %d item(s) was successful.\n", count);
		}

		Debug::Log(Logging, "[AttachEffect]Update was successful.\n");
	}

	//#1623 - generating AttachedEffect from Type
	if(pTypeData->AttachedTechnoEffect.Duration && !pData->AttachedTechnoEffect_isset) {
		if(!pData->AttachedTechnoEffect_Delay) {
			if(!pSource->Deactivated) {
				Debug::Log(Logging, "[AttachEffect]Missing Type effect of %s...\n", pType->ID);
				//pTypeData->AttachedTechnoEffect.Attach(pSource, pTypeData->AttachedTechnoEffect.Duration, pSource, pTypeData->AttachedTechnoEffect.DamageDelay);

				pTypeData->AttachedTechnoEffect.Attach(pSource, pTypeData->AttachedTechnoEffect.Duration, pSource);

				pData->AttachedTechnoEffect_isset = true;
				Debug::Log(Logging, "[AttachEffect]Readded to %s.\n", pType->ID);
			}
		} else if(pData->AttachedTechnoEffect_Delay > 0) {
			--pData->AttachedTechnoEffect_Delay;
		}
	}
}
