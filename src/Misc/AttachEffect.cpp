#include "AttachEffect.h"
#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/WarheadType/Body.h"

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

void AttachEffectTypeClass::Read(INI_EX *exINI, const char * section) {
	AresCRT::strCopy(this->ID, section);
	this->Duration.Read(exINI, section, "AttachEffect.Duration");
	this->Cumulative.Read(exINI, section, "AttachEffect.Cumulative");
	this->AnimType.Parse(exINI, section, "AttachEffect.Animation");
	this->AnimResetOnReapply.Read(exINI, section, "AttachEffect.AnimResetOnReapply");
	this->TemporalHidesAnim.Read(exINI, section, "AttachEffect.TemporalHidesAnim");
	this->ForceDecloak.Read(exINI, section, "AttachEffect.ForceDecloak");
	this->DiscardOnEntry.Read(exINI, section, "AttachEffect.DiscardOnEntry");
	this->FirepowerMultiplier.Read(exINI, section, "AttachEffect.FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, section, "AttachEffect.ArmorMultiplier");
	this->SpeedMultiplier.Read(exINI, section, "AttachEffect.SpeedMultiplier");
	this->Cloakable.Read(exINI, section, "AttachEffect.Cloakable");

	/*
	this->Damage.Read(exINI, section, "AttachEffect.Damage");
	this->DamageDelay.Read(exINI, section, "AttachEffect.DamageDelay");
	this->Warhead.Parse(exINI, section, "AttachEffect.Warhead");
	*/

	this->Delay.Read(exINI, section, "AttachEffect.Delay");
}

/*!
	This function attaches an effect to a target.

	\param Target The Techno which gets affected.
	\param Duration The location the projectile detonated.
	\param Invoker The Techno that casts the effect.

	\author Graion Dilach
	\date 2011-09-24+
*/

//void AttachEffectTypeClass::Attach(TechnoClass* Target, int Duration, TechnoClass* Invoker, int DamageDelay) {
void AttachEffectTypeClass::Attach(TechnoClass* Target, int Duration, TechnoClass* Invoker) {
	if (!Target || Target->IsIronCurtained()) {return;}

	TechnoExt::ExtData *TargetExt = TechnoExt::ExtMap.Find(Target);

	if (!this->Cumulative) {
		for (size_t i=0; i < TargetExt->AttachedEffects.size(); i++) {
			auto &Item = TargetExt->AttachedEffects.at(i);
			if (!strcmp(this->ID, Item->Type->ID)) {
				Item->ActualDuration = Item->Type->Duration;

				if (!!this->AnimType && !!this->AnimResetOnReapply) {
					Item->CreateAnim(Target);
				}

				if (!!this->ForceDecloak && (Target->CloakState == CloakState::Cloaked || Target->CloakState == CloakState::Cloaking)) {
					Target->Uncloak(true);
				}

				return;
			}
		}
	}

	// there goes the actual attaching
	TargetExt->AttachedEffects.push_back(std::make_unique<AttachEffectClass>(this, Duration));
	auto &Attaching = TargetExt->AttachedEffects.back();

	Attaching->Invoker = Invoker;

	// update the unit with the attached effect
	TechnoExt::RecalculateStats(Target);

	//check cloak
	if (!!this->ForceDecloak && Target->CloakState) {
		Target->Uncloak(true);
	}

	// animation
	Attaching->CreateAnim(Target);
	
}

void AttachEffectClass::InvalidatePointer(void *ptr) {
	if(this->Invoker == ptr) {
		this->Invoker = nullptr;
	}

	if(this->Animation == ptr) {
		this->KillAnim();
	}
}

void AttachEffectClass::CreateAnim(TechnoClass *Owner) {
	if ((Owner->CloakState == CloakState::Cloaked || Owner->CloakState == CloakState::Cloaking) ||
		(Owner->TemporalTargetingMe && !!this->Type->TemporalHidesAnim)) {
		return;
	}

	if (!!this->Type->AnimType) {
		if (this->Animation){
			this->KillAnim();
		}

		GAME_ALLOC(AnimClass, this->Animation, this->Type->AnimType, &Owner->Location);
		if (auto pAnim = this->Animation) {
			pAnim->SetOwnerObject(Owner);
			pAnim->RemainingIterations = 0xFFu;

			if (this->Invoker && this->Invoker->Owner) {
				pAnim->Owner = this->Invoker->Owner;
			}
		}
	}
}

//animation remover, boolean is needed otherwise destructor goes to infinite loop during UnInit
void AttachEffectClass::KillAnim() {
	if (auto pAnim = this->Animation) {
		this->Animation = nullptr;
		pAnim->SetOwnerObject(nullptr);
		pAnim->UnInit();
	}
}

//destructor
void AttachEffectClass::Destroy() {
	this->KillAnim();
}

/*!
	This function updates the units' AttachEffects.

	(retval boolean, to see if the unit gets killed (scrapped 408, it always crashed))

	\param Source The currently updated Techno.

	\author Graion Dilach
	\date 2011-09-24+
*/

void AttachEffectClass::Update(TechnoClass *Source) {

	if (!Source || Source->InLimbo || Source->IsImmobilized || Source->Transporter) {
		return;
	}

	TechnoTypeClass *pType = Source->GetTechnoType();
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Source);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(pType);


	if (!pData->AttachedEffects.empty()) {

		if (!pData->AttachEffects_RecreateAnims &&
		(Source->CloakState == CloakState::Cloaked || Source->CloakState == CloakState::Cloaking)) {
			for (size_t i = pData->AttachedEffects.size(); i > 0; --i) {
				pData->AttachedEffects.at(i - 1)->KillAnim();
			}
			pData->AttachEffects_RecreateAnims = true;
		}

		if (pData->AttachEffects_RecreateAnims &&
		!(Source->CloakState == CloakState::Cloaked || Source->CloakState == CloakState::Cloaking)) {
			for (size_t i = pData->AttachedEffects.size(); i > 0; --i) {
				pData->AttachedEffects.at(i - 1)->CreateAnim(Source);
			}
			pData->AttachEffects_RecreateAnims = false;
		}

		//Debug::Log("[AttachEffect]AttachEffect update of %s...\n", Source->get_ID());
		for (size_t i = pData->AttachedEffects.size(); i > 0; --i) {
			auto Effect = pData->AttachedEffects.at(i - 1).get();
			if(Effect->ActualDuration > 0) {
				--Effect->ActualDuration;
			}

			//#408, residual damage
			/* unfinished, crash if unit gets killed and was targeted
			if (!!Effect->Type->Damage) {
				if (Effect->ActualDamageDelay) {
					Effect->ActualDamageDelay--;
				} else {
					if (Effect->Invoker) {
						Source->ReceiveDamage(Effect->Type->Damage, 0, Effect->Type->Warhead, Effect->Invoker, false, false, Effect->Invoker->Owner);
					} else {
						Source->ReceiveDamage(Effect->Type->Damage, 0, Effect->Type->Warhead, Source, false, false, Source->Owner);
					}

					if(Source->InLimbo || !Source->IsAlive || !Source->Health) {
						//check if the unit is still alive, if residual damage killed it, no reason to continue
						return false;	//this is a void atm
					}

					Effect->ActualDamageDelay = Effect->Type->DamageDelay;
				}

			}*/


			if(!Effect->ActualDuration || (!strcmp(Effect->Type->ID, pType->ID) && Source->Deactivated)) {
				//Debug::Log("[AttachEffect] %d. item expired, removing...\n", i - 1);

				if (!strcmp(Effect->Type->ID, pType->ID)) {		//#1623, hardcodes Cumulative to false
					pData->AttachedTechnoEffect_isset = false;
					pData->AttachedTechnoEffect_Delay = Effect->Type->Delay;
				}

				pData->AttachedEffects.erase(pData->AttachedEffects.begin() + i - 1);
				TechnoExt::RecalculateStats(Source);	//and update the unit's properties
				//Debug::Log("[AttachEffect] Remove #%d was successful.\n", i - 1);
			}

		}
		//Debug::Log("[AttachEffect]Update was successful.\n");
	}


	//#1623 - generating AttachedEffect from Type
	if (pTypeData->AttachedTechnoEffect.Duration != 0 && !pData->AttachedTechnoEffect_isset) {
		if (!pData->AttachedTechnoEffect_Delay) {
			if (!Source->Deactivated) {
				//Debug::Log("[AttachEffect]Missing Type effect of %s...\n", Source->get_ID());
				//pTypeData->AttachedTechnoEffect.Attach(Source, pTypeData->AttachedTechnoEffect.Duration, Source, pTypeData->AttachedTechnoEffect.DamageDelay);

				pTypeData->AttachedTechnoEffect.Attach(Source, pTypeData->AttachedTechnoEffect.Duration, Source);

				pData->AttachedTechnoEffect_isset = true;
				//Debug::Log("[AttachEffect]Readded to %s.\n", Source->get_ID());
			}

		} else if (pData->AttachedTechnoEffect_Delay > 0) {
			pData->AttachedTechnoEffect_Delay--;
		}
	}
	return;
}
