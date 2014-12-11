#include <InfantryClass.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <TActionClass.h>
#include "Body.h"
#include "../WarheadType/Body.h"
#include "../AnimType/Body.h"

#include <SpecificStructures.h>

/* #188 - InfDeaths */

DEFINE_HOOK(5185C8, InfantryClass_ReceiveDamage_InfDeath, 6)
{
	GET(InfantryClass *, I, ESI);
	LEA_STACK(args_ReceiveDamage *, Arguments, 0xD4);
	GET(DWORD, InfDeath, EDI);
	--InfDeath;
	R->EDI(InfDeath);

	bool Handled = false;

	if(!I->Type->NotHuman) {
		if(I->GetHeight() < 10) {
			WarheadTypeExt::ExtData *pWHData = WarheadTypeExt::ExtMap.Find(Arguments->WH);
			if(AnimTypeClass *deathAnim = pWHData->InfDeathAnim) {
				AnimClass *Anim = GameCreate<AnimClass>(deathAnim, I->Location);

				HouseClass *Invoker = (Arguments->Attacker)
					? Arguments->Attacker->Owner
					: Arguments->SourceHouse
				;

				AnimTypeExt::SetMakeInfOwner(Anim, Invoker, I->Owner, Invoker);

				Handled = true;
			}
		}
	}

	return (Handled || InfDeath >= 10)
		? 0x5185F1
		: 0x5185CE
	;
}

DEFINE_HOOK(51849A, InfantryClass_ReceiveDamage_DeathAnim, 5)
{
	GET(InfantryClass *, I, ESI);
	LEA_STACK(args_ReceiveDamage *, Arguments, 0xD4);
	GET(DWORD, InfDeath, EDI);

	// if you got here, a valid DeathAnim for this InfDeath has been defined, and the game has already checked the preconditions
	// just allocate the anim and set its owner/remap

	AnimClass *Anim = GameCreate<AnimClass>(I->Type->DeathAnims[InfDeath], I->Location);

	HouseClass *Invoker = (Arguments->Attacker)
		? Arguments->Attacker->Owner
		: Arguments->SourceHouse
	;

	AnimTypeExt::SetMakeInfOwner(Anim, I->Owner, I->Owner, Invoker);

	R->EAX<AnimClass *>(Anim);
	return 0x5184F2;
}

DEFINE_HOOK_AGAIN(518575, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
DEFINE_HOOK(5183DE, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryVirus animation has been created. set the owner and color.

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimTypeExt::SetMakeInfOwner(pAnim, pInvoker, pThis->Owner, pInvoker);

	// bonus: don't require SpawnsParticle to be present

	if(ParticleSystemClass::Array->ValidIndex(pAnim->Type->SpawnsParticle)) {
		return 0;
	}

	return (R->Origin() == 0x5183DE) ? 0x518422 : 0x5185B9;
}

DEFINE_HOOK_AGAIN(518B93, InfantryClass_ReceiveDamage_Anims, 5) // InfantryBrute
DEFINE_HOOK_AGAIN(518821, InfantryClass_ReceiveDamage_Anims, 5) // InfantryNuked
DEFINE_HOOK_AGAIN(5187BB, InfantryClass_ReceiveDamage_Anims, 5) // InfantryHeadPop
DEFINE_HOOK_AGAIN(518755, InfantryClass_ReceiveDamage_Anims, 5) // InfantryElectrocuted
DEFINE_HOOK_AGAIN(5186F2, InfantryClass_ReceiveDamage_Anims, 5) // FlamingInfantry
DEFINE_HOOK(518698, InfantryClass_ReceiveDamage_Anims, 5) // InfantryExplode
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// animation has been created. set the owner and color.

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimTypeExt::SetMakeInfOwner(pAnim, nullptr, pThis->Owner, pInvoker);

	return 0x5185F1;
}

DEFINE_HOOK(51887B, InfantryClass_ReceiveDamage_InfantryVirus2, A)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryVirus animation has been created. set the owner, but
	// reset the color for default (invoker).

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	auto res = AnimTypeExt::SetMakeInfOwner(pAnim, pInvoker, pThis->Owner, pInvoker);
	if(res == OwnerHouseKind::Invoker) {
		pAnim->LightConvert = nullptr;
	}

	return 0x5185F1;
}

DEFINE_HOOK(518A96, InfantryClass_ReceiveDamage_InfantryMutate, 7)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryMutate animation has been created. set the owner and color.

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimTypeExt::SetMakeInfOwner(pAnim, pInvoker, pThis->Owner, pInvoker);

	return 0x518AFF;
}

DEFINE_HOOK(6E232E, ActionClass_PlayAnimAt, 5)
{
	GET(TActionClass *, pAction, ESI);
	GET_STACK(HouseClass *, pHouse, 0x1C);
	LEA_STACK(CoordStruct *, pCoords, 0xC);

	AnimTypeClass *AnimType = AnimTypeClass::Array->GetItem(pAction->Value);
	AnimClass *Anim = GameCreate<AnimClass>(AnimType, *pCoords);

	if(AnimType->MakeInfantry > -1) {
		AnimTypeExt::SetMakeInfOwner(Anim, pHouse, pHouse, pHouse);
	}

	R->EAX<AnimClass *>(Anim);

	return 0x6E2368;
}

DEFINE_HOOK(469C4E, BulletClass_DetonateAt_DamageAnimSelected, 5)
{
	GET(AnimTypeClass *, AnimType, EBX);
	LEA_STACK(CoordStruct *, XYZ, 0x64);

	AnimClass * Anim = GameCreate<AnimClass>(AnimType, *XYZ, 0, 1, 0x2600, -15, false);

	if(Anim) {
		GET(BulletClass *, Bullet, ESI);

		HouseClass *pInvoker = (Bullet->Owner)
			? Bullet->Owner->Owner
			: nullptr
		;

		HouseClass *pVictim = nullptr;

		if(TechnoClass * Target = generic_cast<TechnoClass *>(Bullet->Target)) {
			pVictim = Target->Owner;
		}

		if(Anim->Type->MakeInfantry > -1) {
			AnimTypeExt::SetMakeInfOwner(Anim, pInvoker, pVictim, pInvoker);
		}
	}

	R->EAX<AnimClass *>(Anim);
	return 0x469C98;
}
