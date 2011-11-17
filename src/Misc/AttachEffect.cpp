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

Todo: something with that crash in cloak contra animation - crap, Documentation and crates
To-to-to-todo: Get a disassembler to update the hook (44A03C, BuildingClass_Mi_Selling_ReestablishMindControl, 6)
within Bugfixes.cpp to be set before TechnoClass::Remove, killing all effects on the way
*/

void AttachEffectClass::Read(INI_EX *exINI, const char * section) {
	AresCRT::strCopy(this->ID, section, 24);
	this->Duration.Read(exINI, section, "AttachEffect.Duration");
	this->Cumulative.Read(exINI, section, "AttachEffect.Cumulative");
	this->AnimType.Parse(exINI, section, "AttachEffect.Animation");
	this->FirepowerMultiplier.Read(exINI, section, "AttachEffect.FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, section, "AttachEffect.ArmorMultiplier");
	this->SpeedMultiplier.Read(exINI, section, "AttachEffect.SpeedMultiplier");
	this->Cloakable.Read(exINI, section, "AttachEffect.Cloakable");
}


void AttachEffectTypeClass::Attach(TechnoClass* Target, int Duration) {
	if (!Target) {return;}

	TechnoExt::ExtData *TargetExt = TechnoExt::ExtMap.Find(Target);

	if (!this->Cumulative && TargetExt->AttachedEffects.Count > 0) {
		for (int i=0; i < TargetExt->AttachedEffects.Count; i++) {
			auto Item = TargetExt->AttachedEffects.GetItem(i);
			if (!strcmp(this->ID, Item->Type->ID)) {
				Item->ActualDuration = Item->Type->Duration;
				return;
			}
		}
	}

	// there goes the actual attaching
	auto Attaching = new AttachEffectClass(this, Duration);
	TargetExt->AttachedEffects.AddItem(Attaching);

	// animation
	if (!!this->AnimType) {
		GAME_ALLOC(AnimClass, Attaching->Animation, this->AnimType, &Target->Location);
		Attaching->Animation->SetOwnerObject(Target);
		// inbefore void pointers, hardcode the iteration to infinitely looped
		Attaching->Animation->RemainingIterations = -1;
	}
	
	// update the unit with the attached effect
	TechnoExt::RecalculateStats(Target);
}

void AttachEffectClass::InvalidateFXAnimPointer(AnimClass *ptr) {
	if(this->Animation == ptr) {
		this->KillAnim();
	}
}

void AttachEffectClass::KillAnim() {
	if (this->Animation) {
		this->Animation->OwnerObject = NULL;
		this->Animation->IsPlaying = false;
		this->Animation->TimeToDie = true;
		//TimeToDie deletes the animation in the following frame... combining it with Remove() crashes, UnInit() crashes, GAME_DEALLOC crashes. Niiice.
		//While the above does the same and being the most elegant resolve.
		this->Animation->Audio1.ShutUp(); //Report
		this->Animation->Audio2.ShutUp();
		this->Animation->Audio3.ShutUp();
		this->Animation->Audio4.ShutUp();
		this->Animation = NULL;
	}
}

//remove the effects from the unit
void AttachEffectClass::Destroy() {
	this->KillAnim();
}
