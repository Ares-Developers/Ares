#include "AttachEffect.h"
#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/WarheadType/Body.h"

/*
Attached Effects system
covering feature requests
#1573, #1623 and #255

Graion Dilach, 2011-09-24+
Proper documentation gets done if proper code there is.

In a nutshell:
	relatively flexible way to interact with the unit's properties (as many as possible)
	to create as many as possible interesting effects

Todo: something with that crash in cloak contra animation - crap (D did it), Documentation and crates
To-to-to-todo: Get a hook in TechnoClass_Put to respawn anims killed in Remove for transported and co.
*/

void AttachEffectTypeClass::Read(INI_EX *exINI, const char * section) {
	AresCRT::strCopy(this->ID, section, 24);
	this->Duration.Read(exINI, section, "AttachEffect.Duration");
	this->Cumulative.Read(exINI, section, "AttachEffect.Cumulative");
	this->AnimType.Parse(exINI, section, "AttachEffect.Animation");
	this->AnimResetOnReapply.Read(exINI, section, "AttachEffect.AnimResetOnReapply");
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

	if (!this->Cumulative && TargetExt->AttachedEffects.Count > 0) {
		for (int i=0; i < TargetExt->AttachedEffects.Count; i++) {
			auto Item = TargetExt->AttachedEffects.GetItem(i);
			if (!strcmp(this->ID, Item->Type->ID)) {
				Item->ActualDuration = Item->Type->Duration;

				if (!!this->AnimType && !!this->AnimResetOnReapply) {
					Item->CreateAnim(Target);
				}

				return;
			}
		}
	}

	// there goes the actual attaching
	auto Attaching = new AttachEffectClass(this, Duration);
	TargetExt->AttachedEffects.AddItem(Attaching);

	if (Invoker) {
		Attaching->Invoker = Invoker;
	} else {
		Attaching->Invoker = NULL;
	}

	// update the unit with the attached effect
	TechnoExt::RecalculateStats(Target);

	// animation
	if (!!this->AnimType) {
		Attaching->CreateAnim(Target);
	}
	
}

void AttachEffectClass::InvalidateAnimPointer(AnimClass *ptr) {
	if(this->Animation == ptr) {
		this->KillAnim();
	}
}

void AttachEffectClass::CreateAnim(TechnoClass *Owner) {
	if (this->Animation){
		this->KillAnim();
	}
	GAME_ALLOC(AnimClass, this->Animation, this->Type->AnimType, &Owner->Location);
	this->Animation->SetOwnerObject(Owner);
	this->Animation->RemainingIterations = -1;
			
	if (this->Invoker && this->Invoker->Owner) {
		this->Animation->Owner = Invoker->Owner;
	}
	this->AnimAlreadyKilled = false;
}

void AttachEffectClass::KillAnim() {
	if (this->Animation && !this->AnimAlreadyKilled) {
		this->AnimAlreadyKilled = true;
		this->Animation->UnInit();
		this->Animation = NULL;
	}
}

//remove the effects from the unit
void AttachEffectClass::Destroy() {
	this->KillAnim();
}

/*!
	This function updates the units' AttachEffects.

	\retval boolean, to see if the unit gets killed (might just scrap 408 itself)

	\param Source The currently updated Techno.

	\author Graion Dilach
	\date 2011-09-24+
*/

bool AttachEffectClass::Update(TechnoClass *Source) {

	if (!Source || Source->InLimbo || Source->IsImmobilized || Source->Transporter || Source->TemporalTargetingMe) {
		return true;
	}

	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Source);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Source->GetTechnoType());

	if (pData->AttachedEffects.Count) {

		if(pData->AttachEffects_RecreateAnims) {
			for (int i = pData->AttachedEffects.Count; i > 0; --i) {
				pData->AttachedEffects.GetItem(i - 1)->CreateAnim(Source);
			}
			pData->AttachEffects_RecreateAnims = false;
		}

		//Debug::Log("[AttachEffect]AttachEffect update of %s...\n", Source->get_ID());
		for (int i = pData->AttachedEffects.Count; i > 0; --i) {
			auto Effect = pData->AttachedEffects.GetItem(i - 1);
			--Effect->ActualDuration;

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
						return false;
					}

					Effect->ActualDamageDelay = Effect->Type->DamageDelay;
				}

			}*/


			if(!Effect->ActualDuration || (!strcmp(Effect->Type->ID, Source->GetTechnoType()->ID) && Source->Deactivated)) {			//Bloody crashes - apparently if cloaked and attached, during delete it might crash - FIXED
				//Debug::Log("[AttachEffect] %d. item expired, removing...\n", i - 1);
				Effect->Destroy();

				if (!strcmp(Effect->Type->ID, Source->GetTechnoType()->ID)) {		//#1623, hardcodes Cumulative to false
					pData->AttachedTechnoEffect_isset = false;
					pData->AttachedTechnoEffect_Delay = Effect->Type->Delay;
				}

				delete Effect;
				pData->AttachedEffects.RemoveItem(i - 1);
				TechnoExt::RecalculateStats(Source);	//and update the unit's properties
				//Debug::Log("[AttachEffect] Remove #%d was successful.\n", i - 1);
			}

		}
		//Debug::Log("[AttachEffect]Update was successful.\n");
	}
	
	//#1623 - generating AttachedEffect from Type
	if (!!pTypeData->AttachedTechnoEffect.Duration && !pData->AttachedTechnoEffect_isset) {
		if (!pData->AttachedTechnoEffect_Delay){
			if (!Source->Deactivated){
				//Debug::Log("[AttachEffect]Missing Type effect of %s...\n", Source->get_ID());
				//pTypeData->AttachedTechnoEffect.Attach(Source, pTypeData->AttachedTechnoEffect.Duration, Source, pTypeData->AttachedTechnoEffect.DamageDelay);

				pTypeData->AttachedTechnoEffect.Attach(Source, pTypeData->AttachedTechnoEffect.Duration, Source);

				pData->AttachedTechnoEffect_isset = true;
				//Debug::Log("[AttachEffect]Readded to %s.\n", Source->get_ID());
			}

		} else {
			pData->AttachedTechnoEffect_Delay--;
		}
	}
	return true; //the unit is still alive
}