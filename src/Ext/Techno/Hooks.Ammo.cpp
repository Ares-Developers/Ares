#include "Body.h"
#include "../TechnoType/Body.h"
#include "../WeaponType/Body.h"
#include "../../Misc/Debug.h"
#include "../../Ares.h"

#include <BuildingClass.h>
#include <InfantryClass.h>

// bugfix #471: InfantryTypes and BuildingTypes don't reload their ammo properly

DEFINE_HOOK(43FE8E, BuildingClass_Update_Reload, 6)
{
	GET(BuildingClass *, B, ESI);
	BuildingTypeClass *BType = B->Type;
	if(!BType->Hospital && !BType->Armory) { // TODO: rethink this
		B->Reload();
	}
	return 0x43FEBE;
}

DEFINE_HOOK(6FCFA4, TechnoClass_GetROF_BuildingHack, 5)
{
	//GET(TechnoClass *, T, ESI);
	// actual game code: if(auto B = specific_cast<BuildingClass *>(T)) { if(T->currentAmmo > 1) { return 1; } }
	// if the object being queried doesn't have a weapon (Armory/Hospital), it'll return 1 anyway

	return 0x6FCFC1;
}

DEFINE_HOOK(5200D7, InfantryClass_UpdatePanic_DontReload, 6)
{
	return 0x52010B;
}

DEFINE_HOOK(51BCB2, InfantryClass_Update_Reload, 6)
{
	GET(InfantryClass *, I, ESI);
	if(I->InLimbo) {
		return 0x51BDCF;
	}
	I->Reload();
	return 0x51BCC0;
}

DEFINE_HOOK(51DF8C, InfantryClass_Fire_RearmTimer, 6)
{
	GET(InfantryClass *, I, ESI);
	int Ammo = I->Type->Ammo;
	if(Ammo > 0 && I->Ammo < Ammo) {
		I->StartReloading();
	}
	return 0;
}

DEFINE_HOOK(6FF66C, TechnoClass_Fire_RearmTimer, 6)
{
	GET(TechnoClass *, T, ESI);
	if(BuildingClass * B = specific_cast<BuildingClass *>(T)) {
		int Ammo = B->Type->Ammo;
		if(Ammo > 0 && B->Ammo < Ammo) {
			B->StartReloading();
		}
	}
	return 0;
}

// weapons can take more than one round of ammo
DEFINE_HOOK(6FCA0D, TechnoClass_GetFireError_Ammo, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	return (pThis->Ammo < 0 || pThis->Ammo >= pExt->Ammo)
		? 0x6FCA26u : 0x6FCA17u;
}

// remove ammo rounds depending on weapon
DEFINE_HOOK(6FF656, TechnoClass_Fire_Ammo, A)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass const* const, pWeapon, EBX);

	TechnoExt::DecreaseAmmo(pThis, pWeapon);

	return 0x6FF660;
}

// variable amounts of rounds to reload
DEFINE_HOOK(6FB05B, TechnoClass_Reload_ReloadAmount, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	int amount = pExt->ReloadAmount;
	if(!pThis->Ammo) {
		amount = pExt->EmptyReloadAmount.Get(amount);
	}

	// clamping to support negative values
	auto const ammo = pThis->Ammo + amount;
	pThis->Ammo = Math::clamp(ammo, 0, pType->Ammo);

	return 0x6FB061;
}
