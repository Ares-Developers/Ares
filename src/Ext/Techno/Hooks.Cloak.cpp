#include "Body.h"

#include "../Rules/Body.h"
#include "../TechnoType/Body.h"

#include <HouseClass.h>

// replace the cloak checking functions to include checks for new features
DEFINE_HOOK(6FB757, TechnoClass_UpdateCloak, 8)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool tryCloak = !pExt->CloakDisallowed(false);

	return tryCloak ? 0x6FB7FD : 0x6FB75F;
}

DEFINE_HOOK(6FBDC0, TechnoClass_ShouldBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool ret = pExt->CloakAllowed();

	R->EAX(ret ? 1 : 0);
	return 0x6FBF93;
}

DEFINE_HOOK(6FBC90, TechnoClass_ShouldNotBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	// the original code would not disallow cloaking as long as
	// pThis->Cloakable is set, but this prevents CloakStop from
	// working, because it overrides IsCloakable().
	bool ret = pExt->CloakDisallowed(true);

	R->EAX(ret ? 1 : 0);
	return 0x6FBDBC;
}

DEFINE_HOOK(70380A, TechnoClass_Cloak_CloakSound, 6)
{
	GET(TechnoClass*, pThis, ESI);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	R->ECX(pExt->CloakSound.Get(RulesClass::Instance->CloakSound));
	return 0x703810;
}

DEFINE_HOOK(70375B, TechnoClass_Uncloak_DecloakSound, 6)
{
	GET(int, ptr, ESI);
	TechnoClass* pThis = reinterpret_cast<TechnoClass*>(ptr - 0x9C);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	int default = RulesExt::Global()->DecloakSound.Get(RulesClass::Instance->CloakSound);
	R->ECX(pExt->DecloakSound.Get(default));
	return 0x703761;
}

// replace Is_Moving_Now, because it doesn't check the
// current speed in case the unit is turning.
DEFINE_HOOK(4DBDD4, FootClass_IsCloakable_CloakStop, 6)
{
	GET(FootClass*, pThis, ESI);
	R->AL(pThis->Locomotor->Is_Moving());
	return 0x4DBDE3;
}

// health bar for detected submerged units
DEFINE_HOOK(6F5388, TechnoClass_DrawExtras_Submerged, 6)
{
	GET(TechnoClass*, pThis, EBP);

	bool drawHealth = pThis->IsSelected;
	if(!drawHealth) {
		// sensed submerged units
		drawHealth = !pThis->IsSurfaced() && pThis->GetCell()->Sensors_InclHouse(HouseClass::Player->ArrayIndex);
	}

	R->EAX(drawHealth);
	return 0x6F538E;
}

// customizable cloaking stages
DEFINE_HOOK(7036EB, TechnoClass_Uncloak_CloakingStages, 6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	R->ECX(pTypeExt->CloakStages.Get(RulesClass::Instance->CloakingStages));
	return 0x7036F1;
}

DEFINE_HOOK(703A79, TechnoClass_VisualCharacter_CloakingStages, A)
{
	GET(TechnoClass*, pThis, ESI);

	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	int stages = pTypeExt->CloakStages.Get(RulesClass::Instance->CloakingStages);
	int ret = Game::F2I(256.0 * pThis->CloakProgress.Value / stages);

	R->EAX(ret);
	return 0x703A94;
}

// respect the remove state when updating the parasite.
DEFINE_HOOK(4D99AA, FootClass_PointerGotInvalid_Parasite, 6)
{
	GET(FootClass*, pThis, ESI);
	GET(AbstractClass*, ptr, EDI);
	GET(bool, remove, EBX);

	// pass the real remove state, instead of always true. this was unused
	// in the original game, but now propagates the real value.
	if(auto pParasiteOwner = pThis->ParasiteEatingMe) {
		if(pParasiteOwner->Health > 0) {
			pParasiteOwner->ParasiteImUsing->PointerExpired(ptr, remove);
		}
	}

	// only unset the parasite owner, if we are removed.
	// cloaking does not count any more.
	if(pThis == ptr && remove) {
		pThis->ParasiteEatingMe = nullptr;
	}

	return 0x4D99D3;
}

// only eject the parasite if the unit leaves the battlefield,
// not just when it goes out of sight.
DEFINE_HOOK(62A283, ParasiteClass_PointerGotInvalid_Cloak, 9)
{
	GET(ParasiteClass*, pThis, ESI);
	GET(void*, ptr, EAX);
	GET_STACK(bool, remove, 0x2C);

	// remove only if pointee really got removed
	return (pThis->Victim == ptr && remove) ? 0x62A28C : 0x62A484;
}
