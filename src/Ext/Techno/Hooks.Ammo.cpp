#include "Body.h"
#include "../../Misc/Debug.h"
#include "../../Ares.h"

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
	GET(TechnoClass *, T, ESI);
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
		I->ReloadNow();
	}
	return 0;
}

DEFINE_HOOK(6FF66C, TechnoClass_Fire_RearmTimer, 6)
{
	GET(TechnoClass *, T, ESI);
	if(BuildingClass * B = specific_cast<BuildingClass *>(T)) {
		int Ammo = B->Type->Ammo;
		if(Ammo > 0 && B->Ammo < Ammo) {
			B->ReloadNow();
		}
	}
	return 0;
}

